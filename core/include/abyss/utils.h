#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>
#include "types.h"


namespace abyss::utils {


Hash sha256(const uint8_t* data, size_t size);

inline Hash sha256(const std::vector<uint8_t>& data) {
    return sha256(data.data(), data.size());
}


std::string hash_to_hex(const Hash& hash);

bool hex_to_hash(const std::string& hex, Hash& out);

ExitCode zlib_compress(const std::vector<uint8_t>& input,
                       std::vector<uint8_t>&       output);

ExitCode zlib_decompress(const std::vector<uint8_t>& input,
                         std::vector<uint8_t>&       output);

ExitCode atomic_write(const std::string&           path,
                      const std::vector<uint8_t>&  data);

} 