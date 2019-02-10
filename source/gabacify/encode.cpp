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
#include "gabac/configuration.h"
#include "gabac/file_input_stream.h"
#include "gabac/file_output_stream.h"
#include "gabac/exceptions.h"
#include "gabacify/input_file.h"
#include "gabacify/tmp_file.h"


namespace gabacify {

//------------------------------------------------------------------------------

void encode_plain(const std::string& inputFilePath,
                  const std::string& configurationFilePath,
                  const std::string& outputFilePath,
                  size_t blocksize
){
    InputFile configurationFile(configurationFilePath);
    std::string jsonInput("\0", configurationFile.size());
    configurationFile.read(&jsonInput[0], 1, jsonInput.size());
    gabac::Configuration configuration(jsonInput);

    // Read in the entire input file
    InputFile inputFile(inputFilePath);
    gabac::FileInputStream instream(inputFile.handle());

    // Write the bytestream
    OutputFile outputFile(outputFilePath);
    gabac::FileOutputStream outstream(outputFile.handle());

    // Convert Symbol stream to bytestream
    gabac::encode(configuration, &instream, &outstream, blocksize);

    /*GABACIFY_LOG_INFO << "Wrote bytestream of size "
                      << outstream.bytesWritten()
                      << " to: "
                      << outputFilePath;*/
}

//------------------------------------------------------------------------------

void encode(
        const std::string& inputFilePath,
        bool analyze,
        const std::string& configurationFilePath,
        const std::string& outputFilePath,
        size_t blocksize
){
    assert(!inputFilePath.empty());
    assert(!configurationFilePath.empty());
    assert(!outputFilePath.empty());

    if (analyze) {
        encode_analyze(inputFilePath, configurationFilePath, outputFilePath);
        return;
    }
    encode_plain(inputFilePath, configurationFilePath, outputFilePath, blocksize);
}

//------------------------------------------------------------------------------

}  // namespace gabacify

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
