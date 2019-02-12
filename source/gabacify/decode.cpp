#include "gabacify/decode.h"


#include <cassert>
#include <cmath>
#include <vector>

#include "gabac/constants.h"
#include "gabac/diff_coding.h"
#include "gabac/decoding.h"

#include "gabac/configuration.h"
#include "gabac/file_input_stream.h"
#include "gabac/file_output_stream.h"
#include "gabac/exceptions.h"
#include "gabacify/input_file.h"
#include "gabacify/output_file.h"


namespace gabacify {

//------------------------------------------------------------------------------

void decode(
        const std::string& inputFilePath,
        const std::string& configurationFilePath,
        const std::string& outputFilePath
){
    assert(!inputFilePath.empty());
    assert(!configurationFilePath.empty());
    assert(!outputFilePath.empty());

    // Read in the entire input file
    InputFile inputFile(inputFilePath);
    gabac::FileInputStream inStream(inputFile.handle());

    // Read the entire configuration file as a string and convert the JSON
    // input string to the internal GABAC configuration
    InputFile configurationFile(configurationFilePath);
    std::string jsonInput("\0", configurationFile.size());
    configurationFile.read(&jsonInput[0], 1, jsonInput.size());
    gabac::EncodingConfiguration configuration(jsonInput);

    // Write the bytestream
    OutputFile outputFile(outputFilePath);
    gabac::FileOutputStream outStream(outputFile.handle());

    gabac::IOConfiguration ioconf = {&inStream,
                                      &outStream,
                                      0,
                                      &std::cout,
                                      gabac::IOConfiguration::LogLevel::INFO
    };

    gabac::decode(ioconf, configuration);


    /*GABACIFY_LOG_INFO << "Wrote buffer of size "
                      << outStream.bytesWritten()
                      << " to: "
                      << outputFilePath;*/
}

//------------------------------------------------------------------------------

}  // namespace gabacify

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------