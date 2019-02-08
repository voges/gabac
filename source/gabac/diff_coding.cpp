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
        DataStream symbolsVector(symbols, (symbols + symbolsSize));
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
        DataStream *const transformedSymbols
){
    assert(transformedSymbols != nullptr);

    // Do the diff coding
    uint64_t previousSymbol = 0;
    StreamReader r = transformedSymbols->getReader();
    while(r.isValid()) {
        uint64_t symbol = r.get();
        uint64_t diff = symbol - previousSymbol;
        r.set(diff);
        previousSymbol = symbol;
        r.inc();
    }
}


void inverseTransformDiffCoding(
        DataStream *const symbols
){
    assert(symbols != nullptr);

    // Re-compute the symbols from the differences
    uint64_t previousSymbol = 0;
    StreamReader r = symbols->getReader();
    while(r.isValid()) {
        uint64_t symbol = r.get();
        r.set(previousSymbol + symbol);
        previousSymbol = previousSymbol + symbol;
        r.inc();
    }
}


}  // namespace gabac
