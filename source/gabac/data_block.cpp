#include "data_block.h"

namespace gabac {

BlockStepper DataBlock::getReader() const{
    return {(uint8_t *) data.data(), (uint8_t *) data.end().base(), wordSize};
}

bool DataBlock::operator==(const DataBlock& d) const{
    return wordSize == d.wordSize && data == d.data;
}

DataBlock& DataBlock::operator=(const std::initializer_list<uint64_t>& il){
    resize(il.size());
    size_t ctr = 0;
    for (const auto& v : il) {
        set(ctr, v);
        ++ctr;
    }
    return *this;
}

size_t DataBlock::size() const{
    return data.size() / wordSize;
}

void DataBlock::reserve(size_t size){
    data.reserve(size * wordSize);
}

void DataBlock::shrink_to_fit(){
    data.shrink_to_fit();
}

void DataBlock::clear(){
    data.clear();
}

void DataBlock::resize(size_t size){
    data.resize(size * wordSize);
}

bool DataBlock::empty() const{
    return data.empty();
}

void DataBlock::swap(DataBlock *const d){
    size_t tmp = wordSize;
    wordSize = d->wordSize;
    d->wordSize = static_cast<uint8_t>(tmp);
    data.swap(d->data);
}


DataBlock::DataBlock(size_t size, size_t wsize) : wordSize(static_cast<uint8_t>(wsize)), data(size * wsize){
}

}