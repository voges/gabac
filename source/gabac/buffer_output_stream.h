#ifndef PROJECT_BUFFER_OUTPUT_STREAM_H
#define PROJECT_BUFFER_OUTPUT_STREAM_H

#include "gabac/output_stream.h"
#include "gabac/data_block.h"

#include <cstdint>
#include <cstddef>

namespace gabac {

class BufferOutputStream : public OutputStream
{
    DataBlock buffer;
 public:
    BufferOutputStream();
    size_t writeStream(DataBlock *inbuffer) override;
    size_t writeBytes(DataBlock *inbuffer) override;
    void flush(DataBlock *outbuffer);
    size_t bytesWritten() override;
};

}

#endif //PROJECT_BUFFER_OUTPUT_STREAM_H
