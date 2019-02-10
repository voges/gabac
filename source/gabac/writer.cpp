#include "gabac/writer.h"

#include <cassert>
#include <cmath>
#include <cstdint>
#include <limits>

#include "gabac/constants.h"
#include "gabac/context_tables.h"

//
// #include binary_arithmetic_decoder.cpp from here instead of compiling it
// separately, so that we may call inlined member functions of class
// BinaryArithmeticDecoder in this file.
//
#include "binary_arithmetic_encoder.cpp"
 

namespace gabac {


static unsigned int bitLength(
        uint64_t value
){
    unsigned int numBits = 0;

    if (value > 0x7FFF)
    {
        value >>= 16;
        numBits += 16;
    }

    if (value > 0x7F)
    {
        value >>= 8;
        numBits += 8;
    }

    if (value > 0x7)
    {
        value >>= 4;
        numBits += 4;
    }

    if (value > 0x1)
    {
        value >>= 2;
        numBits += 2;
    }

    if (value > 0x0)
    {
        numBits += 1;
    }

    return numBits;
}


Writer::Writer(
        DataBlock *const bitstream
)
        : m_bitOutputStream(bitstream),
        m_contextSelector(),
        m_binaryArithmeticEncoder(m_bitOutputStream),
        m_contextModels(contexttables::buildContextTable()){
}


Writer::~Writer() = default;


void Writer::start(
        size_t numSymbols
){
    assert(numSymbols <= std::numeric_limits<unsigned>::max());
    writeNumSymbols(static_cast<unsigned>(numSymbols));
}


void Writer::reset(){
    m_binaryArithmeticEncoder.flush();
    m_contextModels = contexttables::buildContextTable();
}


void Writer::writeBypassValue(
        uint64_t symbol,
        const BinarizationId& binarizationId,
        const std::vector<unsigned int>& binarizationParameters
){
    // TODO(anyone): might crash if in release mode, because asserts are disabled and wrong parameters might be provided
#ifndef NDEBUG
    constexpr static unsigned int params[unsigned(BinarizationId::STEG) + 1u] = { 1, 1, 0, 0, 1, 1 };
    assert(binarizationParameters.size() >= params[static_cast<unsigned int>(binarizationId)]);
#endif
    switch (binarizationId)
    {
        case BinarizationId::BI:
            writeAsBIbypass(symbol, binarizationParameters[0]);
            break;
        case BinarizationId::TU:
            writeAsTUbypass(symbol, binarizationParameters[0]);
            break;
        case BinarizationId::EG:
            writeAsEGbypass(symbol);
            break;
        case BinarizationId::SEG:
            writeAsSEGbypass(int64_t (symbol));
            break;
        case BinarizationId::TEG:
            writeAsTEGbypass(symbol, binarizationParameters[0]);
            break;
        case BinarizationId::STEG:
            writeAsSTEGbypass(int64_t(symbol), binarizationParameters[0]);
            break;
        default:
            // TODO(Jan): handle default case
            break;
    }
}

void Writer::writeCabacAdaptiveValue(
        uint64_t symbol,
        const BinarizationId& binarizationId,
        const std::vector<unsigned int>& binarizationParameters,
        unsigned int prevValue,
        unsigned int prevPrevValue
){
#ifndef NDEBUG
    constexpr static unsigned int params[unsigned(BinarizationId::STEG) + 1u] = { 1, 1, 0, 0, 1, 1 };
    assert(binarizationParameters.size() >= params[static_cast<unsigned int>(binarizationId)]);
#endif
    unsigned int offset = (prevValue << 2u) + prevPrevValue;
    switch (binarizationId)
    {
        case BinarizationId::BI:
            writeAsBIcabac(
                    symbol,
                    binarizationParameters[0],
                    offset);
            break;
        case BinarizationId::TU:
            writeAsTUcabac(
                    symbol,
                    binarizationParameters[0],
                    offset);
            break;
        case BinarizationId::EG:
            writeAsEGcabac(
                    symbol,
                    offset);
            break;
        case BinarizationId::SEG:
            writeAsSEGcabac(
                    int64_t (symbol),
                    offset);
            break;
        case BinarizationId::TEG:
            writeAsTEGcabac(
                    symbol,
                    binarizationParameters[0],
                    offset);
            break;
        case BinarizationId::STEG:
            writeAsSTEGcabac(
                    int64_t (symbol),
                    binarizationParameters[0],
                    offset);
            break;
        default:
            // TODO(Jan): handle default case
            break;
    }
}


void Writer::writeAsBIbypass(
        uint64_t input,
        unsigned int cLength
){
    assert(binarizationInformation[unsigned(BinarizationId::BI)].sbCheck(input, input, cLength));
    m_binaryArithmeticEncoder.encodeBinsEP(static_cast<unsigned int>(input), cLength);
}


void Writer::writeAsBIcabac(
        uint64_t input,
        unsigned int cLength,
        unsigned int offset
){
    assert(binarizationInformation[unsigned(BinarizationId::BI)].sbCheck(input, input, cLength));

    unsigned int cm = ContextSelector::getContextForBi(offset, 0);
    std::vector<ContextModel>::iterator scan = m_contextModels.begin() + cm;
    for (unsigned int i = 0; i < cLength; i++)
    {
        unsigned int bin = static_cast<unsigned int>(static_cast<uint64_t >(input) >> (cLength - i - 1)) & 0x1u;
        m_binaryArithmeticEncoder.encodeBin(bin, &*(scan++));
    }
}


void Writer::writeAsTUbypass(
        uint64_t input,
        unsigned int cMax
){
    assert(binarizationInformation[unsigned(BinarizationId::TU)].sbCheck(input, input, cMax));

    for (uint64_t i = 0; i < input; i++)
    {
        m_binaryArithmeticEncoder.encodeBinEP(1);
    }
    if (input != cMax)
    {
        m_binaryArithmeticEncoder.encodeBinEP(0);
    }
}


void Writer::writeAsTUcabac(
        uint64_t input,
        unsigned int cMax,
        unsigned int offset
){
    assert(binarizationInformation[unsigned(BinarizationId::TU)].sbCheck(input, input, cMax));

    unsigned int cm = ContextSelector::getContextForTu(offset, 0);

    std::vector<ContextModel>::iterator scan = m_contextModels.begin() + cm;

    for (uint64_t i = 0; i < input; i++)
    {
        m_binaryArithmeticEncoder.encodeBin(1, &*(scan++));
    }

    if (input != cMax)
    {
        m_binaryArithmeticEncoder.encodeBin(0, &*scan);
    }
}


void Writer::writeAsEGbypass(
        uint64_t input
){
    assert(binarizationInformation[unsigned(BinarizationId::EG)].sbCheck(input, input, 0));

    input++;
    unsigned int length = ((bitLength(static_cast<uint64_t>(input)) - 1) << 1u) + 1;
    assert(input <= std::numeric_limits<unsigned>::max());
    m_binaryArithmeticEncoder.encodeBinsEP(static_cast<unsigned >(input), length);
}


void Writer::writeAsEGcabac(
        uint64_t input,
        unsigned int offset
){
    assert(binarizationInformation[unsigned(BinarizationId::EG)].sbCheck(input, input, 0));

    input++;
    unsigned int i = 0;

    unsigned int cm = ContextSelector::getContextForEg(offset, i);
    std::vector<ContextModel>::iterator scan = m_contextModels.begin() + cm;
    unsigned int length = ((bitLength(static_cast<uint64_t>(input)) - 1) << 1u) + 1;
    unsigned int suffixSizeMinus1 = length >> 1u;

    for (; i < suffixSizeMinus1; i++)
    {
        m_binaryArithmeticEncoder.encodeBin(0, &*(scan++));
    }

    if (i < length)
    {
        m_binaryArithmeticEncoder.encodeBin(1, &*scan);
        length -= (i + 1);
        if (length != 0)
        {
            input -= (1u << length);
            assert(input <= std::numeric_limits<unsigned>::max());
            m_binaryArithmeticEncoder.encodeBinsEP(static_cast<unsigned>(input), length);
        }
    }
}


void Writer::writeAsSEGbypass(
        int64_t input
){
    assert(binarizationInformation[unsigned(BinarizationId::SEG)].sbCheck(input, input, 0));
    if (input <= 0)
    {
        writeAsEGbypass(static_cast<unsigned int>(-input) << 1u);
    }
    else
    {
        writeAsEGbypass(static_cast<unsigned int>(static_cast<uint64_t>(input) << 1u) - 1);
    }
}


void Writer::writeAsSEGcabac(
        int64_t input,
        unsigned int offset
){
    assert(binarizationInformation[unsigned(BinarizationId::SEG)].sbCheck(input, input, 0));

    if (input <= 0)
    {
        writeAsEGcabac(static_cast<unsigned int>(-input) << 1u, offset);
    }
    else
    {
        writeAsEGcabac(static_cast<unsigned int>(static_cast<uint64_t>(input) << 1u) - 1, offset);
    }
}


void Writer::writeAsTEGbypass(
        uint64_t input,
        unsigned int cTruncExpGolParam
){
    assert(binarizationInformation[unsigned(BinarizationId::TEG)].sbCheck(input, input, cTruncExpGolParam));

    if (input < cTruncExpGolParam)
    {
        writeAsTUbypass(input, cTruncExpGolParam);
    }
    else
    {
        writeAsTUbypass(cTruncExpGolParam, cTruncExpGolParam);
        writeAsEGbypass(input - cTruncExpGolParam);
    }
}


void Writer::writeAsTEGcabac(
        uint64_t input,
        unsigned int cTruncExpGolParam,
        unsigned int offset
){
    assert(binarizationInformation[unsigned(BinarizationId::TEG)].sbCheck(input, input, cTruncExpGolParam));

    if (input < cTruncExpGolParam)
    {
        writeAsTUcabac(input, cTruncExpGolParam, offset);
    }
    else
    {
        writeAsTUcabac(cTruncExpGolParam, cTruncExpGolParam, offset);
        writeAsEGcabac(input - cTruncExpGolParam, offset);
    }
}


void Writer::writeAsSTEGbypass(
        int64_t input,
        unsigned int cSignedTruncExpGolParam
){
    assert(binarizationInformation[unsigned(BinarizationId::STEG)].sbCheck(input, input, cSignedTruncExpGolParam));

    if (input < 0)
    {
        writeAsTEGbypass(-1 * input, cSignedTruncExpGolParam);
        writeAsBIbypass(1, 1);
    }
    else if (input > 0)
    {
        writeAsTEGbypass(input, cSignedTruncExpGolParam);
        writeAsBIbypass(0, 1);
    }
    else
    {
        writeAsTEGbypass(0, cSignedTruncExpGolParam);
    }
}


void Writer::writeAsSTEGcabac(
        int64_t input,
        unsigned int cSignedTruncExpGolParam,
        unsigned int offset
){
    assert(binarizationInformation[unsigned(BinarizationId::STEG)].sbCheck(input, input, cSignedTruncExpGolParam));

    if (input < 0)
    {
        writeAsTEGcabac(-1 * input, cSignedTruncExpGolParam, offset);
        writeAsBIcabac(1, 1, offset);
    }
    else if (input > 0)
    {
        writeAsTEGcabac(input, cSignedTruncExpGolParam, offset);
        writeAsBIcabac(0, 1, offset);
    }
    else
    {
        writeAsTEGcabac(0, cSignedTruncExpGolParam, offset);
    }
}


void Writer::writeNumSymbols(
        unsigned int numSymbols
){
    writeAsBIbypass(numSymbols, 32);
}


}  // namespace gabac
