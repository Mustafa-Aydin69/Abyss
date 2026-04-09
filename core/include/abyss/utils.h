// SHA-256, zlib, dosya IO yardimci fonksiyonlar
#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>
#include "types.h"

namespace abyys :: utils{

    //Yüksek Seviye Vector için
    Hash sha256(const std::vector<uint8_t>& data);


    // Düşük Seviye - ham pointer için (streaming, mmap vb.)
    Hash sha256(const uint8_t* data, size_t size);
    // Hash → hex string (64 karakter)  
    std::string  hash_to_hex(const Hash& hash);
    // Hex string → Hash
    // Geçersiz hex gelirse false döner, out değiştirilmez
    bool hex_to_hash(const std::string& hex, Hash& out);


    std::vector<uint8_t> zlib_compress(const std::vector<uint8_t>& data);
    std::vector<uint8_t> zlib_decompress(const std::vector<uint8_t>& data);
    // temp + fsync(file) + rename + fsync(dir)
    // Başarısızlık durumunda ExitCode döner — void değil

    ExitCode atomic_write(const std::string& path, const std::vector<uint8_t>& data);

}