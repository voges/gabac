#ifndef GABACIFY_ANALYSIS_H_
#define GABACIFY_ANALYSIS_H_


#include <vector>

#include "gabacify/configuration.h"


namespace gabacify {

void encode_analyze(const std::string& inputFilePath,
                    const std::string& configurationFilePath,
                    const std::string& outputFilePath
);
}


#endif  // GABACIFY_ANALYSIS_H_
