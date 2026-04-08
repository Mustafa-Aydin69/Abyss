#pragma once

#include <vector>
#include <cstdint>
#include "abyss/types.h"

namespace abyss {
 

struct Object {
    abyys :: ObjectType test; 
    std::vector<uint8_t> content;  // header sonrası ham içerik
};
 
}
