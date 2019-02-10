#include "file_output_stream.h"

#include "gabac/data_block.h"

namespace gabac {
FileOutputStream::FileOutputStream(FILE *f) : file(f){

}

size_t FileOutputStream::writeStream(DataBlock *inbuffer){
    uint32_t size = inbuffer->size() * inbuffer->getWordSize();
    auto *ptr = (uint8_t *) &size;
    fwrite(ptr, sizeof(uint32_t), 1, file);

    return writeBytes(inbuffer) + sizeof(uint32_t);
}

size_t FileOutputStream::writeBytes(DataBlock *inbuffer){
    size_t ret = fwrite(inbuffer->getData(), 1, inbuffer->size() * inbuffer->getWordSize(), file);
    inbuffer->clear();
    inbuffer->shrink_to_fit();
    return ret;
}

size_t FileOutputStream::bytesWritten(){
    return ftell(file);
}
}