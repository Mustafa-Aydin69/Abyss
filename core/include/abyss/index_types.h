#pragma once

#include<string>
#include<cstdint>
#include "types.h"

namespace abyys {
    struct IndexEntry
    {
        std::string path;
        Hash hash;
        int64_t mtime;
        uint64_t size;
    };
    
}