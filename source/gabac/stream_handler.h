#ifndef GABAC_STREAM_HANDLER_H
#define GABAC_STREAM_HANDLER_H

#include <cstddef>
#include <istream>
#include <ostream>

namespace gabac {

class DataBlock;

class StreamHandler
{
 public:
    static size_t readStream(std::istream& input, DataBlock *buffer);
    static size_t readBytes(std::istream& input, size_t bytes, DataBlock *buffer);
    static size_t readFull(std::istream& input, DataBlock *buffer);
    static size_t readBlock(std::istream& input, size_t bytes, DataBlock *buffer);
    static size_t writeStream(std::ostream& output, DataBlock *buffer);
    static size_t writeBytes(std::ostream& output, DataBlock *buffer);
};
}

#endif //GABAC_STREAM_HANDLER_H
