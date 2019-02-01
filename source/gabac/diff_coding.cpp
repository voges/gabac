#include "gabac/diff_coding.h"

#include <algorithm>
#include <cassert>
#include <limits>

#include "gabac/return_codes.h"


/*int gabac_transformDiffCoding(
        const uint64_t *const symbols,
        const size_t symbolsSize,
        int64_t **const transformedSymbols
){
    if (symbols == nullptr)
    {
        return GABAC_FAILURE;
    }
    if (transformedSymbols == nullptr)
    {
        return GABAC_FAILURE;
    }
    try
    {
        // C++-style vectors to receive input data / accumulate output data
        std::vector<uint64_t> symbolsVector(symbols, (symbols + symbolsSize));
        std::vector<int64_t> transformedSymbolsVector;

        // Execute
        gabac::transformDiffCoding(symbolsVector, &transformedSymbolsVector);

        // Extract plain C array data from result vectors
        *transformedSymbols = static_cast<int64_t *>(malloc(sizeof(int64_t) * transformedSymbolsVector.size()));
        std::copy(transformedSymbolsVector.begin(), transformedSymbolsVector.end(), *transformedSymbols);
    }
    catch (...)
    {
        return GABAC_FAILURE;
    }

    return GABAC_SUCCESS;
}


int gabac_inverseTransformDiffCoding(
        const int64_t *const transformedSymbols,
        const size_t transformedSymbolsSize,
        uint64_t **const symbols
){
    if (transformedSymbols == nullptr)
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
        std::vector<int64_t>
                transformedSymbolsVector(transformedSymbols, (transformedSymbols + transformedSymbolsSize));
        std::vector<uint64_t> symbolsVector;

        // Execute
        gabac::inverseTransformDiffCoding(transformedSymbolsVector, &symbolsVector);

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


void transformDiffCoding(
        const DataStream& symbols,
        DataStream *const transformedSymbols
){
    assert(transformedSymbols != nullptr);

    // Prepare the output vector
    transformedSymbols->clear();
    transformedSymbols->resize(symbols.size());

    // Do the diff coding
    uint64_t previousSymbol = 0;
    for (size_t i = 0; i < symbols.size(); i++)
    {
#ifndef NDEBUG
        uint64_t diff = 0;
        if (previousSymbol < symbols[i])
        {
            diff = symbols[i] - previousSymbol;
            assert(diff <= static_cast<uint64_t>(std::numeric_limits<int64_t>::max()));
        }
        else  // previousSymbol >= symbols[i]
        {
            diff = previousSymbol - symbols[i];
            assert(diff <= static_cast<uint64_t>(std::numeric_limits<int64_t>::max()) + 1);
        }
#endif  // NDEBUG
        (*transformedSymbols)[i] = symbols[i] - previousSymbol;
        previousSymbol = symbols[i];
    }
}


void inverseTransformDiffCoding(
        const DataStream& transformedSymbols,
        DataStream *const symbols
){
    assert(symbols != nullptr);

    // Prepare the output vector
    symbols->resize(transformedSymbols.size());

    // Re-compute the symbols from the differences
    uint64_t previousSymbol = 0;
    for (size_t i = 0; i < transformedSymbols.size(); i++)
    {
#ifndef NDEBUG
        if (transformedSymbols[i] < 0)
        {
            if (transformedSymbols[i] == std::numeric_limits<int64_t>::min())
            {
                assert(previousSymbol >= static_cast<uint64_t>(std::numeric_limits<int64_t>::max()));
            }
            assert(previousSymbol >= static_cast<uint64_t>(-1 * transformedSymbols[i]));
        }
        else  // transformedSymbols[i] >= 0
        {
            assert(std::numeric_limits<uint64_t>::max() - previousSymbol >=
                   static_cast<uint64_t>(transformedSymbols[i]));
        }
#endif  // NDEBUG
        (*symbols)[i] = previousSymbol + transformedSymbols[i];
        previousSymbol = (*symbols)[i];
    }
}


}  // namespace gabac
