#include <gabac/gabac.h>
#include "analyze.h"

#include "gabac/analysis.h"
#include "gabacify/input_file.h"
#include "output_file.h"

namespace gabacify {
void analyze(const std::string& inputFilePath,
             const std::string& configurationFilePath
){
    gabac::EncodingConfiguration bestConfig;

    InputFile inputFile(inputFilePath);
    gabac::FileInputStream instream(inputFile.handle());
    gabac::BufferOutputStream outstream;

    gabac::IOConfiguration ioConf = {&instream,
                                     &outstream,
                                     0,
                                     &std::cout,
                                     gabac::IOConfiguration::LogLevel::TRACE
    };

    gabac::analyze(ioConf,gabac::getCandidateConfig(), &bestConfig);
    /*GABACIFY_LOG_INFO << "Wrote smallest bytestream of size "
                      << bestByteStream.size()
                      << " to: "
                      << outputFilePath;*/

    // Write the best configuration as JSON
    std::string jsonString = bestConfig.toJsonString();
    OutputFile configurationFile(configurationFilePath);
    configurationFile.write(&jsonString[0], 1, jsonString.size());
    /*GABACIFY_LOG_DEBUG << "with configuration: \n"
                       << bestConfig.toPrintableString();
    //GABACIFY_LOG_INFO << "Wrote best configuration to: " << configurationFilePath;*/
}
}