#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

namespace planet {

static constexpr uint32_t KERDATA_MAGIC   = 0x4452454Bu;  // "KERD" little-endian
static constexpr uint32_t KERDATA_VERSION = 1;

struct KerdataHeader {
    uint32_t magic         = 0;
    uint32_t version       = 0;
    uint32_t fileCount     = 0;
    uint32_t tableOffset   = 0;   // byte offset from file start to entry table
    uint32_t dataOffset    = 0;   // byte offset from file start to raw data
    uint32_t totalDataSize = 0;   // total uncompressed size of all files
    uint32_t flags         = 0;   // reserved
};

static constexpr uint32_t KERDATA_FLAG_COMPRESSED = 0x01;

struct KerdataEntry {
    std::string path;
    uint32_t dataOffset = 0;   // from dataOffset in header
    uint32_t dataSize   = 0;
    uint32_t flags      = 0;
};

struct KerdataArchive {
    KerdataHeader header;
    std::vector<KerdataEntry> entries;
    std::unordered_map<std::string, uint32_t> pathIndex;  // path -> entry index
    std::vector<uint8_t> rawData;
};

bool KerdataRead(KerdataArchive& archive, const std::string& filepath);
bool KerdataReadFromMemory(KerdataArchive& archive, const uint8_t* data, size_t size);
bool KerdataWrite(const KerdataArchive& archive, const std::string& filepath);
bool KerdataGetFile(const KerdataArchive& archive, const std::string& path,
                    const uint8_t** outData, size_t* outSize);

} // namespace planet
