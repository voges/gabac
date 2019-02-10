#include "buffer_input_stream.h"

namespace gabac {
BufferInputStream::BufferInputStream(DataBlock *stream) : mainBuffer(0, 1){
    mainBuffer.swap(stream);
    mainBuffer.setWordSize(1);
    r = mainBuffer.getReader();
}

size_t BufferInputStream::readStream(DataBlock *buffer){
    uint32_t streamSize = 0;
    auto *streamPtr = (uint8_t *) &streamSize;
    *streamPtr = (uint8_t) r.get();
    r.inc();
    *streamPtr = (uint8_t) r.get();
    r.inc();
    *streamPtr = (uint8_t) r.get();
    r.inc();
    *streamPtr = (uint8_t) r.get();
    r.inc();

    return BufferInputStream::readBytes(streamSize, buffer);
}

size_t BufferInputStream::readBytes(size_t size, DataBlock *buffer){
    *buffer = DataBlock(0, 1);
    buffer->resize(size);
    memcpy(buffer->getData(), this->mainBuffer.getData(), size);
    r.curr += size;
    return size;
}

size_t BufferInputStream::readFull(DataBlock *buffer){
    mainBuffer.swap(buffer);
    return buffer->size();
}

bool BufferInputStream::isValid(){
    return r.isValid();
}

size_t BufferInputStream::getTotalSize(){
    return mainBuffer.size();
}

size_t BufferInputStream::getRemainingSize(){
    return r.end - r.curr;
}
}