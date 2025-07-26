#pragma once
#include <filesystem>

typedef uint64_t ResourceID;
typedef std::filesystem::path ResourcePath;
#define PATHS(name) ResourcePath name_PATH []

// using fnv1a_hash 
constexpr uint64_t hash_str(const char* str) {
    uint64_t hash = 14695981039346656037ULL; // FNV offset basis
    while (*str) {
        hash ^= static_cast<uint64_t>(*str++);
        hash *= 1099511628211ULL; // FNV prime
    }
    return hash;
}