#include "gabac/decoding.h"

#include <algorithm>
#include <cassert>
#include <limits>
#include <cmath>

#include "gabac/constants.h"
#include "gabac/configuration.h"
#include "gabac/reader.h"
#include "gabac/diff_coding.h"
#include "input_stream.h"
#include "output_stream.h"


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


int decode_cabac(
        const uint8_t wordsize,
        const BinarizationId& binarizationId,
        const std::vector<unsigned int>& binarizationParameters,
        const ContextSelectionId& contextSelectionId,
        DataBlock *const bitstream
){
    DataBlock symbols(0, wordsize);
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

    BlockStepper r = symbols.getReader();
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


//------------------------------------------------------------------------------

static void decodeInverseLUT(unsigned bits0,
                             unsigned order,
                             gabac::InputStream* inStream,
                             gabac::DataBlock *const inverseLut,
                             gabac::DataBlock *const inverseLut1
){
    inStream->readStream(inverseLut);

    size_t lutWordSize = 0;
    if (bits0 <= 8) {
        lutWordSize = 1;
    } else if (bits0 <= 16) {
        lutWordSize = 2;
    } else if (bits0 <= 32) {
        lutWordSize = 4;
    } else if (bits0 <= 64) {
        lutWordSize = 8;
    }

    gabac::decode_cabac(
            lutWordSize,
            gabac::BinarizationId::BI,
            {bits0},
            gabac::ContextSelectionId::bypass,
            inverseLut
    );

    if (order > 0) {
        inStream->readStream(inverseLut1);
        unsigned bits1 = unsigned(inverseLut->size());

        bits1 = unsigned(std::ceil(std::log2(bits1)));

        size_t lut1WordSize = 0;
        if (bits1 <= 8) {
            lut1WordSize = 1;
        } else if (bits1 <= 16) {
            lut1WordSize = 2;
        } else if (bits1 <= 32) {
            lut1WordSize = 4;
        } else if (bits1 <= 64) {
            lut1WordSize = 8;
        }

        gabac::decode_cabac(
                lut1WordSize,
                gabac::BinarizationId::BI,
                {bits1},
                gabac::ContextSelectionId::bypass,
                inverseLut1
        );
    }
}

//------------------------------------------------------------------------------

static void doDiffCoding(bool enabled,
                         gabac::DataBlock *const lutTransformedSequence
){
    // Diff coding
    if (enabled) {
        //GABACIFY_LOG_TRACE << "Diff coding *en*abled";
        gabac::inverseTransformDiffCoding(lutTransformedSequence);
        return;
    }

    //GABACIFY_LOG_TRACE << "Diff coding *dis*abled";
}

//------------------------------------------------------------------------------

static void doLUTCoding(bool enabled,
                        unsigned order,
                        std::vector<gabac::DataBlock> *const lutSequences
){
    if (enabled) {
        //GABACIFY_LOG_TRACE << "LUT transform *en*abled";

        // Do the inverse LUT transform
        const unsigned LUT_INDEX = 4;
        gabac::transformationInformation[LUT_INDEX].inverseTransform(order, lutSequences);
        return;
    }

    //GABACIFY_LOG_TRACE << "LUT transform *dis*abled";
}

//------------------------------------------------------------------------------

static void doEntropyCoding(const gabac::TransformedSequenceConfiguration& transformedSequenceConfiguration,
                            uint8_t wordsize,
                            gabac::InputStream* inStream,
                            gabac::DataBlock *const diffAndLutTransformedSequence
){
    inStream->readStream(diffAndLutTransformedSequence);
    //GABACIFY_LOG_TRACE << "Bitstream size: " << diffAndLutTransformedSequence->size();

    // Decoding
    gabac::decode_cabac(
            wordsize,
            transformedSequenceConfiguration.binarizationId,
            transformedSequenceConfiguration.binarizationParameters,
            transformedSequenceConfiguration.contextSelectionId,
            diffAndLutTransformedSequence
    );

}

//------------------------------------------------------------------------------

void decode(
        const gabac::Configuration& configuration,
        gabac::InputStream *const inStream,
        gabac::OutputStream *const outStream
){

    while(inStream->isValid()) {

        // Set up for the inverse sequence transformation
        size_t numTransformedSequences =
                gabac::transformationInformation[unsigned(configuration.sequenceTransformationId)].wordsizes.size();

        // Loop through the transformed sequences
        std::vector<gabac::DataBlock> transformedSequences;
        for (size_t i = 0; i < numTransformedSequences; i++) {
            //GABACIFY_LOG_TRACE << "Processing transformed sequence: " << i;
            auto transformedSequenceConfiguration = configuration.transformedSequenceConfigurations.at(i);

            std::vector<gabac::DataBlock> lutTransformedSequences(3);
            if (transformedSequenceConfiguration.lutTransformationEnabled) {
                decodeInverseLUT(
                        configuration.transformedSequenceConfigurations[i].lutBits,
                        configuration.transformedSequenceConfigurations[i].lutOrder,
                        inStream,
                        &lutTransformedSequences[1],
                        &lutTransformedSequences[2]
                );
            }

            uint8_t wsize =
                    gabac::transformationInformation[unsigned(configuration.sequenceTransformationId)].wordsizes[i];
            if (wsize == 0) {
                wsize = configuration.wordSize;
            }

            doEntropyCoding(
                    configuration.transformedSequenceConfigurations[i],
                    wsize,
                    inStream,
                    &lutTransformedSequences[0]
            );

            doDiffCoding(
                    configuration.transformedSequenceConfigurations[i].diffCodingEnabled,
                    &(lutTransformedSequences[0])
            );

            doLUTCoding(
                    configuration.transformedSequenceConfigurations[i].lutTransformationEnabled,
                    configuration.transformedSequenceConfigurations[i].lutOrder,
                    &lutTransformedSequences
            );

            transformedSequences.emplace_back();
            transformedSequences.back().swap(&(lutTransformedSequences[0]));
        }


        gabac::transformationInformation[unsigned(configuration.sequenceTransformationId)].inverseTransform(
                configuration.sequenceTransformationParameter,
                &transformedSequences
        );
        //GABACIFY_LOG_TRACE << "Decoded sequence of length: " << transformedSequences[0].size();

        outStream->writeBytes(&transformedSequences[0]);
    }
}


}  // namespace gabac
