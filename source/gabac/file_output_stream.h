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
    FileOutputStream(FILE *f);

    size_t writeStream(DataBlock *inbuffer);

    size_t writeBytes(DataBlock *inbuffer);

    size_t bytesWritten();
};
}

#endif //PROJECT_FILE_OUTPUT_STREAM_H
