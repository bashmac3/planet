#include "bundler/kerdata.h"
#include <fstream>
#include <cstring>
#include "planet/core/logger.h"

namespace planet {

bool KerdataRead(KerdataArchive& archive, const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        LOG_ERROR() << "[Kerdata] Cannot open: " << filepath;
        return false;
    }
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buf(size);
    file.read(reinterpret_cast<char*>(buf.data()), size);
    file.close();

    return KerdataReadFromMemory(archive, buf.data(), buf.size());
}

bool KerdataReadFromMemory(KerdataArchive& archive, const uint8_t* data, size_t size) {
    if (!data || size < sizeof(KerdataHeader)) return false;

    const auto* hdr = reinterpret_cast<const KerdataHeader*>(data);
    if (hdr->magic != KERDATA_MAGIC) {
        LOG_ERROR() << "[Kerdata] Bad magic: 0x" << std::hex << hdr->magic;
        return false;
    }
    if (hdr->version != KERDATA_VERSION) {
        LOG_ERROR() << "[Kerdata] Wrong version: " << hdr->version << " (expected " << KERDATA_VERSION << ")";
        return false;
    }

    archive.header = *hdr;
    archive.entries.clear();
    archive.pathIndex.clear();

    uint32_t fcnt = hdr->fileCount;
    // Name length table starts right after header
    const uint8_t* nameLenTable = data + sizeof(KerdataHeader);
    // Entry table starts at tableOffset
    const uint8_t* tbl = data + hdr->tableOffset;

    if (hdr->tableOffset + fcnt * sizeof(uint32_t) * 4 > size) return false;

    std::vector<uint32_t> nameLengths(fcnt);
    std::memcpy(nameLengths.data(), nameLenTable, fcnt * sizeof(uint32_t));

    for (uint32_t i = 0; i < fcnt; i++) {
        KerdataEntry e;
        if (tbl - data + nameLengths[i] > static_cast<ptrdiff_t>(size)) return false;
        e.path.assign(reinterpret_cast<const char*>(tbl), nameLengths[i]);
        // Skip name + alignment padding (aligned to 4 bytes)
        uint32_t nameAligned = ((nameLengths[i] + 3) / 4) * 4;
        tbl += nameAligned;
        e.dataOffset = *reinterpret_cast<const uint32_t*>(tbl); tbl += 4;
        e.dataSize   = *reinterpret_cast<const uint32_t*>(tbl); tbl += 4;
        e.flags      = *reinterpret_cast<const uint32_t*>(tbl); tbl += 4;
        archive.entries.push_back(e);
        archive.pathIndex[e.path] = i;
    }

    uint32_t rawSize = (hdr->dataOffset > 0) ? (static_cast<uint32_t>(size) - hdr->dataOffset) : 0;
    rawSize = (rawSize > hdr->totalDataSize) ? hdr->totalDataSize : rawSize;
    archive.rawData.resize(rawSize);
    if (rawSize > 0) {
        std::memcpy(archive.rawData.data(), data + hdr->dataOffset, rawSize);
    }

    return true;
}

static uint32_t align4(uint32_t val) { return (val + 3) & ~3u; }

bool KerdataWrite(const KerdataArchive& archive, const std::string& filepath) {
    std::ofstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        LOG_ERROR() << "[Kerdata] Cannot write: " << filepath;
        return false;
    }

    KerdataHeader hdr = archive.header;
    hdr.magic   = KERDATA_MAGIC;
    hdr.version  = KERDATA_VERSION;
    hdr.fileCount = static_cast<uint32_t>(archive.entries.size());

    // Layout:
    //   Header (28 bytes)
    //   Name length table (fcnt * 4 bytes)
    //   Entry table (aligned names + 12 bytes per entry)
    //   Raw data
    uint32_t nameLenTableSize = hdr.fileCount * sizeof(uint32_t);
    hdr.tableOffset = sizeof(KerdataHeader) + nameLenTableSize;

    uint32_t entryTableSize = 0;
    for (const auto& e : archive.entries) {
        entryTableSize += ((static_cast<uint32_t>(e.path.size()) + 3) / 4) * 4; // aligned name
        entryTableSize += 12; // dataOffset(4) + dataSize(4) + flags(4)
    }

    hdr.dataOffset = hdr.tableOffset + entryTableSize;
    hdr.totalDataSize = static_cast<uint32_t>(archive.rawData.size());

    // Write header
    file.write(reinterpret_cast<const char*>(&hdr), sizeof(KerdataHeader));

    // Write name length table
    for (const auto& e : archive.entries) {
        uint32_t nl = static_cast<uint32_t>(e.path.size());
        file.write(reinterpret_cast<const char*>(&nl), sizeof(uint32_t));
    }

    // Write entry table
    for (const auto& e : archive.entries) {
        file.write(e.path.c_str(), e.path.size());
        // pad to 4-byte alignment
        uint32_t alignedLen = ((static_cast<uint32_t>(e.path.size()) + 3) / 4) * 4;
        uint32_t pad = alignedLen - static_cast<uint32_t>(e.path.size());
        for (uint32_t p = 0; p < pad; p++) file.put('\0');
        file.write(reinterpret_cast<const char*>(&e.dataOffset), sizeof(uint32_t));
        file.write(reinterpret_cast<const char*>(&e.dataSize),   sizeof(uint32_t));
        file.write(reinterpret_cast<const char*>(&e.flags),      sizeof(uint32_t));
    }

    // Write raw data
    if (!archive.rawData.empty()) {
        file.write(reinterpret_cast<const char*>(archive.rawData.data()), archive.rawData.size());
    }

    file.close();
    LOG_INFO() << "[Kerdata] Wrote " << filepath << " (" << hdr.fileCount << " files, " << hdr.totalDataSize << " bytes data)";
    return true;
}

bool KerdataGetFile(const KerdataArchive& archive, const std::string& path,
                    const uint8_t** outData, size_t* outSize) {
    auto it = archive.pathIndex.find(path);
    if (it == archive.pathIndex.end()) return false;
    const auto& e = archive.entries[it->second];
    if (e.dataOffset + e.dataSize > archive.rawData.size()) return false;
    *outData = archive.rawData.data() + e.dataOffset;
    *outSize = e.dataSize;
    return true;
}

} // namespace planet
