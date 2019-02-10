#ifndef PROJECT_INPUT_STREAM_H
#define PROJECT_INPUT_STREAM_H

#include <cstddef>

namespace gabac {

class DataBlock;

class InputStream {
 public:
    virtual size_t readStream (DataBlock* buffer) = 0;
    virtual size_t readBytes(size_t size, DataBlock* buffer) = 0;
    virtual size_t readFull(DataBlock* buffer) = 0;
    virtual bool isValid() = 0;
    virtual ~InputStream() = default;
    virtual size_t getTotalSize() = 0;
    virtual size_t getRemainingSize() = 0;
};
}

#endif //PROJECT_INPUT_STREAM_H
