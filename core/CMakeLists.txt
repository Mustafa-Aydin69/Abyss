cmake_minimum_required(VERSION 3.17)
project(abyss VERSION 0.1.0 LANGUAGES CXX)

# ── C++17 ─────────────────────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ── Build type ────────────────────────────────────────────────────────────────
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# ── Uyarılar ──────────────────────────────────────────────────────────────────
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# ── zlib ──────────────────────────────────────────────────────────────────────
find_package(ZLIB REQUIRED)

# ── OpenSSL (SHA-256 için) ────────────────────────────────────────────────────
find_package(OpenSSL REQUIRED)

# ── Kaynak dosyalar ───────────────────────────────────────────────────────────
set(ABYSS_SOURCES
    src/main.cpp
    src/utils.cpp
    src/object_store.cpp
    src/repository.cpp
    src/index.cpp
    src/commit.cpp
    src/branch.cpp
    src/diff.cpp
    src/lock.cpp
    src/recovery.cpp
    src/commands/init.cpp
    src/commands/add.cpp
    src/commands/commit_cmd.cpp
    src/commands/status.cpp
    src/commands/log.cpp
    src/commands/diff_cmd.cpp
    src/commands/branch_cmd.cpp
    src/commands/checkout.cpp
    src/commands/gc.cpp
    src/commands/recover.cpp
)

# ── Ana binary ────────────────────────────────────────────────────────────────
add_executable(abyss ${ABYSS_SOURCES})

target_include_directories(abyss PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(abyss PRIVATE
    ZLIB::ZLIB
    OpenSSL::Crypto
)

# ── Test binary ───────────────────────────────────────────────────────────────
set(ABYSS_TEST_SOURCES
    src/utils.cpp
    src/object_store.cpp
    src/repository.cpp
    src/index.cpp
    src/commit.cpp
    src/branch.cpp
    src/diff.cpp
    src/lock.cpp
    src/recovery.cpp
    src/commands/init.cpp
    src/commands/add.cpp
    src/commands/commit_cmd.cpp
    src/commands/status.cpp
    src/commands/log.cpp
    src/commands/diff_cmd.cpp
    src/commands/branch_cmd.cpp
    src/commands/checkout.cpp
    src/commands/gc.cpp
    src/commands/recover.cpp
    tests/test_object_store.cpp
    tests/test_index.cpp
    tests/test_diff.cpp
    tests/test_commit.cpp
)

add_executable(abyss_tests ${ABYSS_TEST_SOURCES})

target_include_directories(abyss_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(abyss_tests PRIVATE
    ZLIB::ZLIBC
    OpenSSL::Crypto
)

# ── Install ───────────────────────────────────────────────────────────────────
install(TARGETS abyss DESTINATION bin)

# ── Özet ──────────────────────────────────────────────────────────────────────
message(STATUS "")
message(STATUS "Abyss ${PROJECT_VERSION}")
message(STATUS "  Build type : ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++        : ${CMAKE_CXX_STANDARD}")
message(STATUS "  zlib       : ${ZLIB_VERSION_STRING}")
message(STATUS "  OpenSSL    : ${OPENSSL_VERSION}")
message(STATUS "")