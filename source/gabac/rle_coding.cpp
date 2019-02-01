#include "gabac/rle_coding.h"

#include <algorithm>
#include <cassert>

#include "gabac/return_codes.h"

/*

int gabac_transformRleCoding(
        const uint64_t * const symbols,
        const size_t symbolsSize,
        const uint64_t guard,
        uint64_t ** const rawValues,
        size_t * const rawValuesSize,
        uint64_t ** const lengths,
        size_t * const lengthsSize
){
    if (symbols == nullptr) { return GABAC_FAILURE; }
    if (rawValues == nullptr) { return GABAC_FAILURE; }
    if (rawValuesSize == nullptr) { return GABAC_FAILURE; }
    if (lengths == nullptr) { return GABAC_FAILURE; }
    if (lengthsSize == nullptr) { return GABAC_FAILURE; }

    try
    {
        // C++-style vectors to receive input data / accumulate output data
        gabac::DataStream symbolsVector(symbols, (symbols + symbolsSize));
        gabac::DataStream rawValuesVector;
        gabac::DataStream lengthsVector;

        // Execute
        gabac::transformRleCoding(symbolsVector, guard, &rawValuesVector, &lengthsVector);

        // Extract plain C array data from result vectors
        *rawValuesSize = rawValuesVector.size();
        *lengthsSize = lengthsVector.size();
        *rawValues = static_cast<uint64_t *>(malloc(sizeof(uint64_t) * (*rawValuesSize)));
        *lengths = static_cast<uint64_t *>(malloc(sizeof(uint64_t) * (*lengthsSize)));
        std::copy(rawValuesVector.begin(), rawValuesVector.end(), *rawValues);
        std::copy(lengthsVector.begin(), lengthsVector.end(), *lengths);
    }
    catch (...)
    {
        return GABAC_FAILURE;
    }

    return GABAC_SUCCESS;
}


int gabac_inverseTransformRleCoding(
        const uint64_t * const rawValues,
        const size_t rawValuesSize,
        const uint64_t * const lengths,
        const size_t lengthsSize,
        const uint64_t guard,
        uint64_t ** const symbols,
        size_t * const symbolsSize
){
    if (rawValues == nullptr) { return GABAC_FAILURE; }
    if (lengths == nullptr) { return GABAC_FAILURE; }
    if (symbols == nullptr) { return GABAC_FAILURE; }
    if (symbolsSize == nullptr) { return GABAC_FAILURE; }

    try
    {
        // C++-style vectors to receive input data / accumulate output data
        gabac::DataStream rawValuesVector(rawValues, (rawValues + rawValuesSize));
        gabac::DataStream lengthsVector(lengths, (lengths + lengthsSize));
        gabac::DataStream symbolsVector;

        // Execute
        gabac::inverseTransformRleCoding(rawValuesVector, lengthsVector, guard, &symbolsVector);

        // Extract plain C array data from result vectors
        *symbolsSize = symbolsVector.size();
        *symbols = static_cast<uint64_t *>(malloc(sizeof(uint64_t) * (*symbolsSize)));
        std::copy(symbolsVector.begin(), symbolsVector.end(), *symbols);
    }
    catch (...)
    {
        return GABAC_FAILURE;
    }

    return GABAC_SUCCESS;
}*/


namespace gabac {


void transformRleCoding(
        const gabac::DataStream& symbols,
        const uint64_t guard,
        gabac::DataStream * const rawValues,
        gabac::DataStream * const lengths
){
    assert(guard > 0);
    assert(rawValues != nullptr);
    assert(lengths != nullptr);

    // Prepare the output vectors
    rawValues->clear();
    lengths->clear();

    // Do the RLE coding
    for (size_t i = 0; i < symbols.size();)
    {
        rawValues->push_back(symbols[i++]);
        uint64_t lengthValue = 1;
        while ((i < symbols.size()) && (symbols[i] == symbols[i - 1]))
        {
            lengthValue++;
            i++;
        }
        while (lengthValue > guard)
        {
            lengths->push_back(guard);
            lengthValue -= guard;
        }
        lengths->push_back(lengthValue - 1);
    }
}


void inverseTransformRleCoding(
        const gabac::DataStream& rawValues,
        const gabac::DataStream& lengths,
        const uint64_t guard,
        gabac::DataStream * const symbols
){
    assert(!rawValues.empty());
    assert(!lengths.empty());
    assert(guard > 0);
    assert(symbols != nullptr);

    // Prepare the output vectors
    symbols->clear();

    // Re-compute the symbol sequence
    size_t j = 0;
    for (const auto& rawValue : rawValues)
    {
        uint64_t lengthValue = lengths.at(j++);
        uint64_t totalLengthValue = lengthValue;
        while ((lengthValue != 0) && (totalLengthValue % guard == 0))
        {
            lengthValue = lengths.at(j++);
            totalLengthValue += lengthValue;
        }
        totalLengthValue++;
        while (totalLengthValue > 0)
        {
            totalLengthValue--;
            symbols->push_back(rawValue);
        }
    }
}


}  // namespace gabac
