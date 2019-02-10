#ifndef PROJECT_OUTPUT_STREAM_H
#define PROJECT_OUTPUT_STREAM_H

#include <cstddef>

namespace gabac {

class DataBlock;

class OutputStream
{
 public:
    virtual size_t writeStream(DataBlock *buffer) = 0;
    virtual size_t writeBytes(DataBlock *buffer) = 0;
    virtual size_t bytesWritten() = 0;
    virtual ~OutputStream() = default;
};
}


#endif //PROJECT_OUTPUT_STREAM_H
