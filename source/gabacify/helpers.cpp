#include "gabacify/helpers.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <fstream>
#include <limits>
#include <map>


namespace gabacify {

template<typename T>
void deriveMinMax(  // warning: this should still support signed!!!!
        const gabac::DataStream& symbols,
        unsigned int word_size,
        T *const min,
        T *const max
){
    if (symbols.empty())
    {
        *min = 0;
        *max = 0;
        return;
    }

    // [wordsize, sign, min, max]
    static const std::array<std::array<T, 3>, 4>
            aborts =
            {{{1, std::numeric_limits<typename std::conditional<std::is_signed<T>::value,
                    int8_t, uint8_t>::type>::min(),
                      std::numeric_limits<typename std::conditional<std::is_signed<T>::value,
                              int8_t, uint8_t>::type>::max()},
                     {2, std::numeric_limits<typename std::conditional<std::is_signed<T>::value,
                             int16_t, uint16_t>::type>::min(),
                             std::numeric_limits<typename std::conditional<std::is_signed<T>::value,
                                     int16_t, uint16_t>::type>::max()},
                     {4, std::numeric_limits<typename std::conditional<std::is_signed<T>::value,
                             int32_t, uint32_t>::type>::min(),
                             std::numeric_limits<typename std::conditional<std::is_signed<T>::value,
                                     int32_t, uint32_t>::type>::max()},
                     {8, std::numeric_limits<typename std::conditional<std::is_signed<T>::value,
                             int64_t, uint64_t>::type>::min(),
                             std::numeric_limits<typename std::conditional<std::is_signed<T>::value,
                                     int64_t, uint64_t>::type>::max()}}};
    std::array<T, 3> curstate{};

    auto it = std::find_if(
            aborts.begin(), aborts.end(), [&word_size](const std::array<T, 3>& c) -> bool
            {
                return c[0] == word_size;
            }
    );

    assert(it != aborts.end());
    curstate = *it;
    *min = (*it)[2];
    *max = (*it)[1];

    for (size_t i = 0; i < symbols.size(); ++i)
    {
        uint64_t symbol = symbols.get(i);
        if (symbol < *min)
        {
            *min = symbol;
        }
        if (symbol > *max)
        {
            *max = symbol;
        }
        std::array<T, 3> conf = {word_size, *min, *max};
        if (curstate == conf)
        {
            return;
        }
    }
}


void deriveMinMaxSigned(
        const gabac::DataStream& symbols,
        unsigned int word_size,
        int64_t *const min,
        int64_t *const max
){
    deriveMinMax(symbols, word_size, min, max);
}


void deriveMinMaxUnsigned(
        const gabac::DataStream& symbols,
        unsigned int word_size,
        uint64_t *const min,
        uint64_t *const max
){
    *min = std::numeric_limits<uint64_t>::max();
    *max = std::numeric_limits<uint64_t >::min();
    for(size_t i = 0; i < symbols.size(); ++i) {
        uint64_t v = symbols.get(i);
        if(v < *min) {
            *min = v;
        }
        if(v > *max) {
            *max = v;
        }
    }
}


bool fileExists(
        const std::string& path
){
    std::ifstream ifs(path);
    return ifs.good();
}


void generateByteBuffer(
        const gabac::DataStream& symbols,
        unsigned int wordSize,
        gabac::DataStream * const buffer
){
    assert((wordSize == 1) || (wordSize == 2) || (wordSize == 4) || (wordSize == 8));
    assert(buffer != nullptr);

    // Prepare the (output) buffer
    buffer->clear();

    switch (wordSize)
    {
        case 1:
        {
            for (size_t i = 0; i < symbols.size(); ++i)
            {
                uint64_t symbol = symbols.get(i);
                buffer->push_back(symbol & 0xff);
            }
            break;
        }
        case 2:
        {
            for (size_t i = 0; i < symbols.size(); ++i)
            {
                uint64_t symbol = symbols.get(i);
                buffer->push_back(symbol & 0xff);
                buffer->push_back((symbol >> 8u) & 0xff);
            }
            break;
        }
        case 4:
        {
            for (size_t i = 0; i < symbols.size(); ++i)
            {
                uint64_t symbol = symbols.get(i);
                buffer->push_back(symbol & 0xff);
                buffer->push_back((symbol >> 8u) & 0xff);
                buffer->push_back((symbol >> 16u) & 0xff);
                buffer->push_back((symbol >> 24u) & 0xff);
            }
            break;
        }
        case 8:
        {
            for (size_t i = 0; i < symbols.size(); ++i)
            {
                uint64_t symbol = symbols.get(i);
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
        const gabac::DataStream& buffer,
        unsigned int wordSize,
        gabac::DataStream * const symbols
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
                symbol = buffer.get(i);
                break;
            }
            case 2:
            {
                symbol = static_cast<uint16_t>(buffer.get(i + 1)) << 8u;
                symbol |= static_cast<uint16_t>(buffer.get(i));
                break;
            }
            case 4:
            {
                symbol = static_cast<uint32_t>(buffer.get(i + 3)) << 24u;
                symbol |= static_cast<uint32_t>(buffer.get(i + 2)) << 16u;
                symbol |= static_cast<uint32_t>(buffer.get(i + 1)) << 8u;
                symbol |= static_cast<uint32_t>(buffer.get(i));
                break;
            }
            case 8:
            {
                symbol = static_cast<uint64_t>(buffer.get(i + 7)) << 56u;
                symbol |= static_cast<uint64_t>(buffer.get(i + 6)) << 48u;
                symbol |= static_cast<uint64_t>(buffer.get(i + 5)) << 40u;
                symbol |= static_cast<uint64_t>(buffer.get(i + 4)) << 32u;
                symbol |= static_cast<uint64_t>(buffer.get(i + 3)) << 24u;
                symbol |= static_cast<uint64_t>(buffer.get(i + 2)) << 16u;
                symbol |= static_cast<uint64_t>(buffer.get(i + 1)) << 8u;
                symbol |= static_cast<uint64_t>(buffer.get(i));
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
        (*symbols).set(symbolsIdx++, symbol);
    }
}


double shannonEntropy(
        const gabac::DataStream& data
){
    size_t size = data.size();
    double entropy = 0;
    std::map<uint64_t, uint64_t> counts;
    typename std::map<uint64_t, uint64_t>::iterator it;
    //
    for (size_t dataIndex = 0; dataIndex < size; ++dataIndex)
    {
        counts[data.get(dataIndex)]++;
    }
    //
    it = counts.begin();
    while (it != counts.end())
    {
        auto p_x = static_cast<double>(it->second) / size;
        if (p_x > 0)
        {
            entropy -= p_x * std::log(p_x) / std::log(2);
        }
        ++it;
    }
    return entropy;
}


}  // namespace gabacify
