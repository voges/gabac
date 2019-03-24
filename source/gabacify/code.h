#ifndef GABACIFY_ENCODE_H_
#define GABACIFY_ENCODE_H_

#include <cstddef>
#include <string>

namespace gabacify {


void code(const std::string& inputFilePath,
          const std::string& configurationFilePath,
          const std::string& outputFilePath,
          size_t blocksize,
          bool decode
);


}  // namespace gabacify


#endif  // GABACIFY_ENCODE_H_
