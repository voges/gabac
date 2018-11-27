#ifndef GABACIFY_DECODE_H_
#define GABACIFY_DECODE_H_


#include <string>


namespace gabacify {


void decode(
        const std::string& inputFilePath,
        const std::string& configurationFilePath,
        const std::string& outputFilePath
);


}  // namespace gabacify


#endif  // GABACIFY_DECODE_H_
