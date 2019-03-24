#ifndef GABAC_DATA_BLOCK_H_
#define GABAC_DATA_BLOCK_H_

#include <cstring>
#include <string>
#include <utility>
#include <vector>

#include "gabac/exceptions.h"

namespace gabac {

struct BlockStepper;

class DataBlock
{
 private:
    uint8_t wordSize;

    std::vector<uint8_t> data;

 public:
    const BlockStepper getReader() const;
    bool operator==(const DataBlock& d) const;
    DataBlock& operator=(const std::initializer_list<uint64_t>& il);

    uint64_t get(size_t index) const;

    void set(size_t index, uint64_t val);

    template<typename T>
    class ProxyCore
    {
     private:
        T stream;
        size_t position;
     public:
        ProxyCore(T str, size_t pos);

        explicit operator uint64_t() const;

        ProxyCore& operator=(uint64_t val);
    };

    using Proxy = ProxyCore<DataBlock *>;
    using ConstProxy = ProxyCore<const DataBlock *>;

    template<typename T>
    class IteratorCore
    {
     private:
        T stream;
        size_t position;
     public:
        IteratorCore(T str, size_t pos);

        IteratorCore operator+(size_t offset) const;

        IteratorCore operator-(size_t offset) const;

        size_t operator-(const IteratorCore& offset) const{
            return position - offset.position;
        }

        IteratorCore& operator++();

        IteratorCore& operator--();

        const IteratorCore operator++(int);

        const IteratorCore operator--(int);
        size_t getOffset() const;
        T getStream() const;
        ProxyCore<T> operator*() const;
        bool operator==(const IteratorCore& c) const;

        bool operator!=(const IteratorCore& c) const;

        using iterator_category = std::random_access_iterator_tag;
        using reference = ProxyCore<T>;
        using pointer = ProxyCore<T> *;
        using value_type = ProxyCore<T>;
        using difference_type = size_t;
    };

    using Iterator = IteratorCore<DataBlock *>;
    using ConstIterator = IteratorCore<const DataBlock *>;

    size_t size() const;

    void reserve(size_t size);

    void shrink_to_fit();
    void clear();

    void resize(size_t size);

    bool empty() const;

    ConstIterator begin() const;

    Iterator begin();

    ConstIterator end() const;

    Iterator end();

    void push_back(uint64_t val);

    void emplace_back(uint64_t val);

    const void *getData() const;

    void *getData();

    uint8_t getWordSize() const;

    void setWordSize(uint8_t size);

    size_t getRawSize() const{
        return getWordSize() * size();
    }

    void swap(DataBlock *d);


    template<typename IT1, typename IT2>
    void insert(const IT1& pos, const IT2& start, const IT2& end);

    explicit DataBlock(size_t size = 0, size_t wsize = 1);

    template<typename T>
    explicit DataBlock(std::vector<T> *vec);

    explicit DataBlock(std::vector<uint8_t> *vec);
    explicit DataBlock(std::string *vec);

    explicit DataBlock(uint8_t *d, size_t size, uint8_t word_size);
};


inline uint64_t DataBlock::get(size_t index) const{
    switch (wordSize) {
        case 1:
            return *(data.data() + index);
        case 2:
            return *reinterpret_cast<const uint16_t *> (data.data() + (index << 1u));
        case 4:
            return *reinterpret_cast<const uint32_t *> (data.data() + (index << 2u));
        case 8:
            return *reinterpret_cast<const uint64_t *> (data.data() + (index << 3u));
        default:
            return 0;
    }
}

inline void DataBlock::set(size_t index, uint64_t val){
    switch (wordSize) {
        case 1:
            *(data.data() + index) = static_cast<uint8_t>(val);
            return;
        case 2:
            *reinterpret_cast<uint16_t *> (data.data() + (index << 1u)) = static_cast<uint16_t>(val);
            return;
        case 4:
            *reinterpret_cast<uint32_t *> (data.data() + (index << 2u)) = static_cast<uint32_t>(val);
            return;
        case 8:
            *reinterpret_cast<uint64_t *> (data.data() + (index << 3u)) = static_cast<uint64_t>(val);
            return;
        default:
            return;
    }
}


inline DataBlock::ConstIterator DataBlock::begin() const{
    return {this, 0};
}

inline DataBlock::Iterator DataBlock::begin(){
    return {this, 0};
}

inline DataBlock::ConstIterator DataBlock::end() const{
    return {this, data.size() / wordSize};
}

inline DataBlock::Iterator DataBlock::end(){
    return {this, data.size() / wordSize};
}

inline void DataBlock::push_back(uint64_t val){
    /*
      set(data.size() / wordSize - 1, val);*/
    data.resize(data.size() + wordSize);
    switch (wordSize) {
        case 1:
            *reinterpret_cast<uint8_t *> (data.end().base() - 1) = static_cast<uint8_t>(val);
            return;
        case 2:
            *reinterpret_cast<uint16_t *> (data.end().base() - 2) = static_cast<uint16_t>(val);
            return;
        case 4:
            *reinterpret_cast<uint32_t *> (data.end().base() - 4) = static_cast<uint32_t>(val);
            return;
        case 8:
            *reinterpret_cast<uint64_t *> (data.end().base() - 8) = static_cast<uint64_t>(val);
            return;
        default:
            break;
    }
}

inline void DataBlock::emplace_back(uint64_t val){
    push_back(val);
}

inline const void *DataBlock::getData() const{
    return data.data();
}

inline void *DataBlock::getData(){
    return data.data();
}

inline uint8_t DataBlock::getWordSize() const{
    return wordSize;
}

inline void DataBlock::setWordSize(uint8_t size){
    wordSize = size;
    if (data.size() % size) {
        GABAC_DIE("Could not resize");
    }
}


template<typename T>
inline DataBlock::ProxyCore<T>::ProxyCore(T str, size_t pos) : stream(str), position(pos){
}

template<typename T>
inline DataBlock::ProxyCore<T>::operator uint64_t() const{
    return stream->get(position);
}

template<typename T>
inline DataBlock::ProxyCore<T>& DataBlock::ProxyCore<T>::operator=(uint64_t val){
    stream->set(position, val);
    return *this;
}

template<typename T>
DataBlock::IteratorCore<T>::IteratorCore(T str, size_t pos) : stream(str), position(pos){
}

template<typename T>
inline DataBlock::IteratorCore<T> DataBlock::IteratorCore<T>::operator+(size_t offset) const{
    return IteratorCore(stream, position + offset);
}

template<typename T>
inline DataBlock::IteratorCore<T> DataBlock::IteratorCore<T>::operator-(size_t offset) const{
    return IteratorCore(stream, position - offset);
}

template<typename T>
inline DataBlock::IteratorCore<T>& DataBlock::IteratorCore<T>::operator++(){
    *this = *this + 1;
    return *this;
}

template<typename T>
inline DataBlock::IteratorCore<T>& DataBlock::IteratorCore<T>::operator--(){
    *this = *this + 1;
    return *this;
}

template<typename T>
inline const DataBlock::IteratorCore<T> DataBlock::IteratorCore<T>::operator++(int){
    IteratorCore ret = *this;
    ++(*this);
    return ret;
}

template<typename T>
inline const DataBlock::IteratorCore<T> DataBlock::IteratorCore<T>::operator--(int){
    IteratorCore ret = *this;
    ++(*this);
    return ret;
}

template<typename T>
inline size_t DataBlock::IteratorCore<T>::getOffset() const{
    return position;
}

template<typename T>
inline T DataBlock::IteratorCore<T>::getStream() const{
    return stream;
}

template<typename T>
inline DataBlock::ProxyCore<T> DataBlock::IteratorCore<T>::operator*() const{
    return {stream, position};
}

template<typename T>
inline bool DataBlock::IteratorCore<T>::operator==(const IteratorCore& c) const{
    return this->stream == c.stream && this->position == c.position;
}

template<typename T>
inline bool DataBlock::IteratorCore<T>::operator!=(const IteratorCore& c) const{
    return !(*this == c);
}

template<typename IT1, typename IT2>
void DataBlock::insert(const IT1& pos, const IT2& start, const IT2& end){
    if (pos.getStream() != this || start.getStream() != end.getStream()) {
        return;
    }
    data.insert(
            data.begin() + pos.getOffset(),
            start.getStream()->data.begin() + start.getOffset(),
            end.getStream()->data.begin() + end.getOffset());
}

template<typename T>
DataBlock::DataBlock(std::vector<T> *vec) : wordSize(sizeof(T)){
    size_t size = vec->size() * sizeof(T);
    this->data.resize(size);
    this->data.shrink_to_fit();
    std::memcpy(this->data.data(), vec->data(), size);
    vec->clear();
}

}  // namespace gabac


#endif  // GABAC_DATA_BLOCK_H_
