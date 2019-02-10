//
// Created by fabian on 2/10/19.
//

#ifndef PROJECT_BUFFER_INPUT_STREAM_H
#define PROJECT_BUFFER_INPUT_STREAM_H

#include "gabac/data_block.h"
#include "gabac/input_stream.h"
#include "gabac/block_stepper.h"

#include <cstddef>
#include <cstdint>

namespace gabac {

class BufferInputStream : public InputStream
{
 private:
    DataBlock mainBuffer;
    BlockStepper r;
 public:
    BufferInputStream(DataBlock *stream);

    size_t readStream(DataBlock *buffer) override;

    size_t readBytes(size_t size, DataBlock *buffer) override;

    size_t readFull(DataBlock *buffer) override;

    bool isValid() override;

    size_t getTotalSize() override;

    size_t getRemainingSize() override;
};
}


#endif //PROJECT_BUFFER_INPUT_STREAM_H
