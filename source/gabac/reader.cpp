#include "gabac/reader.h"

#include <cassert>
#include <limits>

#include "gabac/constants.h"
#include "gabac/context_tables.h"

//
// #include binary_arithmetic_decoder.cpp from here instead of compiling it
// separately, so that we may call inlined member functions of class
// BinaryArithmeticDecoder in this file.
//
#include "binary_arithmetic_decoder.cpp"

namespace gabac {


Reader::Reader(
        DataBlock* const bitstream
)
        : m_bitInputStream(bitstream),
        m_contextSelector(),
        m_decBinCabac(m_bitInputStream),
        m_contextModels(contexttables::buildContextTable()){
}


Reader::~Reader() = default;


uint64_t Reader::readBypassValue(
        const BinarizationId& binarizationId,
        const std::vector<unsigned int>& binarizationParameters
){
    uint64_t ureturn = 0;
    switch (binarizationId)
    {
        case BinarizationId::BI:
            return readAsBIbypass(binarizationParameters[0]);
        case BinarizationId::TU:
            return readAsTUbypass(binarizationParameters[0]);
        case BinarizationId::EG:
            return readAsEGbypass();
        case BinarizationId::SEG:
            return static_cast<uint64_t >(readAsSEGbypass());
        case BinarizationId::TEG:
            return readAsTEGbypass(binarizationParameters[0]);
        case BinarizationId::STEG:
            return static_cast<uint64_t >(readAsSTEGbypass(binarizationParameters[0]));
        default:
            // TODO(Jan): handle default case
            break;
    }
    assert(false);  // We should not get here
    return ureturn;
}


uint64_t Reader::readAdaptiveCabacValue(
        const BinarizationId& binarizationId,
        const std::vector<unsigned int>& binarizationParameters,
        unsigned int prevValue,
        unsigned int prevPrevValue
){
    unsigned int offset = (prevValue << 2u) + prevPrevValue;
    switch (binarizationId)
    {
        case BinarizationId::BI:
            return readAsBIcabac(
                    binarizationParameters[0],
                    offset
            );
        case BinarizationId::TU:
            return readAsTUcabac(
                    binarizationParameters[0],
                    offset
            );
        case BinarizationId::EG:
            return readAsEGcabac(offset);
        case BinarizationId::SEG:
            return static_cast<uint64_t >(readAsSEGcabac(offset));
        case BinarizationId::TEG:
            return readAsTEGcabac(
                    binarizationParameters[0],
                    offset
            );
        case BinarizationId::STEG:
            return static_cast<uint64_t >(readAsSTEGcabac(
                    binarizationParameters[0],
                    offset
            ));
    }
    assert(false);  // We should not get here
    return 0;
}


uint64_t Reader::readAsBIbypass(
        unsigned int cLength
){
    return m_decBinCabac.decodeBinsEP(cLength);
}


uint64_t Reader::readAsBIcabac(
        unsigned int cLength,
        unsigned int offset
){
    unsigned int bins = 0;
    unsigned int cm = ContextSelector::getContextForBi(offset, 0);
    for (size_t i = 0; i < cLength; i++)
    {
        bins = (bins << 1u) | m_decBinCabac.decodeBin(&m_contextModels[cm++]);
    }
    return bins;
}


uint64_t Reader::readAsTUbypass(
        unsigned int cMax
){
    unsigned int i = 0;
    while (readAsBIbypass(1) == 1)
    {
        i++;
        if (i == cMax)
        {
            break;
        }
    }
    return i;
}


uint64_t Reader::readAsTUcabac(
        unsigned int cMax,
        unsigned int offset
){
    unsigned int i = 0;
    unsigned int cm = ContextSelector::getContextForTu(offset, i);
    while (m_decBinCabac.decodeBin(&m_contextModels[cm]) == 1)
    {
        i++;
        if (cMax == i)
        {
            break;
        }
        else
        {
            cm++;
        }
    }
    return i;
}


uint64_t Reader::readAsEGbypass(){
    unsigned int bins = 0;
    unsigned int i = 0;
    while (readAsBIbypass(1) == 0)
    {
        i++;
    }
    if (i != 0)
    {
        bins = (1u << i) | m_decBinCabac.decodeBinsEP(i);
    }
    else
    {
        return 0;
    }
    return (bins - 1);
}


uint64_t Reader::readAsEGcabac(
        unsigned int offset
){
    unsigned int cm = ContextSelector::getContextForEg(offset, 0);
    unsigned int i = cm;
    while (m_decBinCabac.decodeBin(&m_contextModels[cm]) == 0)
    {
        cm++;
    }
    i = cm - i;
    unsigned int bins = 0;
    if (i != 0)
    {
        bins = (1u << i) | m_decBinCabac.decodeBinsEP(i);
    }
    else
    {
        return 0;
    }
    return (bins - 1);
}


int64_t Reader::readAsSEGbypass(){
    int64_t tmp = readAsEGbypass();
    // Save, only last bit
    if ((static_cast<uint64_t> (tmp) & 0x1u) == 0)
    {
        if (tmp == 0)
        {
            return 0;
        }
        else
        {
            return (-1 * static_cast<int64_t>((static_cast<uint64_t>(tmp) >> 1u)));
        }
    }
    else
    {
        return static_cast<int64_t>((static_cast<uint64_t>(tmp + 1) >> 1u));
    }
}


int64_t Reader::readAsSEGcabac(
        unsigned int offset
){
    int64_t tmp = readAsEGcabac(offset);
    if ((static_cast<uint64_t>(tmp) & 0x1u) == 0)
    {
        if (tmp == 0)
        {
            return 0;
        }
        else
        {
            return (-1 * static_cast<int64_t>(static_cast<uint64_t>(tmp) >> 1u));
        }
    }
    else
    {
        return static_cast<int64_t>((static_cast<uint64_t>(tmp + 1) >> 1u));
    }
}


uint64_t Reader::readAsTEGbypass(
        unsigned int treshold
){
    uint64_t value = readAsTUbypass(treshold);
    if (value == treshold)
    {
        value += readAsEGbypass();
    }
    return value;
}


uint64_t Reader::readAsTEGcabac(
        unsigned int treshold,
        unsigned int offset
){
    uint64_t value = readAsTUcabac(treshold, offset);
    if (value == treshold)
    {
        value += readAsEGcabac(offset);
    }
    return value;
}


int64_t Reader::readAsSTEGbypass(
        unsigned int treshold
){
    int64_t value = readAsTEGbypass(treshold);
    if (value != 0)
    {
        if (readAsBIbypass(1) == 1)
        {
            return -1 * value;
        }
        else
        {
            return value;
        }
    }
    return value;
}


int64_t Reader::readAsSTEGcabac(
        unsigned int treshold,
        unsigned int offset
){
    int64_t value = readAsTEGcabac(treshold, offset);
    if (value != 0)
    {
        if (readAsBIcabac(1, offset) == 1)
        {
            return -1 * value;
        }
        else
        {
            return value;
        }
    }
    return value;
}


size_t Reader::readNumSymbols()
{
    auto result = readAsBIbypass(32);
    return static_cast<size_t>(result);
}


size_t Reader::start()
{
    return readNumSymbols();
}


void Reader::reset()
{
    m_contextModels = contexttables::buildContextTable();
    m_decBinCabac.reset();
}


}  // namespace gabac
