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

// Optimized for wordsize 1. In place for equality flags
static void transformEqualityCoding0(
        DataStream *const values,
        DataStream *const equalityFlags
){
    uint64_t previousSymbol = 0;
    *equalityFlags = DataStream(0, values->getWordSize());

    StreamReader r = values->getReader();
    // Treat value as equalityFlags and vice versa
    while(r.isValid()){
        uint64_t symbol = r.get();
        if (symbol == previousSymbol) {
            r.set(1);
        } else {
            r.set(0);
            if (symbol > previousSymbol) {
                equalityFlags->push_back(symbol - 1);
            } else {
                equalityFlags->push_back(symbol);
            }
            previousSymbol = symbol;
        }
        r.inc();
    }

    // Swap back before returning
    equalityFlags->swap(values);
}

// Optimized for wordsize 1 > 0. In place for values
static void transformEqualityCoding1(
        DataStream *const values,
        DataStream *const equalityFlags
){
    uint64_t previousSymbol = 0;

    *equalityFlags = DataStream(0, 1);
    StreamReader r = values->getReader();
    StreamReader w = values->getReader();
    // Treat value as equalityFlags and vice versa
    while(r.isValid()){
        uint64_t symbol = r.get();
        if (symbol == previousSymbol) {
            equalityFlags->push_back(1);
        } else {
            equalityFlags->push_back(0);
            if (symbol > previousSymbol) {
                w.set(symbol - 1);
            } else {
                w.set(symbol);
            }
            w.inc();
            previousSymbol = symbol;
        }
        r.inc();
    }

    values->resize(values->size() - (w.end-w.curr)/w.wordSize);
}

void transformEqualityCoding(
        DataStream *const values,
        DataStream *const equalityFlags
){
    assert(equalityFlags != nullptr);
    assert(values != nullptr);

    if(values->getWordSize() == 1) {
        transformEqualityCoding0(values, equalityFlags);
    } else {
        transformEqualityCoding1(values, equalityFlags);
    }
}

// ----------------------------------------------------------------------------

void inverseTransformEqualityCoding(
        DataStream *const values,
        DataStream *const equalityFlags
){
    assert(values != nullptr);
    assert(equalityFlags != nullptr);
    DataStream output(0, values->getWordSize());
    DataStream *outputptr;

    // Wordsize 1 allows in place operation in equality flag buffer
    if(values->getWordSize() == 1) {
        outputptr = equalityFlags;
    } else {
        // Other wordsizes have to use a distinct buffer
        outputptr = &output;
        output.resize(equalityFlags->size());
    }

    // Re-compute the symbols from the equality flags and values
    uint64_t previousSymbol = 0;

    StreamReader rflag = equalityFlags->getReader();
    StreamReader rval = values->getReader();
    StreamReader rwrite = outputptr->getReader();

    while(rflag.isValid()) {
        if(rflag.get() == 0) {
            uint64_t val = rval.get();
            rval.inc();
            if (val >= previousSymbol) {
                previousSymbol = val + 1;
            } else {
                previousSymbol = val;
            }
        }

        rwrite.set(previousSymbol);
        rwrite.inc();
        rflag.inc();
    }
    outputptr->resize(outputptr->size() - (rwrite.end-rwrite.curr)/rwrite.wordSize);

    // Swap memory to value buffer to meet conventions
    if(values->getWordSize() == 1) {
        values->swap(equalityFlags);
    } else {
        values->swap(&output);
    }

    // Clear equality buffer
    equalityFlags->clear();
    equalityFlags->shrink_to_fit();
}


}  // namespace gabac
