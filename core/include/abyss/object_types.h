#pragma once

#include <vector>
#include <cstdint>
#include "abyss/types.h"

namespace abyss {
 

struct Object {
    abyss :: ObjectType test; 
    std::vector<uint8_t> content;  // header sonrası ham içerik
};
 
}
