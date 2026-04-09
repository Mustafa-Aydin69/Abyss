// Ortak veri tipleri ve enum'lar. Tüm sistemlerde ortak tipler kullanılacak. Ağır dependency olmamasına dikkat edilecek. Enum Class kullanılacak. Type Safety.
// hash tipi string olarak kabul edilebilir ama zayıf bir sistemdir. Binary stringe nazaran daha güçlü. timestamp string olarak kullanılabilir ama parse edilmesi ve
// karşılaştırma yapılması daha maliyetlidir. DiffType'de MODIFIED'da olacak bunu unutma. Exit Codeler biraz daha açıklayıcı olmalı ERROR yerine ERR_NO_REPO tarzı
// açıklayıcı şekilde olmalı. Dosya içeriği çok uzun olmamalı Types.h dosyası fazla şişirilmemeli minimal olmalı.
#pragma once

#include <array>
#include <cstdint>
namespace abyss{


    using Hash = std::array<uint8_t, 32>;

    enum class ObjectType : uint8_t {
        BLOB    = 0,
        TREE    = 1,
        COMMIT  = 2,
        UNKNOWN = 255
    };

    enum class ExitCode : int {
        SUCCESS = 0,
        ERR_GENERAL = 1,
        ERR_NO_REPO = 2,
        ERR_NOT_FOUND = 3,
        ERR_CONFLICT = 4,
        ERR_EXISTS = 5,
        ERR_CORRUPTED = 6,
        ERR_LOCKED = 7,
        ERR_DIRTY = 8,
        ERR_TIMEOUT = 9,
        ERR_INVALID_JSON = 10,
        ERR_NOT_BINARY = 11
    };

    enum class DiffLineType : uint8_t {
        ADDED =0,
        REMOVED = 1,
        UNCHANCED = 2,
        MODIFIED = 3
    };

    struct LockInfo
    {
        int pid;
        int64_t timestamp;
        char hostname[265];
    };

}