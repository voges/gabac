#include "gabac/encoding.h"

#include <algorithm>
#include <cassert>
#include <limits>
#include <cmath>

#include "gabac/constants.h"
#include "gabac/writer.h"
#include "gabac/stream_handler.h"
#include "configuration.h"
#include "gabac.h"


namespace gabac {


ReturnCode encode_cabac(
        const BinarizationId& binarizationId,
        const std::vector<unsigned int>& binarizationParameters,
        const ContextSelectionId& contextSelectionId,
        DataBlock *const symbols,
        size_t maxSize
){
    DataBlock bitstream(0, 1);
    assert(symbols != nullptr);
#ifndef NDEBUG
    const unsigned int paramSize[unsigned(BinarizationId::STEG) + 1u] = {1, 1, 0, 0, 1, 1};
#endif
    assert(binarizationParameters.size() >= paramSize[static_cast<int>(binarizationId)]);

    Writer writer(&bitstream);
    writer.start(symbols->size());

    unsigned int previousSymbol = 0;
    unsigned int previousPreviousSymbol = 0;

    BlockStepper r = symbols->getReader();

    if (contextSelectionId == ContextSelectionId::bypass) {
        while (r.isValid()) {
            if (maxSize <= bitstream.size()) {
                break;
            }
            writer.writeBypassValue(
                    r.get(),
                    binarizationId,
                    binarizationParameters
            );
            r.inc();
        }
    } else if (contextSelectionId == ContextSelectionId::adaptive_coding_order_0) {
        while (r.isValid()) {
            if (maxSize <= bitstream.size()) {
                break;
            }
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
            if (maxSize <= bitstream.size()) {
                break;
            }
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
            if (maxSize <= bitstream.size()) {
                break;
            }
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
        return ReturnCode::failure;
    }

    writer.reset();

    symbols->swap(&bitstream);

    return ReturnCode::success;
}


static uint64_t getMax(const gabac::DataBlock& b){
    uint64_t max = 0;
    gabac::BlockStepper r = b.getReader();
    while (r.isValid()) {
        max = std::max(max, r.get());
    }
    return max;
}

//------------------------------------------------------------------------------

void doSequenceTransform(const gabac::SequenceTransformationId& transID,
                         uint64_t param,
                         std::vector<gabac::DataBlock> *const transformedSequences
){
    //GABACIFY_LOG_TRACE << "Encoding sequence of length: " << (*transformedSequences)[0].size();

    auto id = unsigned(transID);
    //GABACIFY_LOG_DEBUG << "Performing sequence transformation " << gabac::transformationInformation[id].name;

    gabac::transformationInformation[id].transform(param, transformedSequences);

    //GABACIFY_LOG_TRACE << "Got " << transformedSequences->size() << " sequences";
    for (unsigned i = 0; i < transformedSequences->size(); ++i) {
        //GABACIFY_LOG_TRACE << i << ": " << (*transformedSequences)[i].size() << " bytes";
    }
}

//------------------------------------------------------------------------------

static void encodeStream(const gabac::TransformedSequenceConfiguration& conf,
                         gabac::DataBlock *const diffAndLutTransformedSequence,
                         std::ostream *out
){
    // Encoding
    gabac::encode_cabac(
            conf.binarizationId,
            conf.binarizationParameters,
            conf.contextSelectionId,
            diffAndLutTransformedSequence
    );

    gabac::StreamHandler::writeStream(*out, diffAndLutTransformedSequence);
}

//------------------------------------------------------------------------------

void doLutTransform(unsigned int order,
                    std::vector<gabac::DataBlock> *const lutSequences,
                    unsigned *bits0,
                    std::ostream *out
){
    //GABACIFY_LOG_TRACE << "LUT transform *en*abled";
    const unsigned LUT_INDEX = 4;

    // Put raw sequence in, get transformed sequence and lut tables
    gabac::transformationInformation[LUT_INDEX].transform(order, lutSequences);


    //GABACIFY_LOG_DEBUG << "Got uncompressed stream after LUT: " << (*lutSequences)[0].size() << " bytes";
    //GABACIFY_LOG_DEBUG << "Got table0 after LUT: " << (*lutSequences)[1].size() << " bytes";
    //GABACIFY_LOG_DEBUG << "Got table1 after LUT: " << (*lutSequences)[2].size() << " bytes";

    // Calculate bit size for order 0 table
    if (*bits0 == 0) {
        uint64_t max = getMax(lutSequences->at(1));
        *bits0 = unsigned(std::ceil(std::log2(max + 1)));
        if (max <= 1) {
            *bits0 = 1;
        }
    }

    auto bits1 = unsigned((*lutSequences)[1].size());
    encodeStream(
            {false, 0, 0, false, gabac::BinarizationId::BI, {*bits0}, gabac::ContextSelectionId::bypass},
            &(*lutSequences)[1],
            out
    );

    if (order > 0) {
        bits1 = unsigned(std::ceil(std::log2(bits1)));
        encodeStream(
                {false, 0, 0, false, gabac::BinarizationId::BI, {bits1}, gabac::ContextSelectionId::bypass},
                &(*lutSequences)[2],
                out
        );
    }

}

//------------------------------------------------------------------------------

void doDiffTransform(std::vector<gabac::DataBlock> *const sequence
){
    //GABACIFY_LOG_TRACE << "LUT transform *en*abled";
    const unsigned DIFF_INDEX = 5;

    // Put raw sequence in, get transformed sequence and lut tables
    gabac::transformationInformation[DIFF_INDEX].transform(0, sequence);

    //GABACIFY_LOG_TRACE << "Diff coding *dis*abled";
    //GABACIFY_LOG_DEBUG << "Got uncompressed stream after diff: " << diffAndLutTransformedSequence->size() << " bytes";
}

//------------------------------------------------------------------------------

static void encodeSingleSequence(const gabac::TransformedSequenceConfiguration& configuration,
                                 gabac::DataBlock *const seq,
                                 std::ostream *out
){
    std::vector<gabac::DataBlock> lutTransformedSequences;

    // Symbol/transformed symbols, lut0 bytestream, lut1 bytestream
    lutTransformedSequences.resize(1);
    lutTransformedSequences[0].swap(seq);

    // Put sequence in, get lut sequence and lut bytestreams
    if (configuration.lutTransformationEnabled) {
        unsigned bits = configuration.lutBits;
        doLutTransform(
                configuration.lutOrder,
                &lutTransformedSequences,
                &bits,
                out
        );
    }

    // Put lut transformed in, get difftransformed out
    if (configuration.diffCodingEnabled) {
        doDiffTransform(
                &lutTransformedSequences
        );
    }

    encodeStream(configuration, &lutTransformedSequences[0], out);
}

//------------------------------------------------------------------------------


void encode(
        const IOConfiguration& conf,
        const EncodingConfiguration& enConf
){
    conf.validate();
    gabac::DataBlock sequence(0, 1);
    sequence.setWordSize(static_cast<uint8_t>(enConf.wordSize));
    size_t size = 0;
    if (!conf.blocksize) {
        size = gabac::StreamHandler::readFull(*conf.inputStream, &sequence);
    } else {
        size = gabac::StreamHandler::readBlock(*conf.inputStream, conf.blocksize, &sequence);
    }
    while (size) {
        // Insert sequence into vector
        std::vector<gabac::DataBlock> transformedSequences;
        transformedSequences.resize(1);
        transformedSequences[0].swap(&sequence);

        // Put symbol stream in, get transformed streams out
        doSequenceTransform(
                enConf.sequenceTransformationId,
                enConf.sequenceTransformationParameter,
                &transformedSequences
        );

        // Loop through the transformed sequences
        for (size_t i = 0; i < transformedSequences.size(); i++) {
            // Put transformed sequence in, get partial bytestream back
            encodeSingleSequence(
                    enConf.transformedSequenceConfigurations[i],
                    &(transformedSequences[i]),
                    conf.outputStream
            );
        }
        if (conf.blocksize) {
            size = gabac::StreamHandler::readBlock(*conf.inputStream, conf.blocksize, &sequence);
        } else {
            size = 0;
        }
    }

}

}  // namespace gabac
