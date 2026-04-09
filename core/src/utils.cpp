#include "abyss/utils.h"

#include <openssl/sha.h>
#include <zlib.h>

#include <cerrno>
#include <cassert>
#include <cstring>
#include <cstdio>
#include <filesystem>
#include <sstream>
#include <iomanip>
#include <random>

#ifdef _WIN32
#   include <windows.h>
#else
#   include <unistd.h>
#   include <fcntl.h>
#endif

namespace fs = std::filesystem;

namespace abyss::utils {   // ← typo düzeltildi (abyys → abyss)

// ── SHA-256 ───────────────────────────────────────────────────────────────────

Hash sha256(const uint8_t* data, size_t size) {
    // nullptr + size > 0 → programlama hatası, silent fail değil
    // Hash{} döndürmek valid hash gibi gözükür → silent data corruption
    assert(data != nullptr || size == 0);
    Hash out;
    SHA256(data, size, out.data());
    return out;
}

// ── Hash / Hex dönüşüm ────────────────────────────────────────────────────────

std::string hash_to_hex(const Hash& hash) {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (uint8_t byte : hash) {
        oss << std::setw(2) << static_cast<int>(byte);
    }
    return oss.str();  // her zaman lowercase (a-f)
}

// Hex karakterini sayıya çeviren yardımcı — exception yok, manuel parse
static bool hex_char_to_nibble(char c, uint8_t& out) {
    if (c >= '0' && c <= '9') { out = static_cast<uint8_t>(c - '0');        return true; }
    if (c >= 'a' && c <= 'f') { out = static_cast<uint8_t>(c - 'a' + 10);  return true; }
    // uppercase kabul edilmiyor — standart lowercase (hash_to_hex ile tutarlı)
    return false;
}

bool hex_to_hash(const std::string& hex, Hash& out) {
    if (hex.size() != 64) return false;

    Hash result;
    for (size_t i = 0; i < 32; ++i) {
        uint8_t hi = 0, lo = 0;
        if (!hex_char_to_nibble(hex[i * 2],     hi)) return false;
        if (!hex_char_to_nibble(hex[i * 2 + 1], lo)) return false;
        result[i] = static_cast<uint8_t>((hi << 4) | lo);
    }

    out = result;
    return true;
    // stoul kullanılmadı — exception riski yok, API kontratı korunuyor
}

// ── Zlib ──────────────────────────────────────────────────────────────────────

ExitCode zlib_compress(const std::vector<uint8_t>& input,
                       std::vector<uint8_t>&       output) {
    uLongf bound    = compressBound(static_cast<uLong>(input.size()));
    output.resize(bound);

    uLongf out_size = bound;
    int ret = compress2(
        output.data(), &out_size,
        input.data(),  static_cast<uLong>(input.size()),
        Z_BEST_COMPRESSION
    );

    if (ret != Z_OK) {
        output.clear();
        return ExitCode::ERR_GENERAL;
    }

    output.resize(out_size);
    return ExitCode::SUCCESS;
}

ExitCode zlib_decompress(const std::vector<uint8_t>& input,
                         std::vector<uint8_t>& output) {
    // Boş input — anlamsız, erken dön
    if (input.empty()) {
        output.clear();
        return ExitCode::SUCCESS;
    }

    size_t buf_size = input.size() * 4;
    if (buf_size < 1024) buf_size = 1024;
    output.resize(buf_size);

    z_stream stream{};
    // const_cast — zlib API'si non-const istiyor, bilinçli kullanım
    stream.next_in  = const_cast<Bytef*>(input.data());
    stream.avail_in = static_cast<uInt>(input.size());

    if (inflateInit(&stream) != Z_OK) {
        return ExitCode::ERR_CORRUPTED;
    }

    int ret = Z_OK;
    while (ret != Z_STREAM_END) {
        stream.next_out  = output.data() + stream.total_out;
        stream.avail_out = static_cast<uInt>(output.size() - stream.total_out);

        ret = inflate(&stream, Z_NO_FLUSH);

        if (ret == Z_STREAM_ERROR ||
            ret == Z_DATA_ERROR   ||
            ret == Z_MEM_ERROR    ||
            ret == Z_BUF_ERROR) {
            inflateEnd(&stream);
            return ExitCode::ERR_CORRUPTED;
        }

        // Buffer doldu → 2 katına çıkar
        if (ret != Z_STREAM_END && stream.avail_out == 0) {
            output.resize(output.size() * 2);
        }
    }

    output.resize(stream.total_out);
    inflateEnd(&stream);
    return ExitCode::SUCCESS;
}

// ── Atomic write ──────────────────────────────────────────────────────────────

// Temp dosya ismi için random suffix — çakışma önleme
static std::string make_tmp_path(const std::string& path) {
    // static → her çağrıda random_device initialize edilmez, ucuz
    static std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<uint32_t> dist;
    std::ostringstream oss;
    oss << path << ".tmp." << std::hex << dist(gen);
    return oss.str();
}

ExitCode atomic_write(const std::string&          path,
                      const std::vector<uint8_t>& data) {

    std::string tmp_path = make_tmp_path(path);  // random suffix → çakışma yok

#ifdef _WIN32

    // ── Windows ──────────────────────────────────────────────────────────────
    HANDLE fh = CreateFileA(
        tmp_path.c_str(),
        GENERIC_WRITE, 0, nullptr,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr
    );
    if (fh == INVALID_HANDLE_VALUE) return ExitCode::ERR_GENERAL;

    DWORD written = 0;
    BOOL  ok = WriteFile(fh, data.data(),
                         static_cast<DWORD>(data.size()), &written, nullptr);
    if (!ok || written != static_cast<DWORD>(data.size())) {
        CloseHandle(fh);
        DeleteFileA(tmp_path.c_str());
        return ExitCode::ERR_GENERAL;
    }

    if (!FlushFileBuffers(fh)) {
        CloseHandle(fh);
        DeleteFileA(tmp_path.c_str());
        return ExitCode::ERR_GENERAL;
    }
    CloseHandle(fh);

    if (!MoveFileExA(tmp_path.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING)) {
        DeleteFileA(tmp_path.c_str());
        return ExitCode::ERR_GENERAL;
    }

    // Parent klasör flush — kabul edilebilir seviyede
    std::string parent = fs::path(path).parent_path().string();
    HANDLE dh = CreateFileA(
        parent.c_str(),
        GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr, OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS, nullptr
    );
    if (dh != INVALID_HANDLE_VALUE) {
        FlushFileBuffers(dh);
        CloseHandle(dh);
    }

#else

    // ── POSIX (Linux / macOS) ─────────────────────────────────────────────────
    int fd = open(tmp_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) return ExitCode::ERR_GENERAL;

    // Write loop — EINTR durumunda tekrar dene
    size_t written = 0;
    while (written < data.size()) {
        ssize_t n = write(fd, data.data() + written, data.size() - written);
        if (n < 0) {
            if (errno == EINTR) continue;
            close(fd);
            unlink(tmp_path.c_str());
            return ExitCode::ERR_GENERAL;
        }
        written += static_cast<size_t>(n);
    }

    // Dosyayı diske zorla
    if (fsync(fd) != 0) {
        close(fd);
        unlink(tmp_path.c_str());
        return ExitCode::ERR_GENERAL;
    }
    close(fd);

    // Atomic rename
    if (rename(tmp_path.c_str(), path.c_str()) != 0) {
        unlink(tmp_path.c_str());
        return ExitCode::ERR_GENERAL;
    }

    // Parent klasörü fsync — O_DIRECTORY flag eklendi
    std::string parent = fs::path(path).parent_path().string();
    int dir_fd = open(parent.c_str(), O_RDONLY | O_DIRECTORY);
    if (dir_fd >= 0) {
        fsync(dir_fd);  // hata olsa bile devam — dosya zaten yazıldı
        close(dir_fd);
    }

#endif

    return ExitCode::SUCCESS;
}

} // namespace abyss::utils