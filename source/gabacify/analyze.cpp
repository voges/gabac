#include <fstream>
#include <iostream>

#include <gabac/gabac.h>
#include "analyze.h"

#include "gabac/analysis.h"
#include "gabac/exceptions.h"
#include "gabac/streams.h"

namespace gabacify {
void analyze(const std::string& inputFilePath,
             const std::string& configurationFilePath,
             size_t blocksize
){
    std::ifstream inputFile;
    std::ofstream configurationFile;
    gabac::NullStream nullstream;

    std::istream *istream = &std::cin;
    std::ostream *ostream = &std::cout;
    std::ostream *logstream = &std::cout;

    if (!inputFilePath.empty()) {
        inputFile = std::ifstream(inputFilePath, std::ios::binary);
        if (!inputFile) {
            GABAC_DIE("Could not open input file");
        }
        istream = &inputFile;
    }
    if (!configurationFilePath.empty()) {
        configurationFile = std::ofstream(configurationFilePath, std::ios::binary);
        if (!configurationFile) {
            GABAC_DIE("Could not open output file");
        }
        ostream = &configurationFile;
    } else {
        logstream = &nullstream;
    }

    gabac::IOConfiguration ioConf = {istream,
                                     ostream,
                                     blocksize,
                                     logstream,
                                     gabac::IOConfiguration::LogLevel::TRACE
    };


    gabac::analyze(ioConf, gabac::getCandidateConfig());
    /*GABACIFY_LOG_INFO << "Wrote smallest bytestream of size "
                      << bestByteStream.size()
                      << " to: "
                      << outputFilePath;*/

    // Write the best configuration as JSON
    /*GABACIFY_LOG_DEBUG << "with configuration: \n"
                       << bestConfig.toPrintableString();
    //GABACIFY_LOG_INFO << "Wrote best configuration to: " << configurationFilePath;*/
}
}