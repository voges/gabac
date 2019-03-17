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
    std::vector<int64_t> symbolsVector(
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
}


// ----------------------------------------------------------------------------
// C wrapper END
// ----------------------------------------------------------------------------


namespace gabac {


int encode(
        const std::vector<int64_t>& symbols,
        const BinarizationId& binarizationId,
        const std::vector<unsigned int>& binarizationParameters,
        const ContextSelectionId& contextSelectionId,
        std::vector<unsigned char> *const bitstream
){
    assert(bitstream != nullptr);
#ifndef NDEBUG
    const unsigned int paramSize[unsigned(BinarizationId::STEG) + 1u] = {1, 1, 0, 0, 1, 1};
#endif
    assert(binarizationParameters.size() >= paramSize[static_cast<int>(binarizationId)]);
    bitstream->clear();

    Writer writer(bitstream);
    writer.start(symbols.size());

    unsigned int binarizationParameter = 0;
    if (binarizationParameters.size() > 0) {
       binarizationParameter = binarizationParameters[0];
    }

    if (contextSelectionId == ContextSelectionId::bypass)
    {
        void (Writer::*func)(int64_t, unsigned int);
        switch (binarizationId)
        {
            case BinarizationId::BI:
                func = &Writer::writeAsBIbypass;
                break;
            case BinarizationId::TU:
                func = &Writer::writeAsTUbypass;
                break;
            case BinarizationId::EG:
                func = &Writer::writeAsEGbypass;
                break;
            case BinarizationId::SEG:
                func = &Writer::writeAsSEGbypass;
                break;
            case BinarizationId::TEG:
                func = &Writer::writeAsTEGbypass;
                break;
            case BinarizationId::STEG:
                func = &Writer::writeAsSTEGbypass;
                break;
            default:
                assert(0);
        }
        for (int64_t symbol : symbols)
        {
            (writer.*func)(
                    symbol,
                    binarizationParameter
            );
        }

        writer.reset();
        return GABAC_SUCCESS;
    }

    void (Writer::*func)(int64_t, unsigned int, unsigned int);
    switch (binarizationId)
    {
        case BinarizationId::BI:
            func = &Writer::writeAsBIcabac;
            break;
        case BinarizationId::TU:
            func = &Writer::writeAsTUcabac;
            break;
        case BinarizationId::EG:
            func = &Writer::writeAsEGcabac;
            break;
        case BinarizationId::SEG:
            func = &Writer::writeAsSEGcabac;
            break;
        case BinarizationId::TEG:
            func = &Writer::writeAsTEGcabac;
            break;
        case BinarizationId::STEG:
            func = &Writer::writeAsSTEGcabac;
            break;
        default:
            assert(0);
    }

    if (contextSelectionId == ContextSelectionId::adaptive_coding_order_0)
    {
        for (int64_t symbol : symbols)
        {
            (writer.*func)(
                    symbol,
                    binarizationParameter,
                    0
            );
        }
    }
    else if (contextSelectionId == ContextSelectionId::adaptive_coding_order_1)
    {
        unsigned int previousSymbol = 0;

        for (int64_t symbol : symbols)
        {
            (writer.*func)(
                    symbol,
                    binarizationParameter,
                    previousSymbol << 2u
            );
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
    }
    else if (contextSelectionId == ContextSelectionId::adaptive_coding_order_2)
    {
        unsigned int previousSymbol = 0;
        unsigned int previousPreviousSymbol = 0;

        for (int64_t symbol : symbols)
        {
            (writer.*func)(
                    symbol,
                    binarizationParameter,
                    (previousSymbol << 2u) + previousPreviousSymbol
            );
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
    }
    else
    {
        return GABAC_FAILURE;
    }

    writer.reset();
    return GABAC_SUCCESS;
}


}  // namespace gabac
