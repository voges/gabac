#include "file_input_stream.h"

#include "gabac/data_block.h"

namespace gabac {
FileInputStream::FileInputStream(FILE
*file) :
input(file){
        fpos = ftell(input);
        fseek(input, 0, SEEK_END);
        fsize = ftell(input);
        fseek(input, fpos, SEEK_SET);
}

size_t FileInputStream::readStream(DataBlock *buffer){
    uint32_t streamSize = 0;
    if (fread(&streamSize, sizeof(uint32_t), 1, input) != 1) {
        throw std::runtime_error("Unexpected file end");
    }
    fpos += +sizeof(uint32_t);
    return readBytes(streamSize, buffer);
}

size_t FileInputStream::readBytes(size_t size, DataBlock *buffer){
    buffer->resize(size);
    if (fread(buffer->getData(), 1, size, input) != size) {
        throw std::runtime_error("Unexpected file end");
    }
    return fpos += size;
}

size_t FileInputStream::readFull(DataBlock *buffer){
    return readBytes(getRemainingSize(), buffer);
}

bool FileInputStream::isValid(){
    return fpos < fsize;
}

size_t FileInputStream::getTotalSize(){
    return fsize;
}

size_t FileInputStream::getRemainingSize(){
    return fsize - fpos;
}
}