#include "gabac/encoding.h"

#include <algorithm>
#include <cassert>
#include <limits>

#include "gabac/constants.h"
#include "gabac/return_codes.h"
#include "gabac/writer.h"


// ----------------------------------------------------------------------------
// C wrapper BEGIN
// ----------------------------------------------------------------------------

/*
int gabac_encode(
        int64_t *const symbols,
        size_t symbolsSize,
        unsigned int binarizationId,
        unsigned int *const binarizationParameters,
        size_t binarizationParametersSize,
        unsigned int contextSelectionId,
        unsigned char **const bitstream,
        size_t *const bitstreamSize
){
    if (symbols == nullptr)
    {
        return GABAC_FAILURE;
    }
    if (binarizationParameters == nullptr)
    {
        return GABAC_FAILURE;
    }
    if (bitstream == nullptr)
    {
        return GABAC_FAILURE;
    }
    if (bitstreamSize == nullptr)
    {
        return GABAC_FAILURE;
    }

    // C++-style vectors to receive input data / accumulate output data
    DataStream symbolsVector(
            symbols,
            (symbols + symbolsSize)
    );
    std::vector<unsigned int> binarizationParametersVector(
            binarizationParameters,
            (binarizationParameters + binarizationParametersSize)
    );
    std::vector<unsigned char> bitstreamVector;

    assert(binarizationId <= static_cast<int>(gabac::BinarizationId::STEG));
    assert(contextSelectionId <= static_cast<int>(gabac::ContextSelectionId::adaptive_coding_order_2));
    // Execute
    int rc = gabac::encode(
            symbolsVector,
            static_cast<gabac::BinarizationId>(binarizationId),
            binarizationParametersVector,
            static_cast<gabac::ContextSelectionId>(contextSelectionId),
            &bitstreamVector
    );
    if (rc != GABAC_SUCCESS)
    {
        return GABAC_FAILURE;
    }

    // Extract plain C array data from result vectors
    *bitstreamSize = bitstreamVector.size();
    *bitstream = (unsigned char *) malloc(sizeof(char) * (*bitstreamSize));
    std::copy(bitstreamVector.begin(), bitstreamVector.end(), *bitstream);

    return GABAC_SUCCESS;
}*/


// ----------------------------------------------------------------------------
// C wrapper END
// ----------------------------------------------------------------------------


namespace gabac {


int encode(
        const BinarizationId& binarizationId,
        const std::vector<unsigned int>& binarizationParameters,
        const ContextSelectionId& contextSelectionId,
        DataStream *const symbols
){
    DataStream bitstream(0, 1);
    assert(symbols != nullptr);
#ifndef NDEBUG
    const unsigned int paramSize[unsigned(BinarizationId::STEG) + 1u] = {1, 1, 0, 0, 1, 1};
#endif
    assert(binarizationParameters.size() >= paramSize[static_cast<int>(binarizationId)]);

    Writer writer(&bitstream);
    writer.start(symbols->size());

    unsigned int previousSymbol = 0;
    unsigned int previousPreviousSymbol = 0;

    StreamReader r = symbols->getReader();

    if (contextSelectionId == ContextSelectionId::bypass) {
        while (r.isValid()) {
            writer.writeBypassValue(
                    r.get(),
                    binarizationId,
                    binarizationParameters
            );
            r.inc();
        }
    } else if (contextSelectionId == ContextSelectionId::adaptive_coding_order_0) {
        while (r.isValid()) {
            writer.writeCabacAdaptiveValue(
                    r.get(),
                    binarizationId,
                    binarizationParameters,
                    0,
                    0
            );
            r.inc();
        }
    } else if (contextSelectionId == ContextSelectionId::adaptive_coding_order_1) {
        while (r.isValid()) {
            uint64_t symbol = r.get();
            r.inc();
            writer.writeCabacAdaptiveValue(
                    symbol,
                    binarizationId,
                    binarizationParameters,
                    previousSymbol,
                    0
            );
            if (int64_t(symbol) < 0) {
                symbol = uint64_t(-int64_t(symbol));
            }
            if (symbol > 3) {
                previousSymbol = 3;
            } else {
                assert(symbol <= std::numeric_limits<unsigned int>::max());
                previousSymbol = static_cast<unsigned int>(symbol);
            }
        }
    } else if (contextSelectionId == ContextSelectionId::adaptive_coding_order_2) {
        while (r.isValid()) {
            uint64_t symbol = r.get();
            r.inc();
            writer.writeCabacAdaptiveValue(
                    symbol,
                    binarizationId,
                    binarizationParameters,
                    previousSymbol,
                    previousPreviousSymbol
            );
            previousPreviousSymbol = previousSymbol;
            if (int64_t(symbol) < 0) {
                symbol = uint64_t(-int64_t(symbol));
            }
            if (symbol > 3) {
                previousSymbol = 3;
            } else {
                assert(symbol <= std::numeric_limits<unsigned int>::max());
                previousSymbol = static_cast<unsigned int>(symbol);
            }
        }
    } else {
        return GABAC_FAILURE;
    }

    writer.reset();

    symbols->swap(&bitstream);

    return GABAC_SUCCESS;
}


}  // namespace gabac
