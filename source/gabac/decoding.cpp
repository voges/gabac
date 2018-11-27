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


// ----------------------------------------------------------------------------
// C wrapper END
// ----------------------------------------------------------------------------


namespace gabac {


int decode(
        const std::vector<unsigned char>& bitstream,
        const BinarizationId& binarizationId,
        const std::vector<unsigned int>& binarizationParameters,
        const ContextSelectionId& contextSelectionId,
        std::vector<int64_t> *const symbols
){
    if (symbols == nullptr)
    {
        return GABAC_FAILURE;
    }

    Reader reader(bitstream);
    size_t symbolsSize = reader.start();

    // symbols->clear();
    symbols->resize(symbolsSize);

    int64_t symbol = 0;
    unsigned int previousSymbol = 0;
    unsigned int previousPreviousSymbol = 0;

    for (size_t i = 0; i < symbolsSize; i++)
    {
        if (contextSelectionId == ContextSelectionId::bypass)
        {
            symbol = reader.readBypassValue(
                    binarizationId,
                    binarizationParameters
            );
            (*symbols)[i] = symbol;
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
            (*symbols)[i] = symbol;
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
            (*symbols)[i] = symbol;
            if (symbol < 0)
            {
                symbol = -symbol;
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
            (*symbols)[i] = symbol;
            previousPreviousSymbol = previousSymbol;
            if (symbol < 0)
            {
                symbol = -symbol;
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
    }

    reader.reset();

    return GABAC_SUCCESS;
}


}  // namespace gabac
