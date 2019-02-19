#ifndef PROJECT_FILE_OUTPUT_STREAM_H
#define PROJECT_FILE_OUTPUT_STREAM_H

#include "gabac/output_stream.h"

#include <cstdio>
#include <cstddef>
#include <cstdint>

namespace gabac {

class FileOutputStream : public OutputStream
{
    FILE *file;
 public:
    explicit FileOutputStream(FILE *f);

    size_t writeStream(DataBlock *inbuffer) override;

    size_t writeBytes(DataBlock *inbuffer) override;

    size_t bytesWritten() override;
};
}

#endif //PROJECT_FILE_OUTPUT_STREAM_H
