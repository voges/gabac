#include "gabacify/encode.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <functional>
#include <iomanip>
#include <limits>
#include <utility>
#include <vector>

#include "gabac/constants.h"
#include "gabac/encoding.h"
#include "gabac/diff_coding.h"

#include "gabacify/analysis.h"
#include "gabacify/configuration.h"
#include "gabacify/exceptions.h"
#include "gabacify/helpers.h"
#include "gabacify/input_file.h"
#include "gabacify/log.h"
#include "gabacify/tmp_file.h"


namespace gabacify {

//------------------------------------------------------------------------------

// Appends the size of a stream and the actual bytes to bytestream
void appendToBytestream(
        const gabac::DataStream& bytes,
        gabac::DataStream *const bytestream
){
    assert(bytestream != nullptr);

    uint32_t size = bytes.size()*bytes.getWordSize();
    uint8_t* ptr = (uint8_t*)&size;
    bytestream->push_back(ptr[0]);
    bytestream->push_back(ptr[1]);
    bytestream->push_back(ptr[2]);
    bytestream->push_back(ptr[3]);

    // Append 'bytes' to the bytestream
    bytestream->insert(bytestream->end(), bytes.begin(), bytes.end());
}

//------------------------------------------------------------------------------

void doSequenceTransform(const gabac::DataStream& sequence,
                         const gabac::SequenceTransformationId& transID,
                         uint64_t param,
                         std::vector<gabac::DataStream> *const transformedSequences
){
    GABACIFY_LOG_TRACE << "Encoding sequence of length: " << sequence.size();

    auto id = unsigned(transID);
    GABACIFY_LOG_DEBUG << "Performing sequence transformation " << gabac::transformationInformation[id].name;

    transformedSequences->resize(gabac::transformationInformation[id].streamNames.size());
    gabac::transformationInformation[id].transform(sequence, param, transformedSequences);

    GABACIFY_LOG_TRACE << "Got " << transformedSequences->size() << " sequences";
    for (unsigned i = 0; i < transformedSequences->size(); ++i)
    {
        GABACIFY_LOG_TRACE << i << ": " << (*transformedSequences)[i].size() << " bytes";
    }
}

//------------------------------------------------------------------------------

void doLutTransform(bool enabled,
                    const gabac::DataStream& transformedSequence,
                    unsigned int order,
                    gabac::DataStream *const bytestream,
                    std::vector<gabac::DataStream> *const lutSequences,
                    unsigned *bits0
){
    if (!enabled)
    {
        GABACIFY_LOG_TRACE << "LUT transform *dis*abled";
        (*lutSequences)[0] = transformedSequence;
        //    appendToBytestream({}, bytestream);
        GABACIFY_LOG_DEBUG << "Got uncompressed stream after LUT: " << (*lutSequences)[0].size() << " bytes";
        GABACIFY_LOG_DEBUG << "Got table after LUT: " << (*lutSequences)[1].size() << " bytes";
        return;
    }

    GABACIFY_LOG_TRACE << "LUT transform *en*abled";
    const unsigned LUT_INDEX = 4;
    lutSequences->resize(gabac::transformationInformation[LUT_INDEX].streamNames.size());
    for(size_t i = 0; i < lutSequences->size(); ++i) {
        (*lutSequences)[i] = gabac::DataStream(0,gabac::fixWordSizes( gabac::transformationInformation[LUT_INDEX].wordsizes, transformedSequence.getWordSize())[i]);
    }
    gabac::transformationInformation[LUT_INDEX].transform(transformedSequence, order, lutSequences);

    if((*lutSequences)[0].empty()) {
        return;
    }

    GABACIFY_LOG_DEBUG << "Got uncompressed stream after LUT: " << (*lutSequences)[0].size() << " bytes";
    GABACIFY_LOG_DEBUG << "Got table0 after LUT: " << (*lutSequences)[1].size() << " bytes";
    GABACIFY_LOG_DEBUG << "Got table1 after LUT: " << (*lutSequences)[2].size() << " bytes";

    // GABACIFY_LOG_DEBUG<<"lut size before coding: "<<inverseLutTmp
    if(*bits0 == 0)
    {
        uint64_t  min=0, max=0;
        deriveMinMaxUnsigned(lutSequences->at(1), sizeof(uint64_t), &min, &max);
        *bits0 = unsigned(std::ceil(std::log2(max + 1)));
        if(max <= 1) {
            *bits0 = 1;
        }
    }
    gabac::DataStream inverseLutBitstream0;
    gabac::DataStream inverseLutBitstream1;
    gabac::encode(
            lutSequences->at(1),
            gabac::BinarizationId::BI,
            {*bits0},
            gabac::ContextSelectionId::bypass,
            &inverseLutBitstream0
    );

    appendToBytestream(inverseLutBitstream0, bytestream);

    unsigned bits1 = 0;

    if(order > 0) {
        bits1 = unsigned((*lutSequences)[1].size());
        bits1 = unsigned(std::ceil(std::log2(bits1)));
        gabac::encode(
                lutSequences->at(2),
                gabac::BinarizationId::BI,
                {bits1},
                gabac::ContextSelectionId::bypass,
                &inverseLutBitstream1
        );

        appendToBytestream(inverseLutBitstream1, bytestream);
    }

    GABACIFY_LOG_TRACE << "Wrote LUT bitstream0 with size: " << inverseLutBitstream0.size();
    GABACIFY_LOG_TRACE << "Wrote LUT bitstream1 with size: " << inverseLutBitstream1.size();
}

//------------------------------------------------------------------------------

void doDiffTransform(bool enabled,
                     gabac::DataStream& lutTransformedSequence,
                     gabac::DataStream *const diffAndLutTransformedSequence
){
    // Diff coding
    if (enabled)
    {
        GABACIFY_LOG_TRACE << "Diff coding *en*abled";
        gabac::transformDiffCoding(lutTransformedSequence, diffAndLutTransformedSequence);
        GABACIFY_LOG_DEBUG << "Got uncompressed stream after diff: "
                           << diffAndLutTransformedSequence->size()
                           << " bytes";
        return;
    }

    diffAndLutTransformedSequence->reserve(lutTransformedSequence.size());

    GABACIFY_LOG_TRACE << "Diff coding *dis*abled";
    lutTransformedSequence.swap(*diffAndLutTransformedSequence);
    GABACIFY_LOG_DEBUG << "Got uncompressed stream after diff: " << diffAndLutTransformedSequence->size() << " bytes";
}

//------------------------------------------------------------------------------

static void encodeStream(const TransformedSequenceConfiguration& conf,
                         const gabac::DataStream& diffAndLutTransformedSequence,
                         gabac::DataStream *const bytestream
){
    // Encoding
    gabac::DataStream bitstream;
    gabac::encode(
            diffAndLutTransformedSequence,
            conf.binarizationId,
            conf.binarizationParameters,
            conf.contextSelectionId,
            &bitstream
    );
    GABACIFY_LOG_TRACE << "Bitstream size: " << bitstream.size();
    appendToBytestream(bitstream, bytestream);
}

//------------------------------------------------------------------------------

static void encodeSingleSequence(const TransformedSequenceConfiguration& configuration,
                                 gabac::DataStream *const seq,
                                 gabac::DataStream *const bytestream
){
    std::vector<gabac::DataStream> lutTransformedSequences;
    lutTransformedSequences.resize(3);
    unsigned bits = configuration.lutBits;
    doLutTransform(
            configuration.lutTransformationEnabled,
            *seq,
            configuration.lutOrder,
            bytestream,
            &lutTransformedSequences,
            &bits
    );
    seq->clear();
    seq->shrink_to_fit();

    gabac::DataStream diffAndLutTransformedSequence(0, lutTransformedSequences[0].getWordSize());
    doDiffTransform(
            configuration.diffCodingEnabled,
            lutTransformedSequences[0],
            &diffAndLutTransformedSequence
    );
    lutTransformedSequences[0].clear();
    lutTransformedSequences[0].shrink_to_fit();

    encodeStream(configuration, diffAndLutTransformedSequence, bytestream);
    diffAndLutTransformedSequence.clear();
    diffAndLutTransformedSequence.shrink_to_fit();
}

//------------------------------------------------------------------------------

static void encodeWithConfiguration(
        const Configuration& configuration,
        gabac::DataStream *const sequence,
        gabac::DataStream *const bytestream
){


    std::vector<gabac::DataStream> transformedSequences;
    doSequenceTransform(
            *sequence,
            configuration.sequenceTransformationId,
            configuration.sequenceTransformationParameter,
            &transformedSequences
    );
    sequence->clear();
    sequence->shrink_to_fit();
    std::vector<unsigned> wordsizes = gabac::fixWordSizes(
            gabac::transformationInformation[unsigned(configuration.sequenceTransformationId)].wordsizes,
            configuration.wordSize
    );

    // Loop through the transformed sequences
    for (size_t i = 0; i < transformedSequences.size(); i++)
    {
        encodeSingleSequence(
                configuration.transformedSequenceConfigurations.at(i),
                &(transformedSequences[i]),
                bytestream
        );
        transformedSequences[i].clear();
        transformedSequences[i].shrink_to_fit();
    }
}

//------------------------------------------------------------------------------

void encode_plain(const std::string& inputFilePath,
                  const std::string& configurationFilePath,
                  const std::string& outputFilePath
){
    InputFile configurationFile(configurationFilePath);
    std::string jsonInput("\0", configurationFile.size());
    configurationFile.read(&jsonInput[0], 1, jsonInput.size());
    Configuration configuration(jsonInput);

    // Read in the entire input file
    InputFile inputFile(inputFilePath);
    gabac::DataStream symbols(inputFile.size() / configuration.wordSize, configuration.wordSize);
    inputFile.read(symbols.getData(), 1, symbols.size()*configuration.wordSize);
    // Read the entire configuration file as a string and convert the JSON
    // input string to the internal GABAC configuration

    // Generate symbol stream from byte buffer
    gabac::DataStream buffer(0, 1);
    encodeWithConfiguration(configuration, &symbols, &buffer);
    symbols.clear();
    symbols.shrink_to_fit();

    // Write the bytestream
    OutputFile outputFile(outputFilePath);
    outputFile.write(buffer.getData(), 1, buffer.size() * buffer.getWordSize());
    GABACIFY_LOG_INFO << "Wrote bytestream of size " << buffer.size() * buffer.getWordSize() << " to: " << outputFilePath;
    buffer.clear();
    buffer.shrink_to_fit();
}

//------------------------------------------------------------------------------

void encode(
        const std::string& inputFilePath,
        bool analyze,
        const std::string& configurationFilePath,
        const std::string& outputFilePath
){
    assert(!inputFilePath.empty());
    assert(!configurationFilePath.empty());
    assert(!outputFilePath.empty());

    if (analyze)
    {
        encode_analyze(inputFilePath, configurationFilePath, outputFilePath);
        return;
    }
    encode_plain(inputFilePath, configurationFilePath, outputFilePath);
}

//------------------------------------------------------------------------------

}  // namespace gabacify

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
