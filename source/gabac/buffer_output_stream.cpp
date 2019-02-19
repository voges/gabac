#include "buffer_output_stream.h"

namespace gabac {
BufferOutputStream::BufferOutputStream() : buffer(0, 1){

}

size_t BufferOutputStream::writeStream(DataBlock *inbuffer){
    auto size = static_cast<uint32_t>(inbuffer->size() * inbuffer->getWordSize());
    auto *ptr = (uint8_t *) &size;
    buffer.push_back(*(ptr++));
    buffer.push_back(*(ptr++));
    buffer.push_back(*(ptr++));
    buffer.push_back(*(ptr));

    return BufferOutputStream::writeBytes(inbuffer) + sizeof(uint32_t);
}

size_t BufferOutputStream::writeBytes(DataBlock *inbuffer){
    size_t oldSize = buffer.size();
    size_t newSize = inbuffer->size() * inbuffer->getWordSize();
    buffer.resize(oldSize + newSize);
    memcpy((uint8_t *) buffer.getData() + oldSize, inbuffer->getData(), newSize);
    inbuffer->clear();
    inbuffer->shrink_to_fit();
    return newSize;
}

void BufferOutputStream::flush(DataBlock *outbuffer){
    outbuffer->swap(&buffer);
    buffer = DataBlock(0, 1);
}

size_t BufferOutputStream::bytesWritten(){
    return buffer.size();
}
}