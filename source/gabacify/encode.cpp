#include "gabacify/encode.h"

#include <cassert>
#include <iomanip>
#include <limits>
#include <utility>
#include <vector>

#include "gabac/diff_coding.h"
#include "gabac/encoding.h"
#include "gabac/equality_coding.h"
#include "gabac/lut_transform.h"
#include "gabac/match_coding.h"
#include "gabac/rle_coding.h"

#include "gabacify/analysis.h"
#include "gabacify/configuration.h"
#include "gabacify/exceptions.h"
#include "gabacify/helpers.h"
#include "gabacify/input_file.h"
#include "gabacify/log.h"
#include "gabacify/output_file.h"


namespace gabacify {

static std::vector<unsigned char> tempSubseq0;
static gabacify::Configuration configurationTempSubseq0;
static std::vector<unsigned char> tempSubseq0Lut0;
static std::vector<unsigned char> tempSubseq1;
static gabacify::Configuration configurationTempSubseq1;
static std::vector<unsigned char> tempSubseq0Lut1;


static void appendToBytestream(
        const std::vector<unsigned char>& bytes,
        std::vector<unsigned char> * const bytestream
){
    assert(bytestream != nullptr);

    // Append the size of 'bytes' to the bytestream
    std::vector<unsigned char> sizeBuffer;
    generateByteBuffer({ static_cast<uint64_t>(bytes.size()) }, 4, &sizeBuffer);
    for (const auto& sizeByte : sizeBuffer)
    {
        bytestream->push_back(sizeByte);
    }

    // Append 'bytes' to the bytestream
    bytestream->insert(bytestream->end(), bytes.begin(), bytes.end());
}


static void encodeWithConfiguration(
        const std::string& inputFilePath,
        const std::vector<uint64_t>& sequence,
        const Configuration& configuration,
        std::vector<unsigned char> * const bytestream
){
    assert(bytestream != nullptr);

    unsigned int wordSize = configuration.wordSize;

    GABACIFY_LOG_TRACE << "Encoding sequence of length: " << sequence.size();
    std::string subseq0Path;
    std::string subseq1Path;
    std::string subseq2Path;

    // Do the sequence transformation
    std::vector<std::vector<uint64_t>> transformedSequences;
    if (configuration.sequenceTransformationId == gabac::SequenceTransformationId::no_transform)
    {
        GABACIFY_LOG_DEBUG << "Performing sequence transformation 'no_transform'";
        transformedSequences.push_back(std::move(sequence));
        GABACIFY_LOG_DEBUG << "Generated transformed sequence with size: " << transformedSequences.front().size();
    }
    else if (configuration.sequenceTransformationId == gabac::SequenceTransformationId::equality_coding)
    {
        GABACIFY_LOG_DEBUG << "Performing sequence transformation 'equality_coding'";
        subseq0Path = inputFilePath+".eq.equalityFlags.tmp";
        subseq1Path = inputFilePath+".eq.values.tmp";
        std::vector<uint64_t> equalityFlags;
        std::vector<uint64_t> values;
        //test if preprocessed files exist
        if(fileExists(subseq0Path) && fileExists(subseq1Path))
        {
            InputFile inputFileSubseq0(subseq0Path);
            InputFile inputFileSubseq1(subseq1Path);
            size_t bufferSize = inputFileSubseq0.size();
            std::vector<unsigned char> buffer(bufferSize);
            inputFileSubseq0.read(&buffer[0], 1, bufferSize);
            generateSymbolStream(buffer, 1, &equalityFlags);
            bufferSize = inputFileSubseq1.size();
            buffer.resize(bufferSize);
            inputFileSubseq1.read(&buffer[0], 1, bufferSize);
            generateSymbolStream(buffer, wordSize, &values);
            GABACIFY_LOG_DEBUG << "Transformed sequence 'equality flags' read with size: " << equalityFlags.size();
            GABACIFY_LOG_DEBUG << "Transformed sequence 'values' read with size: " << values.size();
        }
        else
        {
            gabac::transformEqualityCoding(sequence, &equalityFlags, &values);
            GABACIFY_LOG_DEBUG << "Generated transformed sequence 'equality flags' with size: " << equalityFlags.size();
            GABACIFY_LOG_DEBUG << "Generated transformed sequence 'values' with size: " << values.size();
            OutputFile outputFileSubseq0(subseq0Path);
            OutputFile outputFileSubseq1(subseq1Path);
            // Write equalityFlags
            std::vector<unsigned char> equalityFlagsBuffer;
            generateByteBuffer(equalityFlags, 1, &equalityFlagsBuffer);
            outputFileSubseq0.write(&equalityFlagsBuffer[0], 1, equalityFlagsBuffer.size());
            // Write values
            std::vector<unsigned char> valuesBuffer;
            generateByteBuffer(values, wordSize, &valuesBuffer);
            outputFileSubseq1.write(&valuesBuffer[0], 1, valuesBuffer.size());
        }
        transformedSequences.push_back(std::move(equalityFlags));
        transformedSequences.push_back(std::move(values));
    }
    else if (configuration.sequenceTransformationId == gabac::SequenceTransformationId::match_coding)
    {
        GABACIFY_LOG_DEBUG << "Performing sequence transformation 'match_coding'";
        auto windowSize = static_cast<uint32_t>(configuration.sequenceTransformationParameter);
        subseq0Path = inputFilePath + ".match." + std::to_string(windowSize) + ".pointers.tmp";
        subseq1Path = inputFilePath + ".match." + std::to_string(windowSize) + ".lengths.tmp";
        subseq2Path = inputFilePath + ".match." + std::to_string(windowSize) + ".values.tmp";
        if (configuration.sequenceTransformationParameter > std::numeric_limits<uint32_t>::max())
        {
            GABACIFY_DIE("Match coding window size must be smaller than 2^32");
        }
        GABACIFY_LOG_DEBUG << "Match coding window size: " << windowSize;
        std::vector<uint64_t> pointers;
        std::vector<uint64_t> lengths;
        std::vector<uint64_t> rawValues;
        //test if preprocessed files exist
        if(fileExists(subseq0Path) && fileExists(subseq1Path) && fileExists(subseq2Path))
        {
            InputFile inputFileSubseq0(subseq0Path);
            InputFile inputFileSubseq1(subseq1Path);
            InputFile inputFileSubseq2(subseq2Path);
            size_t bufferSize = inputFileSubseq0.size();
            std::vector<unsigned char> buffer(bufferSize);
            inputFileSubseq0.read(&buffer[0], 1, bufferSize);
            generateSymbolStream(buffer, 4, &pointers);
            bufferSize = inputFileSubseq1.size();
            buffer.resize(bufferSize);
            inputFileSubseq1.read(&buffer[0], 1, bufferSize);
            generateSymbolStream(buffer, 4, &lengths);
            bufferSize = inputFileSubseq2.size();
            buffer.resize(bufferSize);
            inputFileSubseq2.read(&buffer[0], 1, bufferSize);
            generateSymbolStream(buffer, wordSize, &rawValues);
            GABACIFY_LOG_DEBUG << "Transformed sequence 'pointers' read with size: " << pointers.size();
            GABACIFY_LOG_DEBUG << "Transformed sequence 'lengths' read with size: " << lengths.size();
            GABACIFY_LOG_DEBUG << "Transformed sequence 'values' read with size: " << rawValues.size();
        }
        else
        {
          gabac::transformMatchCoding(sequence, windowSize, &pointers, &lengths, &rawValues);
          GABACIFY_LOG_DEBUG << "Generated transformed sequence 'pointers' with size: " << pointers.size();
          GABACIFY_LOG_DEBUG << "Generated transformed sequence 'lengths' with size: " << lengths.size();
          GABACIFY_LOG_DEBUG << "Generated transformed sequence 'raw values' with size: " << rawValues.size();
          OutputFile outputFileSubseq0(subseq0Path);
          OutputFile outputFileSubseq1(subseq1Path);
          OutputFile outputFileSubseq2(subseq2Path);
          //schrijf equalityFlags
          std::vector<unsigned char> equalityFlagsBuffer;
          generateByteBuffer(pointers, 4, &equalityFlagsBuffer);
          outputFileSubseq0.write(&equalityFlagsBuffer[0], 1, equalityFlagsBuffer.size());
          //schrijf lengths
          std::vector<unsigned char> lengthsBuffer;
          generateByteBuffer(lengths, 4, &lengthsBuffer);
          outputFileSubseq1.write(&lengthsBuffer[0], 1, lengthsBuffer.size());
          //schrijf rawValues
          std::vector<unsigned char> valuesBuffer;
          generateByteBuffer(rawValues, wordSize, &valuesBuffer);
          outputFileSubseq2.write(&valuesBuffer[0], 1, valuesBuffer.size());
        }

        transformedSequences.push_back(std::move(pointers));
        transformedSequences.push_back(std::move(lengths));
        transformedSequences.push_back(std::move(rawValues));
    }
    else if (configuration.sequenceTransformationId == gabac::SequenceTransformationId::rle_coding)
    {
        GABACIFY_LOG_DEBUG << "Performing sequence transformation 'rle_coding'";
        auto guard = static_cast<uint64_t>(configuration.sequenceTransformationParameter);
        subseq0Path = inputFilePath+".rle."+std::to_string(guard)+".values.tmp";
        subseq1Path = inputFilePath+".rle."+std::to_string(guard)+".lengths.tmp";
        GABACIFY_LOG_DEBUG << "RLE coding guard: " << guard;
        std::vector<uint64_t> rawValues;
        std::vector<uint64_t> lengths;
        if(fileExists(subseq0Path) && fileExists(subseq1Path))
        {
            InputFile inputFileSubseq0(subseq0Path);
            InputFile inputFileSubseq1(subseq1Path);
            size_t bufferSize = inputFileSubseq0.size();
            std::vector<unsigned char> buffer(bufferSize);
            inputFileSubseq0.read(&buffer[0], 1, bufferSize);
            generateSymbolStream(buffer, wordSize, &rawValues);
            bufferSize = inputFileSubseq1.size();
            buffer.resize(bufferSize);
            inputFileSubseq1.read(&buffer[0], 1, bufferSize);
            generateSymbolStream(buffer, 4, &lengths);
            GABACIFY_LOG_DEBUG << "Transformed sequence 'raw values' read with size: " << rawValues.size();
            GABACIFY_LOG_DEBUG << "Transformed sequence 'lengths' read with size: " << lengths.size();
        }
        else
        {
          gabac::transformRleCoding(sequence, guard, &rawValues, &lengths);
          GABACIFY_LOG_DEBUG << "Generated transformed sequence 'raw values' with size: " << rawValues.size();
          GABACIFY_LOG_DEBUG << "Generated transformed sequence 'lengths' with size: " << lengths.size();
          OutputFile outputFileSubseq0(subseq0Path);
          OutputFile outputFileSubseq1(subseq1Path);
          //schrijf raw values
          std::vector<unsigned char> rawValueBuffer;
          generateByteBuffer(rawValues, wordSize, &rawValueBuffer);
          outputFileSubseq0.write(&rawValueBuffer[0], 1, rawValueBuffer.size());
          //schrijf lengths
          std::vector<unsigned char> lengthsBuffer;
          generateByteBuffer(lengths, 4, &lengthsBuffer);
          outputFileSubseq1.write(&lengthsBuffer[0], 1, lengthsBuffer.size());
        }
        transformedSequences.push_back(std::move(rawValues));
        transformedSequences.push_back(std::move(lengths));
    }
    else
    {
        GABACIFY_DIE("Invalid sequence transformation ID");
    }

    // Loop through the transformed sequences
    for (size_t i = 0; i < transformedSequences.size(); i++)
    {
        if (i == 0)
        {
            if (configuration.equal(configurationTempSubseq0, i))
            {
                GABACIFY_LOG_DEBUG<<"reuse of config";
                if (configuration.transformedSequenceConfigurations.at(i).lutTransformationEnabled)
                {
                    appendToBytestream(tempSubseq0Lut0, bytestream);
                }
                appendToBytestream(tempSubseq0, bytestream);
                continue;
            }
        } else if (i == 1)
        {
            //TODO: i wanted something like config1.equalTo(config2, i)
            if (configuration.equal(configurationTempSubseq1, i))
            {
                GABACIFY_LOG_DEBUG<<"reuse of config";
                if (configuration.transformedSequenceConfigurations.at(i).lutTransformationEnabled)
                {
                    appendToBytestream(tempSubseq0Lut1, bytestream);
                }
                appendToBytestream(tempSubseq1, bytestream);
                continue;
            }
        }
        else
        {
            // For i == 2 (which only occcurs using match_coding) the
            // configuration will always change. Thus a re-use does not make
            // sense.
        }

        GABACIFY_LOG_TRACE << "Processing transformed sequence: " << i;
        auto transformedSequence = transformedSequences.at(i);
        auto transformedSequenceConfiguration = configuration.transformedSequenceConfigurations.at(i);
        std::string seqTransformExtension="";
        if(configuration.sequenceTransformationId == gabac::SequenceTransformationId::match_coding)
        {
          unsigned int windowSize = configuration.sequenceTransformationParameter;
          switch(i)
          {
            case 0:
                wordSize = 4;
                seqTransformExtension = ".match." + std::to_string(windowSize) + ".pointers.tmp";
                break;
            case 1:
                wordSize = 4;
                seqTransformExtension = ".match." + std::to_string(windowSize) + ".lengths.tmp";
                break;
            case 2:
                wordSize = configuration.wordSize;
                seqTransformExtension = ".match." + std::to_string(windowSize) + ".values.tmp";
                break;
            default:
                wordSize = configuration.wordSize;
                break;
          }
        }
        else if (configuration.sequenceTransformationId == gabac::SequenceTransformationId::equality_coding)
        {
          switch(i)
          {
            case 0:
                wordSize=1;
                seqTransformExtension = ".eq.equalityFlags.tmp";
                break;
            case 1:
                wordSize=configuration.wordSize;
                seqTransformExtension = ".eq.values.tmp";
                break;
            default:
                wordSize = configuration.wordSize;
                break;
          }
        }
        else if (configuration.sequenceTransformationId == gabac::SequenceTransformationId::rle_coding)
        {
          unsigned int guard = configuration.sequenceTransformationParameter;
          switch(i)
          {
            case 0:
                wordSize=configuration.wordSize;
                seqTransformExtension = ".rle."+std::to_string(guard)+".values.tmp";
                break;
            case 1:
                wordSize=4;
                seqTransformExtension = ".rle."+std::to_string(guard)+".lengths.tmp";
                break;
            default:
                wordSize = configuration.wordSize;
                break;
          }
        }
        else
        {
            wordSize = configuration.wordSize;
        }

        std::vector<uint64_t> lutTransformedSequence;
        std::vector<unsigned char> inverseLutBitstream;
        std::vector<int64_t> inverseLutTmp;
        std::string lutTransformExtension = "";
        if (transformedSequenceConfiguration.lutTransformationEnabled)
        {
            GABACIFY_LOG_TRACE << "LUT transform *en*abled";
            lutTransformExtension="."+std::to_string(i)+".lutTransformed.tmp";
            // Do the LUT transform
            std::vector<uint64_t> inverseLut;
            GABACIFY_LOG_DEBUG<<"used file: "+inputFilePath+seqTransformExtension+lutTransformExtension;
            if(fileExists(inputFilePath+seqTransformExtension+lutTransformExtension))
            {
                GABACIFY_LOG_DEBUG<<"files exist";
                InputFile inputFileSubseq0(inputFilePath+seqTransformExtension+lutTransformExtension);
                InputFile inputFileSubseq1(inputFilePath+seqTransformExtension+"."+std::to_string(i)+".lut.tmp");
                size_t bufferSize = inputFileSubseq0.size();
                std::vector<unsigned char> buffer(bufferSize);
                inputFileSubseq0.read(&buffer[0], 1, bufferSize);
                generateSymbolStream(buffer, wordSize, &lutTransformedSequence);
                //read lut
                bufferSize = inputFileSubseq1.size();
                buffer.resize(bufferSize);
                inputFileSubseq1.read(&buffer[0], 1, bufferSize);
                generateSymbolStream(buffer, wordSize, &inverseLut);
                for (const auto& inverseLutEntry : inverseLut)
                {
                    assert(inverseLutEntry <= std::numeric_limits<int64_t>::max());
                    inverseLutTmp.push_back(static_cast<int64_t>(inverseLutEntry));
                }
                GABACIFY_LOG_DEBUG << "Lut Transformed sequence read with size: " << lutTransformedSequence.size();
                GABACIFY_LOG_DEBUG << "lut read with size: " << inverseLut.size();
            }
            else
            {
              GABACIFY_LOG_DEBUG<<"files do not exist";
              gabac::transformLutTransform0(transformedSequence, &lutTransformedSequence, &inverseLut);
              for (const auto& inverseLutEntry : inverseLut)
              {
                  assert(inverseLutEntry <= std::numeric_limits<int64_t>::max());
                  inverseLutTmp.push_back(static_cast<int64_t>(inverseLutEntry));
              }
              GABACIFY_LOG_DEBUG << "Generated Lut transformed sequence with size: " << lutTransformedSequence.size();
              GABACIFY_LOG_DEBUG << "lut with size: " << inverseLut.size();
              OutputFile outputFileSubseq0(inputFilePath+seqTransformExtension+lutTransformExtension);
              OutputFile outputFileSubseq1(inputFilePath+seqTransformExtension+"."+std::to_string(i)+".lut.tmp");
              //schrijf transformed values
              std::vector<unsigned char> transformedValueBuffer;
              generateByteBuffer(lutTransformedSequence, wordSize, &transformedValueBuffer);
              outputFileSubseq0.write(&transformedValueBuffer[0], 1, transformedValueBuffer.size());
              //schrijf lut
              std::vector<unsigned char> lutBuffer;
              generateByteBuffer(inverseLut, wordSize, &lutBuffer);
              outputFileSubseq1.write(&lutBuffer[0], 1, lutBuffer.size());
            }
            // GABACIFY_LOG_DEBUG<<"lut size before coding: "<<inverseLutTmp
            gabac::encode(
                inverseLutTmp,
                gabac::BinarizationId::BI,
                { wordSize * 8 },
                gabac::ContextSelectionId::bypass,
                &inverseLutBitstream
            );
            appendToBytestream(inverseLutBitstream, bytestream);
            GABACIFY_LOG_TRACE << "Wrote LUT bitstream with size: " << inverseLutBitstream.size();
            if (i == 0)
            {
              tempSubseq0Lut0 = std::move(inverseLutBitstream);
            } else if (i == 1)
            {
              tempSubseq0Lut1 = std::move(inverseLutBitstream);
            }
        }
        else
        {
            GABACIFY_LOG_TRACE << "LUT transform *dis*abled";
            lutTransformedSequence = std::move(transformedSequence);
        }

        // Diff coding
        std::vector<int64_t> diffAndLutTransformedSequence;
        if (transformedSequenceConfiguration.diffCodingEnabled)
        {
            GABACIFY_LOG_TRACE << "Diff coding *en*abled";
            gabac::transformDiffCoding(lutTransformedSequence, &diffAndLutTransformedSequence);
        }
        else
        {
            GABACIFY_LOG_TRACE << "Diff coding *dis*abled";
            for (const auto& lutTransformedSymbol : lutTransformedSequence)
            {
                assert(lutTransformedSymbol <= std::numeric_limits<int64_t>::max());
                diffAndLutTransformedSequence.push_back(static_cast<int64_t>(lutTransformedSymbol));
            }
        }

        // Encoding
        std::vector<unsigned char> bitstream;
        gabac::encode(
                diffAndLutTransformedSequence,
                transformedSequenceConfiguration.binarizationId,
                transformedSequenceConfiguration.binarizationParameters,
                transformedSequenceConfiguration.contextSelectionId,
                &bitstream
        );
        GABACIFY_LOG_TRACE << "Bitstream size: " << bitstream.size();
        appendToBytestream(bitstream, bytestream);
        if (i == 0)
        {
          tempSubseq0 = std::move(bitstream);
          configurationTempSubseq0 = configuration;
        }
        else if (i == 1)
        {
          tempSubseq1 = std::move(bitstream);
          configurationTempSubseq1 = configuration;
        }
    }
}


void encode(
        const std::string& inputFilePath,
        bool analyze,
        const std::string& configurationFilePath,
        const std::string& outputFilePath
){
    assert(!inputFilePath.empty());
    assert(!configurationFilePath.empty());
    assert(!outputFilePath.empty());

    // Read in the entire input file
    InputFile inputFile(inputFilePath);
    size_t bufferSize = inputFile.size();
    std::vector<unsigned char> buffer(bufferSize);
    inputFile.read(&buffer[0], 1, bufferSize);

    if (analyze)
    {
        // In analysis mode we assume a word size of 1
        unsigned int wordSize = 1;

        // Generate symbol stream from byte buffer
        std::vector<uint64_t> symbols;
        generateSymbolStream(buffer, wordSize, &symbols);
        // if (symbols.size()>2*1024*1024)
        //   symbols.erase(symbols.begin()+2*1024*1024,symbols.end());

        // Analyze the input symbols and generate all valid configurations.
        std::vector<Configuration> configurations;
        GABACIFY_LOG_INFO << "Defining configurations ...";
        defineConfigurations(symbols, wordSize, &configurations);

        std::vector<unsigned char> smallestBytestream;
        size_t smallestBytestreamSize = std::numeric_limits<size_t>::max();
        Configuration bestConfiguration;
        size_t configurationCounter = 0;
        GABACIFY_LOG_INFO << "Trying " << configurations.size() << " configurations";
        for (const auto& configuration : configurations)
        {
            std::vector<unsigned char> bytestream;
            encodeWithConfiguration(inputFilePath, symbols, configuration, &bytestream);
            if (bytestream.size() < smallestBytestreamSize)
            {
                smallestBytestream = std::move(bytestream);
                smallestBytestreamSize = smallestBytestream.size();
                bestConfiguration = configuration;
                GABACIFY_LOG_INFO << "Found new best configuration (#" << configurationCounter << "; compressed size: " << smallestBytestream.size() << ")";
                GABACIFY_LOG_INFO << configuration.toPrintableString();
            }
            configurationCounter++;
            if(configurationCounter%1000==0)
            {
              GABACIFY_LOG_INFO << "Progress: " <<configurationCounter<<" of "<<configurations.size();
            }
        }

        // Write the smallest bytestream
        OutputFile outputFile(outputFilePath);
        outputFile.write(&smallestBytestream[0], 1, smallestBytestream.size());
        GABACIFY_LOG_INFO << "Wrote smallest bytestream of size " << smallestBytestream.size() << " to: " << outputFilePath;

        // Write the best configuration as JSON
        std::string jsonString = bestConfiguration.toJsonString();
        OutputFile configurationFile(configurationFilePath);
        configurationFile.write(&jsonString[0], 1, jsonString.size());
        GABACIFY_LOG_INFO << "Wrote best configuration to: " << configurationFilePath;
    }
    else
    {
        // Read the entire configuration file as a string and convert the JSON
        // input string to the internal GABAC configuration
        InputFile configurationFile(configurationFilePath);
        std::string jsonInput("\0", configurationFile.size());
        configurationFile.read(&jsonInput[0], 1, jsonInput.size());
        Configuration configuration(jsonInput);

        // Generate symbol stream from byte buffer
        std::vector<uint64_t> symbols;
        generateSymbolStream(buffer, configuration.wordSize, &symbols);

        // Encode with the given configuration
        std::vector<unsigned char> bytestream;
        encodeWithConfiguration(inputFilePath, symbols, configuration, &bytestream);

        // Write the bytestream
        OutputFile outputFile(outputFilePath);
        outputFile.write(&bytestream[0], 1, bytestream.size());
        GABACIFY_LOG_INFO << "Wrote bytestream of size " << bytestream.size() << " to: " << outputFilePath;
    }
}


}  // namespace gabacify
