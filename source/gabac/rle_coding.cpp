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
        const uint64_t guard,
        gabac::DataStream * const rawValues,
        gabac::DataStream * const lengths
){
    assert(guard > 0);
    assert(rawValues != nullptr);
    assert(lengths != nullptr);

    // input for rawValues is guaranteed to grow slower than reading process
    // -> in place possible

    lengths->clear();

    size_t rawValuesPos = 0;

    // Do the RLE coding
    for (size_t i = 0; i < rawValues->size();)
    {
        uint64_t prev = rawValues->get(i++);
        uint64_t cur = rawValues->get(i);
        rawValues->set(rawValuesPos++, prev);
        uint64_t lengthValue = 1;
        while ((i < rawValues->size()) && ( cur == prev))
        {
            lengthValue++;
            i++;
            prev = cur;
            cur = rawValues->get(i);
        }
        while (lengthValue > guard)
        {
            lengths->push_back(guard);
            lengthValue -= guard;
        }
        lengths->push_back(lengthValue - 1);
    }
    rawValues->resize(rawValuesPos);
}


void inverseTransformRleCoding(
        const uint64_t guard,
        gabac::DataStream* const rawValues,
        gabac::DataStream* const lengths
){
    assert(rawValues != nullptr);
    assert(!rawValues->empty());
    assert(guard > 0);

    // input for rawValues is not guaranteed to grow slower than reading process
    // -> in place not possible

    gabac::DataStream symbols(0, rawValues->getWordSize());

    // Re-compute the symbol sequence
    size_t j = 0;
    for (size_t i = 0; i < rawValues->size(); ++i)
    {
        uint64_t rawValue = rawValues->get(i);
        uint64_t lengthValue = lengths->get(j++);
        uint64_t totalLengthValue = lengthValue;
        while ((lengthValue != 0) && (totalLengthValue % guard == 0))
        {
            lengthValue = lengths->get(j++);
            totalLengthValue += lengthValue;
        }
        totalLengthValue++;
        while (totalLengthValue > 0)
        {
            totalLengthValue--;
            symbols.push_back(rawValue);
        }
    }

    symbols.swap(rawValues);
    lengths->clear();
    lengths->shrink_to_fit();
}


}  // namespace gabac
