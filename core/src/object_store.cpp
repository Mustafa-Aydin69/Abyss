#include "abyss/object_store.h"
#include "abyss/utils.h"

#include <filesystem>
#include <fstream>
#include <charconv>
#include <algorithm>
#include <cstring>

namespace fs = std::filesystem;

namespace abyss {

// ── Type dönüşüm yardımcıları (merkezi, duplicate yok) ───────────────────────

static const char* object_type_to_str(ObjectType type) {
    switch (type) {
        case ObjectType::BLOB:   return "blob";
        case ObjectType::TREE:   return "tree";
        case ObjectType::COMMIT: return "commit";
        default:                 return nullptr;
    }
}

static bool str_to_object_type(const std::string& s, ObjectType& out) {
    if (s == "blob")   { out = ObjectType::BLOB;   return true; }
    if (s == "tree")   { out = ObjectType::TREE;   return true; }
    if (s == "commit") { out = ObjectType::COMMIT; return true; }
    return false;
}

// ── Constructor ───────────────────────────────────────────────────────────────

ObjectStore::ObjectStore(const std::string& repo_path)
    : objects_dir_((fs::path(repo_path) / ".abyss" / "objects").string()) {}

// ── object_path ───────────────────────────────────────────────────────────────

std::string ObjectStore::object_path(const Hash& hash) const {
    std::string hex = utils::hash_to_hex(hash);
    return (fs::path(objects_dir_) / hex.substr(0, 2) / hex.substr(2)).string();
}

// ── ensure_object_subdir ─────────────────────────────────────────────────────

ExitCode ObjectStore::ensure_object_subdir(const std::string& hex) const {
    fs::path subdir = fs::path(objects_dir_) / hex.substr(0, 2);
    std::error_code ec;
    fs::create_directories(subdir, ec);
    return ec ? ExitCode::ERR_GENERAL : ExitCode::SUCCESS;
}

// ── make_header ───────────────────────────────────────────────────────────────

std::vector<uint8_t> ObjectStore::make_header(ObjectType type,
                                               size_t content_size) const {
    const char* type_str = object_type_to_str(type);
    if (!type_str) return {};  // UNKNOWN type — çağıran kontrol etmeli

    // "<type_str> <size>\0"
    std::string header = std::string(type_str)
                       + " "
                       + std::to_string(content_size)
                       + '\0';
    return std::vector<uint8_t>(header.begin(), header.end());
}

// ── write_object ──────────────────────────────────────────────────────────────

ExitCode ObjectStore::write_object(ObjectType                  type,
                                   const std::vector<uint8_t>& content,
                                   Hash&                       out_hash) {
    // 1. Header — merkezi type dönüşümü kullanır
    std::vector<uint8_t> header = make_header(type, content.size());
    if (header.empty()) return ExitCode::ERR_GENERAL;  // UNKNOWN type

    // 2. raw = header + content — tek alloc, memcpy ile doldur
    std::vector<uint8_t> raw(header.size() + content.size());
    std::memcpy(raw.data(),                header.data(),  header.size());
    std::memcpy(raw.data() + header.size(), content.data(), content.size());

    // 3. Hash — raw üzerinden, compressed değil
    Hash hash = utils::sha256(raw.data(), raw.size());

    // 4. exists() kontrolü — bilinçli karar:
    //    Hash, exists() çağrısından ÖNCE hesaplanıyor.
    //    Alternatif (önce exists, sonra hash) bir race window açar:
    //    exists=false → başka process yazar → biz tekrar yazarız.
    //    Hash önce hesaplanınca race window kapanır.
    //    Tradeoff: her write'ta hash hesaplanır (ucuz), disk IO atlanır (pahalı).
    if (exists(hash)) {
        out_hash = hash;
        return ExitCode::SUCCESS;
    }

    // 5. Subdirectory oluştur
    std::string hex = utils::hash_to_hex(hash);
    ExitCode ec = ensure_object_subdir(hex);
    if (ec != ExitCode::SUCCESS) return ec;

    // 6. Compress — ExitCode döner, boş input da zlib'e bırakılır
    std::vector<uint8_t> compressed;
    ec = utils::zlib_compress(raw, compressed);
    if (ec != ExitCode::SUCCESS) return ec;

    // 7. Atomic write — utils::atomic_write:
    //    temp + fsync(file) + rename + fsync(dir)
    //    fsync(dir) rename kaydının kaybolmamasını garantiler.
    //    Bu olmadan crash sonrası dosya var ama directory entry yok olabilir.
    ec = utils::atomic_write(object_path(hash), compressed);
    if (ec != ExitCode::SUCCESS) return ec;

    out_hash = hash;
    return ExitCode::SUCCESS;
}

// ── read_object ───────────────────────────────────────────────────────────────

ExitCode ObjectStore::read_object(const Hash& hash, Object& out) {
    fs::path path = fs::path(object_path(hash));

    // 1. Dosya var mı? — ERR_NOT_FOUND, ERR_CORRUPTED ile karıştırılmaz
    if (!fs::exists(path)) return ExitCode::ERR_NOT_FOUND;

    // 2. Dosyayı oku
    // TODO: Büyük obje desteği — şu an tüm obje memory'ye yükleniyor.
    //       İleride streaming read eklenecek (10MB+ objeler için).
    std::ifstream file(path, std::ios::binary);
    if (!file) return ExitCode::ERR_NOT_FOUND;

    std::vector<uint8_t> compressed;

    file.seekg(0, std::ios::end);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    if (size < 0) return ExitCode::ERR_CORRUPTED;

    compressed.resize(size);

    if (!file.read(reinterpret_cast<char*>(compressed.data()), size)) {
        return ExitCode::ERR_CORRUPTED;
    }

    // 3. Decompress — ExitCode döner (compress ile tutarlı)
    //    zlib_decompress total_out ile resize yapar — truncated data üretmez.
    //    Truncation senaryosu: Z_STREAM_END gelmeden loop biter → Z_BUF_ERROR
    //    Bu durum ERR_CORRUPTED olarak döner, sessiz geçmez.
    std::vector<uint8_t> raw;
    ExitCode ec = utils::zlib_decompress(compressed, raw);
    if (ec != ExitCode::SUCCESS) return ExitCode::ERR_CORRUPTED;

    // 4. Hash doğrula — raw üzerinden
    //    Decompress başarılı olsa bile hash uyuşmazsa veri bozuk demektir.
    Hash actual = utils::sha256(raw.data(), raw.size());
    if (actual != hash) return ExitCode::ERR_CORRUPTED;

    // 5. Parse
    ObjectType           type;
    std::vector<uint8_t> content;
    ec = parse_object(raw, type, content);
    if (ec != ExitCode::SUCCESS) return ec;

    out.type    = type;
    out.content = std::move(content);
    return ExitCode::SUCCESS;
}

// ── exists ────────────────────────────────────────────────────────────────────

bool ObjectStore::exists(const Hash& hash) const {
    return fs::exists(object_path(hash));
}

// ── parse_object ─────────────────────────────────────────────────────────────

ExitCode ObjectStore::parse_object(const std::vector<uint8_t>& raw,
                                   ObjectType&                 out_type,
                                   std::vector<uint8_t>&       out_content) const {
    // raw = "<type_str> <size>\0<content>"

    // 1. Null byte bul — header sonu
    auto null_it = std::find(raw.begin(), raw.end(), uint8_t(0));
    if (null_it == raw.end()) return ExitCode::ERR_CORRUPTED;

    // 2. Header string
    std::string header(raw.begin(), null_it);

    // 3. Space bul
    size_t space_pos = header.find(' ');
    if (space_pos == std::string::npos) return ExitCode::ERR_CORRUPTED;

    // 4. Type parse — merkezi str_to_object_type kullanır (duplicate yok)
    ObjectType type;
    if (!str_to_object_type(header.substr(0, space_pos), type)) {
        return ExitCode::ERR_CORRUPTED;
    }

    // 5. Size parse — from_chars: exception yok, taşma güvenli
    std::string size_str = header.substr(space_pos + 1);

    // Strict parsing kararı:
    //   Leading zero reddedilir — "013" geçersiz, "13" geçerli.
    //   Sebep: "013" octal olarak yorumlanabilir, ambiguity riski var.
    //   make_header std::to_string kullanır → leading zero üretmez.
    //   Dışarıdan gelen bozuk veri bu kuralla yakalanır.
    if (size_str.empty()
        || size_str[0] == '-'
        || (size_str.size() > 1 && size_str[0] == '0')  // leading zero
        || size_str.find_first_not_of("0123456789") != std::string::npos) {
        return ExitCode::ERR_CORRUPTED;
    }

    size_t declared_size = 0;
    auto [ptr, ec] = std::from_chars(
        size_str.data(),
        size_str.data() + size_str.size(),
        declared_size
    );
    if (ec != std::errc{} || ptr != size_str.data() + size_str.size()) {
        return ExitCode::ERR_CORRUPTED;
    }

    // 6. Content
    std::vector<uint8_t> content(null_it + 1, raw.end());

    // 7. Size doğrula
    if (content.size() != declared_size) return ExitCode::ERR_CORRUPTED;

    out_type    = type;
    out_content = std::move(content);
    return ExitCode::SUCCESS;
}

} // namespace abyss