#include "gabac/decoding.h"

#include <algorithm>
#include <cassert>
#include <limits>

#include "gabac/constants.h"
#include "gabac/reader.h"
#include "gabac/return_codes.h"


// ----------------------------------------------------------------------------
// C wrapper BEGIN
// ----------------------------------------------------------------------------

/*
int gabac_decode(
        unsigned char *const bitstream,
        size_t bitstreamSize,
        unsigned int binarizationId,
        unsigned int *const binarizationParameters,
        size_t binarizationParametersSize,
        unsigned int contextSelectionId,
        int64_t **const symbols,
        size_t *const symbolsSize
){
    if (bitstream == nullptr)
    {
        return GABAC_FAILURE;
    }
    if (binarizationParameters == nullptr)
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
    std::vector<unsigned char> bitstreamVector(
            bitstream,
            (bitstream + bitstreamSize)
    );
    std::vector<unsigned int> binarizationParametersVector(
            binarizationParameters,
            (binarizationParameters + binarizationParametersSize)
    );
    std::vector<int64_t> symbolsVector;

    assert(binarizationId <= static_cast<int>(gabac::BinarizationId::STEG));
    assert(contextSelectionId <= static_cast<int>(gabac::ContextSelectionId::adaptive_coding_order_2));

    // Execute
    int rc = gabac::decode(
            bitstreamVector,
            static_cast<gabac::BinarizationId>(binarizationId),
            binarizationParametersVector,
            static_cast<gabac::ContextSelectionId>(contextSelectionId),
            &symbolsVector
    );
    if (rc != GABAC_SUCCESS)
    {
        return GABAC_FAILURE;
    }

    // Extract plain C array data from result vectors
    *symbolsSize = symbolsVector.size();
    *symbols = static_cast<int64_t*>(malloc(sizeof(int64_t) * (*symbolsSize)));
    std::copy(symbolsVector.begin(), symbolsVector.end(), *symbols);

    return GABAC_SUCCESS;
}

*/
// ----------------------------------------------------------------------------
// C wrapper END
// ----------------------------------------------------------------------------


namespace gabac {


int decode(
        const uint8_t wordsize,
        const BinarizationId& binarizationId,
        const std::vector<unsigned int>& binarizationParameters,
        const ContextSelectionId& contextSelectionId,
        DataStream *const bitstream
){
    DataStream symbols(0, wordsize);
    if (bitstream == nullptr)
    {
        return GABAC_FAILURE;
    }

    Reader reader(bitstream);
    size_t symbolsSize = reader.start();

    // symbols->clear();
    symbols.resize(symbolsSize);

    uint64_t symbol = 0;
    unsigned int previousSymbol = 0;
    unsigned int previousPreviousSymbol = 0;

    StreamReader r = symbols.getReader();
    while(r.isValid())
    {
        if (contextSelectionId == ContextSelectionId::bypass)
        {
            symbol = reader.readBypassValue(
                    binarizationId,
                    binarizationParameters
            );
            r.set(symbol);
        }
        else if (contextSelectionId
                 == ContextSelectionId::adaptive_coding_order_0)
        {
            symbol = reader.readAdaptiveCabacValue(
                    binarizationId,
                    binarizationParameters,
                    0,
                    0
            );
            r.set(symbol);
        }
        else if (contextSelectionId
                 == ContextSelectionId::adaptive_coding_order_1)
        {
            symbol = reader.readAdaptiveCabacValue(
                    binarizationId,
                    binarizationParameters,
                    previousSymbol,
                    0
            );
            r.set(symbol);
            if (int64_t (symbol) < 0)
            {
                symbol = uint64_t (-int64_t (symbol));
            }
            if (symbol > 3)
            {
                previousSymbol = 3;
            }
            else
            {
                assert(symbol <= std::numeric_limits<unsigned int>::max());
                previousSymbol = static_cast<unsigned int>(symbol);
            }
        }
        else if (contextSelectionId
                 == ContextSelectionId::adaptive_coding_order_2)
        {
            symbol = reader.readAdaptiveCabacValue(
                    binarizationId,
                    binarizationParameters,
                    previousSymbol,
                    previousPreviousSymbol
            );
            r.set(symbol);
            previousPreviousSymbol = previousSymbol;
            if (int64_t (symbol) < 0)
            {
                symbol = uint64_t (-int64_t (symbol));
            }
            if (symbol > 3)
            {
                previousSymbol = 3;
            }
            else
            {
                assert(symbol <= std::numeric_limits<unsigned int>::max());
                previousSymbol = static_cast<unsigned int>(symbol);
            }
        }
        else
        {
            return GABAC_FAILURE;
        }
        r.inc();
    }

    reader.reset();

    symbols.swap(bitstream);

    return GABAC_SUCCESS;
}


}  // namespace gabac
