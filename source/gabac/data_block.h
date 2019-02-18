#ifndef GABAC_DATASTREAM_HPP
#define GABAC_DATASTREAM_HPP

#include <vector>
#include <cstdint>
#include <stdexcept>
#include <cassert>
#include <cstring>

#include "block_stepper.h"

namespace gabac {

class DataBlock
{
 private:
    uint8_t wordSize;

    std::vector<uint8_t> data;
 public:

    BlockStepper getReader () const;
    bool operator==(const DataBlock& d) const;
    DataBlock& operator=(const std::initializer_list<uint64_t >& il);

    uint64_t get(size_t index) const;

    void set(size_t index, uint64_t val);

    template<typename T>
    class ProxyCore {
     private:
        T stream;
        size_t position;
     public:
        ProxyCore(T str, size_t pos);

        operator uint64_t () const;

        ProxyCore& operator= (uint64_t val);

    };

    using Proxy = ProxyCore<DataBlock*>;
    using ConstProxy = ProxyCore<const DataBlock*>;

    template<typename T>
    class IteratorCore {
     private:
        T stream;
        size_t position;
     public:
        IteratorCore(T str, size_t pos);

        IteratorCore operator+ (size_t offset) const;

        IteratorCore operator- (size_t offset) const;

        IteratorCore& operator++ ();

        IteratorCore& operator-- ();

        const IteratorCore operator++ (int);

        const IteratorCore operator-- (int);
        size_t getOffset() const;
        T getStream() const;
        ProxyCore<T> operator* () const;
        bool operator== (const IteratorCore& c) const;

        bool operator!= (const IteratorCore& c) const;

        class iterator_category : public std::random_access_iterator_tag {};
    };

    using Iterator = IteratorCore<DataBlock*>;
    using ConstIterator = IteratorCore<const DataBlock*>;

    size_t size() const;

    void reserve (size_t size);

    void shrink_to_fit ();
    void clear();

    void resize(size_t size);

    bool empty() const;

    ConstIterator begin () const;

    Iterator begin ();

    ConstIterator end () const;

    Iterator end ();

    void push_back (uint64_t val);

    void emplace_back (uint64_t val);

    const void* getData() const;

    void* getData();

    size_t getWordSize() const;

    void setWordSize(uint8_t size);

    void swap (DataBlock* const d);


    template<typename IT1, typename IT2>
    void insert(const IT1& pos, const IT2& start, const IT2& end);

    DataBlock (size_t size = 0, size_t wsize = 1);

};

inline uint64_t DataBlock::get(size_t index) const{
    switch (wordSize) {
        case 1:
            return *(uint8_t *) (data.data() + index);
        case 2:
            return *(uint16_t *) (data.data() + (index << 1));
        case 4:
            return *(uint32_t *) (data.data() + (index << 2));
        case 8:
            return *(uint64_t *) (data.data() + (index << 3));
        default:
            return 0;
    }
}

inline void DataBlock::set(size_t index, uint64_t val){
    switch (wordSize) {
        case 1:
            *(uint8_t *) (data.data() + index) = val;
            return;
        case 2:
            *(uint16_t *) (data.data() + (index << 1)) = val;
            return;
        case 4:
            *(uint32_t *) (data.data() + (index << 2)) = val;
            return;
        case 8:
            *(uint64_t *) (data.data() + (index << 3)) = val;
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
            *(uint8_t *) (data.end().base() - 1) = val;
            return;
        case 2:
            *(uint16_t *) (data.end().base() - 2) = val;
            return;
        case 4:
            *(uint32_t *) (data.end().base() - 4) = val;
            return;
        case 8:
            *(uint64_t *) (data.end().base() - 8) = val;
            return;
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

inline size_t DataBlock::getWordSize() const{
    return wordSize;
}

inline void DataBlock::setWordSize(uint8_t size){
    wordSize = size;
    if (data.size() % size) {
        throw std::runtime_error("Could not resize");
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

}


#endif //GABAC_DATASTREAM_HPP
