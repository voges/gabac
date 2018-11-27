#ifndef GABACIFY_ENCODE_H_
#define GABACIFY_ENCODE_H_


#include <string>


namespace gabacify {


void encode(
        const std::string& inputFilePath,
        bool analyze,
        const std::string& configurationFilePath,
        const std::string& outputFilePath
);


}  // namespace gabacify


#endif  // GABACIFY_ENCODE_H_
