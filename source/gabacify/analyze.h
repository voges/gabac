#ifndef GABACIFY_ANALYZE_H_
#define GABACIFY_ANALYZE_H_

#include <string>

namespace gabacify {

void analyze(const std::string& inputFilePath,
             const std::string& configurationFilePath,
             size_t blocksize
);

}  // namespace gabacify

#endif  // GABACIFY_ANALYZE_H_
