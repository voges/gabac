#include "gabac/rle_coding.h"

#include <algorithm>
#include <cassert>


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
        gabac::DataBlock symbolsVector(symbols, (symbols + symbolsSize));
        gabac::DataBlock rawValuesVector;
        gabac::DataBlock lengthsVector;

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
        gabac::DataBlock rawValuesVector(rawValues, (rawValues + rawValuesSize));
        gabac::DataBlock lengthsVector(lengths, (lengths + lengthsSize));
        gabac::DataBlock symbolsVector;

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
        gabac::DataBlock *const rawValues,
        gabac::DataBlock *const lengths
){
    assert(guard > 0);
    assert(rawValues != nullptr);
    assert(lengths != nullptr);

    lengths->clear();

    if (rawValues->empty()) {
        return;
    }

    // input for rawValues is guaranteed to grow slower than reading process
    // -> in place possible


    BlockStepper r = rawValues->getReader();
    BlockStepper w = rawValues->getReader();

    uint64_t cur = r.get();
    r.inc();
    uint64_t lengthValue = 1;
    while (r.isValid()) {
        uint64_t tmp = r.get();
        r.inc();
        if (tmp == cur) {
            ++lengthValue;
        } else {
            w.set(cur);
            w.inc();
            cur = tmp;
            while (lengthValue > guard) {
                lengths->push_back(guard);
                lengthValue -= guard;
            }
            lengths->push_back(lengthValue - 1);
            lengthValue = 1;
        }
    }

    w.set(cur);
    w.inc();
    while (lengthValue > guard) {
        lengths->push_back(guard);
        lengthValue -= guard;
    }
    lengths->push_back(lengthValue - 1);


    rawValues->resize(rawValues->size() - (w.end - w.curr) / w.wordSize);
}


void inverseTransformRleCoding(
        const uint64_t guard,
        gabac::DataBlock *const rawValues,
        gabac::DataBlock *const lengths
){
    assert(rawValues != nullptr);
    assert(!rawValues->empty());
    assert(guard > 0);

    // input for rawValues is not guaranteed to grow slower than reading process
    // -> in place not possible

    gabac::DataBlock symbols(0, rawValues->getWordSize());

    BlockStepper rVal = rawValues->getReader();
    BlockStepper rLen = lengths->getReader();
    // Re-compute the symbol sequence
    while (rVal.isValid()) {
        uint64_t rawValue = rVal.get();
        uint64_t lengthValue = rLen.get();
        rLen.inc();
        uint64_t totalLengthValue = lengthValue;
        while (lengthValue == guard) {
            lengthValue = rLen.get();
            rLen.inc();
            totalLengthValue += lengthValue;
        }
        totalLengthValue++;
        while (totalLengthValue > 0) {
            totalLengthValue--;
            symbols.push_back(rawValue);
        }
        rVal.inc();
    }

    symbols.swap(rawValues);
    lengths->clear();
    lengths->shrink_to_fit();
}


}  // namespace gabac
