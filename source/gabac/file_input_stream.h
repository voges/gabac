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
    explicit FileInputStream(FILE *file);

    size_t readStream(DataBlock *buffer) override;

    size_t readBytes(size_t size, DataBlock *buffer) override;

    size_t readFull(DataBlock *buffer) override;
    bool isValid() override;

    size_t getTotalSize() override;

    size_t getRemainingSize() override;
};

}


#endif //PROJECT_FILE_INPUT_STREAM_H
