//
// Created by qiuyudai on 2023/12/6.
//

#ifndef NETKIT_ENDIAN_H
#define NETKIT_ENDIAN_H

#include <arpa/inet.h>
#include <cstdint>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wold-style-cast"

// Helper function for 64-bit conversion
inline uint64_t htonLL(uint64_t val) {
    static const int num = 42;

    // Check the endianness
    if (*reinterpret_cast<const char *>(&num) == num) {
        const uint32_t high_part = htonl(static_cast<uint32_t>(val >> 32));
        const uint32_t low_part = htonl(static_cast<uint32_t>(val & 0xFFFFFFFFLL));

        return (static_cast<uint64_t>(low_part) << 32) | high_part;
    } else {
        return val;
    }
}

inline uint64_t ntohLL(uint64_t val) {
    return htonLL(val); // ntohll is the same as htonll
}

inline uint64_t hostToNetwork64(uint64_t host64) {
    return htonLL(host64);
}

inline uint32_t hostToNetwork32(uint32_t host32) {
    return htonl(host32);
}

inline uint16_t hostToNetwork16(uint16_t host16) {
    return htons(host16);
}

inline uint64_t networkToHost64(uint64_t net64) {
    return ntohLL(net64);
}

inline uint32_t networkToHost32(uint32_t net32) {
    return ntohl(net32);
}

inline uint16_t networkToHost16(uint16_t net16) {
    return ntohs(net16);
}

#pragma GCC diagnostic pop


#endif //NETKIT_ENDIAN_H
