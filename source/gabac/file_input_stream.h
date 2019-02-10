//
// Created by fabian on 2/10/19.
//

#ifndef PROJECT_FILE_INPUT_STREAM_H
#define PROJECT_FILE_INPUT_STREAM_H

#include "gabac/input_stream.h"

#include <cstdio>
#include <cstdint>
#include <stdexcept>

namespace gabac {

class FileInputStream : public InputStream
{
    FILE *input;
    size_t fpos;
    size_t fsize;
 public:
    FileInputStream(FILE *file);

    size_t readStream(DataBlock *buffer);

    size_t readBytes(size_t size, DataBlock *buffer);

    size_t readFull(DataBlock *buffer);
    bool isValid();

    size_t getTotalSize();

    size_t getRemainingSize();
};

}


#endif //PROJECT_FILE_INPUT_STREAM_H
