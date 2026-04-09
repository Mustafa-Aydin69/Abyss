#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <cstring>
#include "types.h"
#include "object_types.h"

namespace abyss {

// ── Object Store ──────────────────────────────────────────────────────────────
//
// Sorumluluk: blob / tree / commit objelerini .abyss/objects/ altına
// yazar ve okur. Başka hiçbir şey yapmaz.
//
// Disk formatı:
//   raw        = "<type_str> <size>\0<content>"
//                type_str   → "blob" | "tree" | "commit"
//                size       → content'in byte sayısı (decimal string)
//                \0         → null byte ayraç
//                content    → ham içerik
//
//   hash       = sha256(raw)           ← compressed değil, raw üzerinden
//   on_disk    = zlib_compress(raw)
//   path       = .abyss/objects/<hex[0:2]>/<hex[2:64]>
//
// Concurrency kararı (bilinçli):
//   Aynı anda aynı hash yazılırsa atomic_write overwrite eder.
//   İçerik aynı olduğundan bu güvenlidir (Git de aynı kararı almıştır).
//   Farklı hash'ler farklı path'e gider — çakışma olmaz.
//
// object_path() public bırakılmıştır (test edilebilirlik).
//   Üst katman bu path'e bağımlı olmamalıdır.
//   İleride private olabilir.

class ObjectStore {
public:
    // repo_path: proje kökü (.abyss/'ın parent'ı)
    explicit ObjectStore(const std::string& repo_path);

    // ── Yazma ─────────────────────────────────────────────────────────────────
    //
    // Precondition : content boş olabilir (örn: boş dosya blob'u)
    // Postcondition: out_hash = sha256(header + content)
    //
    // Hata durumları:
    //   ERR_GENERAL   → compressed output üretilemedi
    //   ERR_GENERAL   → directory oluşturulamadı
    //   ERR_GENERAL   → atomic_write başarısız
    ExitCode write_object(ObjectType                  type,
                          const std::vector<uint8_t>& content,
                          Hash&                       out_hash);

    // ── Okuma ─────────────────────────────────────────────────────────────────
    //
    // Hata durumları:
    //   ERR_NOT_FOUND  → dosya diskte yok
    //   ERR_CORRUPTED  → decompress başarısız
    //   ERR_CORRUPTED  → hash doğrulama başarısız (mismatch)
    //   ERR_CORRUPTED  → null byte bulunamadı (header parse)
    //   ERR_CORRUPTED  → size mismatch (header'daki ile gerçek boyut farklı)
    //   ERR_CORRUPTED  → type string geçersiz ("blob"/"tree"/"commit" değil)
    ExitCode read_object(const Hash& hash, Object& out);

    // ── Varlık kontrolü ───────────────────────────────────────────────────────
    //
    // Sadece dosya varlığını kontrol eder.
    // Hash doğrulaması yapmaz — hızlı kontrol içindir.
    bool exists(const Hash& hash) const;

    // ── Yardımcı ──────────────────────────────────────────────────────────────
    //
    // Hash → disk path döner.
    // Örn: hash hex "2cf24dba..." → ".abyss/objects/2c/f24dba..."
    // Not: internal detail — üst katman bu path'e bağımlı olmamalıdır.
    std::string object_path(const Hash& hash) const;

private:
    std::string objects_dir_;  // .abyss/objects/

    // Header oluştur: "<type_str> <size>\0"
    // Örn: "blob 13\0"
    std::vector<uint8_t> make_header(ObjectType type,
                                     size_t     content_size) const;

    // raw veriyi parse eder: header + content ayırır.
    //
    // Contract:
    //   raw = "<type_str> <size>\0<content>"
    //
    // Hata durumları (hepsi ERR_CORRUPTED):
    //   → null byte bulunamazsa
    //   → type string "blob"/"tree"/"commit" değilse
    //   → size parse edilemezse
    //   → size ile content.size() uyuşmazsa
    ExitCode parse_object(const std::vector<uint8_t>& raw,
                          ObjectType&                 out_type,
                          std::vector<uint8_t>&       out_content) const;

    // Hash hex'inin ilk 2 karakterinden subdirectory oluşturur.
    // Örn: hex "2cf24d..." → ".abyss/objects/2c/" klasörünü açar.
    // Zaten varsa başarı döner (idempotent).
    ExitCode ensure_object_subdir(const std::string& hex) const;
};

} // namespace abyss