#ifndef PROJECT_ANALYZE_H
#define PROJECT_ANALYZE_H

#include <string>

namespace gabacify {

void analyze(const std::string& inputFilePath,
             const std::string& configurationFilePath,
             size_t blocksize
);

}

#endif //PROJECT_ANALYZE_H
