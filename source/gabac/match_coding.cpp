#include "gabac/match_coding.h"

#include <cassert>
#include <algorithm>
#include <iostream>

#include "gabac/return_codes.h"


// ----------------------------------------------------------------------------
// C wrapper BEGIN
// ----------------------------------------------------------------------------

/*
int gabac_transformMatchCoding(
        const uint64_t *const symbols,
        const size_t symbolsSize,
        const uint32_t windowSize,
        uint64_t **const pointers,
        size_t *const pointersSize,
        uint64_t **const lengths,
        size_t *const lengthsSize,
        uint64_t **const rawValues,
        size_t *const rawValuesSize
){
    if (symbols == nullptr)
    {
        return GABAC_FAILURE;
    }
    if (pointers == nullptr)
    {
        return GABAC_FAILURE;
    }
    if (pointersSize == nullptr)
    {
        return GABAC_FAILURE;
    }
    if (lengths == nullptr)
    {
        return GABAC_FAILURE;
    }
    if (lengthsSize == nullptr)
    {
        return GABAC_FAILURE;
    }
    if (rawValues == nullptr)
    {
        return GABAC_FAILURE;
    }
    if (rawValuesSize == nullptr)
    {
        return GABAC_FAILURE;
    }

    // C++-style vectors to receive input data / accumulate output data
    gabac::DataStream symbolsVector(symbols, (symbols + symbolsSize));
    gabac::DataStream pointersVector;
    gabac::DataStream lengthsVector;
    gabac::DataStream rawValuesVector;

    // Execute
    try
    {
        gabac::transformMatchCoding(
                symbolsVector,
                windowSize,
                &pointersVector,
                &lengthsVector,
                &rawValuesVector
        );
    }
    catch (...)
    {
        return GABAC_FAILURE;
    }

    // Extract plain C array data from result vectors
    *pointersSize = pointersVector.size();
    *lengthsSize = lengthsVector.size();
    *rawValuesSize = rawValuesVector.size();
    *pointers = static_cast<uint64_t *> (malloc(sizeof(uint64_t) * (*pointersSize)));
    *lengths = static_cast<uint64_t *> (malloc(sizeof(uint64_t) * (*lengthsSize)));
    *rawValues = static_cast<uint64_t *>(malloc(sizeof(uint64_t) * (*rawValuesSize)));
    std::copy(pointersVector.begin(), pointersVector.end(), *pointers);
    std::copy(lengthsVector.begin(), lengthsVector.end(), *lengths);
    std::copy(rawValuesVector.begin(), rawValuesVector.end(), *rawValues);

    return GABAC_SUCCESS;
}

// ----------------------------------------------------------------------------

int gabac_inverseTransformMatchCoding(
        const uint64_t *const pointers,
        const size_t pointersSize,
        const uint64_t *const lengths,
        const size_t lengthsSize,
        const uint64_t *const rawValues,
        const size_t rawValuesSize,
        uint64_t **const symbols,
        size_t *const symbolsSize
){
    if (pointers == nullptr)
    {
        return GABAC_FAILURE;
    }
    if (lengths == nullptr)
    {
        return GABAC_FAILURE;
    }
    if (rawValues == nullptr)
    {
        return GABAC_FAILURE;
    }
    if (symbols == nullptr)
    {
        return GABAC_FAILURE;
    }
    if (symbolsSize == nullptr)
    {
        return GABAC_FAILURE;
    }

    // C++-style vectors to receive input data / accumulate output data
    gabac::DataStream pointersVector(pointers, (pointers + pointersSize));
    gabac::DataStream lengthsVector(lengths, (lengths + lengthsSize));
    gabac::DataStream rawValuesVector(
            rawValues,
            (rawValues + rawValuesSize)
    );
    gabac::DataStream symbolsVector;

    // Execute
    try
    {
        gabac::inverseTransformMatchCoding(
                pointersVector,
                lengthsVector,
                rawValuesVector,
                &symbolsVector
        );
    }
    catch (...)
    {
        return GABAC_FAILURE;
    }

    // Extract plain C array data from result vectors
    *symbolsSize = symbolsVector.size();
    *symbols = static_cast<uint64_t *>(malloc(sizeof(uint64_t) * (*symbolsSize)));
    std::copy(symbolsVector.begin(), symbolsVector.end(), *symbols);

    return GABAC_SUCCESS;
}*/


// ----------------------------------------------------------------------------
// C wrapper END
// ----------------------------------------------------------------------------


namespace gabac {

//TODO: Review if in place possible
void transformMatchCoding(
        const uint32_t windowSize,
        gabac::DataStream *const symbols,
        gabac::DataStream *const pointers,
        gabac::DataStream *const lengths
){
    assert(pointers != nullptr);
    assert(lengths != nullptr);
    assert(symbols != nullptr);
    gabac::DataStream rawValues(0, symbols->getWordSize());

    // Prepare the output vectors
    pointers->clear();
    lengths->clear();

    if (windowSize == 0)
    {
        lengths->resize(symbols->size());
        std::fill(lengths->begin(), lengths->end(), 0);
        return;
    }

    // Do the match coding
    const uint64_t symbolsSize = symbols->size();
    for (uint64_t i = 0; i < symbolsSize; i++)
    {
        uint64_t pointer = 0;
        uint64_t length = 0;
        uint64_t windowStartIdx = i - windowSize;
        uint64_t windowEndIdx = i;

        for (uint64_t w = windowStartIdx; w < windowEndIdx; w++)
        {
            uint64_t offset = i;
            while (
                    (offset < symbolsSize)
                    && (symbols->get(offset) == (symbols->get(w + offset - i))))
            {
                offset++;
            }
            offset -= i;
            if (offset >= length)
            {
                length = offset;
                pointer = w;
            }
        }
        if (length < 2)
        {
            lengths->push_back(0);
            rawValues.push_back(symbols->get(i));
        }
        else
        {
            pointers->push_back(i - pointer);
            lengths->push_back(length);
            i += (length - 1);
        }
    }

    rawValues.swap(symbols);
}

// ----------------------------------------------------------------------------

void inverseTransformMatchCoding(
        gabac::DataStream* const rawValues,
        gabac::DataStream* const pointers,
        gabac::DataStream* const lengths
){
    gabac::DataStream symbols(0, rawValues->size());
    assert(lengths->size() == pointers->size() + rawValues->size());


    // Re-compute the symbols from the pointer, lengths and raw values
    size_t n = 0;
    size_t t0 = 0;
    size_t t1 = 0;
    size_t t2 = 0;
    for (t1 = 0; t1 < lengths->size(); t1++)
    {
        uint64_t length = lengths->get(t1);
        if (length == 0)
        {
            symbols.push_back(rawValues->get(t2++));
            n++;
        }
        else
        {
            uint64_t pointer = pointers->get(t0++);
            for (uint64_t l = 0; l < length; l++)
            {
                symbols.push_back(symbols.get(n - pointer));
                n++;
            }
        }
    }

    symbols.swap(rawValues);
    pointers->clear();
    pointers->shrink_to_fit();
    lengths->clear();
    lengths->shrink_to_fit();

}

// ----------------------------------------------------------------------------

}  // namespace gabac

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
