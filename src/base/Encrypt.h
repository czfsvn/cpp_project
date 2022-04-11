#pragma once

#include <iostream>

namespace encrypt
{
    // static class
    namespace md5
    {
        // 128 bit binary data
        std::string md5_bin(const std::string& str);
        // 128 bit hex string style, lower case
        std::string md5_string_32(const std::string& str);
        // 64  bit hex string style, lower case
        std::string md5_string_16(const std::string& str);

        // 128 bit integer style
        std::pair<uint64_t, uint64_t> md5_integer_32(const std::string& str);
        // 64  bit integer style
        uint64_t md5_integer_16(const std::string& str);
    };  // namespace md5

    namespace crc32c
    {

        // Return the crc32c of concat(A, data[0,n-1]) where init_crc is the
        // crc32c of some string A.  Extend() is often used to maintain the
        // crc32c of a stream of data.
        uint32_t Extend(uint32_t init_crc, const char* data, size_t n);

        // Return the crc32c of data[0,n-1]
        inline uint32_t Value(const char* data, size_t n)
        {
            return Extend(0, data, n);
        }

        static const uint32_t kMaskDelta = 0xa282ead8ul;

        // Return a masked representation of crc.
        //
        // Motivation: it is problematic to compute the CRC of a string that
        // contains embedded CRCs.  Therefore we recommend that CRCs stored
        // somewhere (e.g., in files) should be masked before being stored.
        inline uint32_t Mask(uint32_t crc)
        {
            // Rotate right by 15 bits and add a constant.
            return ((crc >> 15) | (crc << 17)) + kMaskDelta;
        }

        // Return the crc whose masked representation is masked_crc.
        inline uint32_t Unmask(uint32_t masked_crc)
        {
            uint32_t rot = masked_crc - kMaskDelta;
            return ((rot >> 17) | (rot << 15));
        }

    }  // namespace crc32c
}  // namespace encrypt