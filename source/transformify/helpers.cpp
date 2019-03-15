#include "transformify/helpers.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <fstream>
#include <limits>
#include <map>


namespace transformify {


bool fileExists(
        const std::string& path
){
    std::ifstream ifs(path);
    return ifs.good();
}


void generateByteBuffer(
        const std::vector<uint64_t>& symbols,
        unsigned int wordSize,
        std::vector<unsigned char> * const buffer
){
    assert((wordSize == 1) || (wordSize == 2) || (wordSize == 4) || (wordSize == 8));
    assert(buffer != nullptr);

    // Prepare the (output) buffer
    buffer->clear();

    switch (wordSize)
    {
        case 1:
        {
            for (const auto& symbol : symbols)
            {
                buffer->push_back(symbol & 0xff);
            }
            break;
        }
        case 2:
        {
            for (const auto& symbol : symbols)
            {
                buffer->push_back(symbol & 0xff);
                buffer->push_back((symbol >> 8u) & 0xff);
            }
            break;
        }
        case 4:
        {
            for (const auto& symbol : symbols)
            {
                buffer->push_back(symbol & 0xff);
                buffer->push_back((symbol >> 8u) & 0xff);
                buffer->push_back((symbol >> 16u) & 0xff);
                buffer->push_back((symbol >> 24u) & 0xff);
            }
            break;
        }
        case 8:
        {
            for (const auto& symbol : symbols)
            {
                buffer->push_back(symbol & 0xff);
                buffer->push_back((symbol >> 8u) & 0xff);
                buffer->push_back((symbol >> 16u) & 0xff);
                buffer->push_back((symbol >> 24u) & 0xff);
                buffer->push_back((symbol >> 32u) & 0xff);
                buffer->push_back((symbol >> 40u) & 0xff);
                buffer->push_back((symbol >> 48u) & 0xff);
                buffer->push_back((symbol >> 56u) & 0xff);
            }
            break;
        }
        default:
        {
            // The default case can happen if assertions are disabled.
            // However, it should never happen and there is nothing we
            // could do. So we bluntly abort the process.
            abort();
        }
    }
}


void generateSymbolStream(
        const std::vector<unsigned char>& buffer,
        unsigned int wordSize,
        std::vector<uint64_t> * const symbols
){
    assert((wordSize == 1) || (wordSize == 2) || (wordSize == 4) || (wordSize == 8));
    assert((buffer.size() % wordSize) == 0);
    assert(symbols != nullptr);

    // Note: as buffer is a vector of unsigned chars no masks (i.e. 0xff) need
    // to be applied before shifting the bits to the right position within a
    // symbol.

    // Prepare the (output) symbols vector
    symbols->clear();
    size_t symbolsSize = buffer.size() / wordSize;

    // Because we resize the symbols vector we have to get out here if the
    // buffer is empty.
    if (buffer.empty())
    {
        return;
    }
    symbols->resize(symbolsSize);

    // Multiplex every wordSize bytes into one symbol
    size_t symbolsIdx = 0;
    for (size_t i = 0; i < buffer.size(); i += wordSize)
    {
        uint64_t symbol = 0;
        switch (wordSize)
        {
            case 1:
            {
                symbol = buffer[i];
                break;
            }
            case 2:
            {
                symbol = static_cast<uint16_t>(buffer[i + 1]) << 8u;
                symbol |= static_cast<uint16_t>(buffer[i]);
                break;
            }
            case 4:
            {
                symbol = static_cast<uint32_t>(buffer[i + 3]) << 24u;
                symbol |= static_cast<uint32_t>(buffer[i + 2]) << 16u;
                symbol |= static_cast<uint32_t>(buffer[i + 1]) << 8u;
                symbol |= static_cast<uint32_t>(buffer[i]);
                break;
            }
            case 8:
            {
                symbol = static_cast<uint64_t>(buffer[i + 7]) << 56u;
                symbol |= static_cast<uint64_t>(buffer[i + 6]) << 48u;
                symbol |= static_cast<uint64_t>(buffer[i + 5]) << 40u;
                symbol |= static_cast<uint64_t>(buffer[i + 4]) << 32u;
                symbol |= static_cast<uint64_t>(buffer[i + 3]) << 24u;
                symbol |= static_cast<uint64_t>(buffer[i + 2]) << 16u;
                symbol |= static_cast<uint64_t>(buffer[i + 1]) << 8u;
                symbol |= static_cast<uint64_t>(buffer[i]);
                break;
            }
            default:
            {
                // The default case can happen if assertions are disabled.
                // However, it should never happen and there is nothing we
                // could do. So we bluntly abort the process.
                abort();
            }
        }
        (*symbols)[symbolsIdx++] = symbol;
    }
}


}  // namespace transformify
