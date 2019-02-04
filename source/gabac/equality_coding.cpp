#include "gabac/equality_coding.h"

#include <algorithm>
#include <cassert>
#include <iostream>

#include "gabac/return_codes.h"
#include "gabac/data_stream.h"

/*
int gabac_transformEqualityCoding(
        const uint64_t *const symbols,
        const size_t symbolsSize,
        uint64_t **const equalityFlags,
        uint64_t **const values,
        size_t *const valuesSize
){
    if (symbols == nullptr)
    {
        return GABAC_FAILURE;
    }
    if (equalityFlags == nullptr)
    {
        return GABAC_FAILURE;
    }
    if (values == nullptr)
    {
        return GABAC_FAILURE;
    }
    if (valuesSize == nullptr)
    {
        return GABAC_FAILURE;
    }

    try
    {
        // C++-style vectors to receive input data / accumulate output data
        std::vector<uint64_t> symbolsVector(symbols, (symbols + symbolsSize));
        std::vector<uint64_t> equalityFlagsVector;
        std::vector<uint64_t> valuesVector;

        // Execute
        gabac::transformEqualityCoding(symbolsVector, &equalityFlagsVector, &valuesVector);

        // Extract plain C array data from result vectors
        *valuesSize = valuesVector.size();
        *equalityFlags = static_cast<uint64_t *>(malloc(sizeof(uint64_t) * equalityFlagsVector.size()));
        *values = static_cast<uint64_t *>(malloc(sizeof(uint64_t) * (*valuesSize)));
        std::copy(equalityFlagsVector.begin(), equalityFlagsVector.end(), *equalityFlags);
        std::copy(valuesVector.begin(), valuesVector.end(), *values);
    }
    catch (...)
    {
        return GABAC_FAILURE;
    }

    return GABAC_SUCCESS;
}


int gabac_inverseTransformEqualityCoding(
        const uint64_t *const equalityFlags,
        const size_t equalityFlagsSize,
        const uint64_t *const values,
        const size_t valuesSize,
        uint64_t **const symbols
){
    if (equalityFlags == nullptr)
    {
        return GABAC_FAILURE;
    }
    if (values == nullptr)
    {
        return GABAC_FAILURE;
    }
    if (symbols == nullptr)
    {
        return GABAC_FAILURE;
    }
    try
    {
        // C++-style vectors to receive input data / accumulate output data
        std::vector<uint64_t> equalityFlagsVector(equalityFlags, (equalityFlags + equalityFlagsSize));
        std::vector<uint64_t> valuesVector(values, (values + valuesSize));
        std::vector<uint64_t> symbolsVector;

        // Execute
        gabac::inverseTransformEqualityCoding(equalityFlagsVector, valuesVector, &symbolsVector);

        // Extract plain C array data from result vectors
        *symbols = static_cast<uint64_t *>(malloc(sizeof(uint64_t) * symbolsVector.size()));
        std::copy(symbolsVector.begin(), symbolsVector.end(), *symbols);
    }
    catch (...)
    {
        return GABAC_FAILURE;
    }

    return GABAC_SUCCESS;
}*/


namespace gabac {


void transformEqualityCoding(
        DataStream *const equalityFlags,
        DataStream *const values
){
/*    assert(equalityFlags != nullptr);
    assert(values != nullptr);

    // Prepare the output vectors
    equalityFlags->clear();
    values->clear();

    // Leave everything empty if we do not have any symbols
    if (symbols.empty())
    {
        return;
    }
    uint64_t previousSymbol = 0;
    for (size_t i = 0; i < symbols.size(); ++i)
    {
        uint64_t symbol = symbols.get(i);
        if (symbol == previousSymbol)
        {
            equalityFlags->push_back(1);
        }
        else
        {
            equalityFlags->push_back(0);
            if (symbol > previousSymbol)
            {
                values->push_back(symbol - 1);
            }
            else
            {
                values->push_back(symbol);
            }
            previousSymbol = symbol;
        }
    }*/
}

// ----------------------------------------------------------------------------

void inverseTransformEqualityCoding(
        const DataStream& equalityFlags,
        const DataStream& values,
        DataStream *const symbols
){
    assert(symbols != nullptr);

    // Prepare the output vector
    symbols->clear();

    // Re-compute the symbols from the equality flags and values
    uint64_t previousSymbol = 0;
    size_t valuesIdx = 0;
    for (size_t i = 0; i < equalityFlags.size(); i++)
    {
        if (equalityFlags.get(i) == 1)
        {
            symbols->push_back(previousSymbol);
        }
        else
        {
            if (values.get(valuesIdx) >= previousSymbol)
            {
                symbols->push_back(values.get(valuesIdx) + 1);
            }
            else
            {
                symbols->push_back(values.get(valuesIdx));
            }
            previousSymbol = (*symbols).get(i);
            valuesIdx++;
        }
    }
}


}  // namespace gabac
