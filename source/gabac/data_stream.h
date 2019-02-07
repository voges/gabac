#ifndef GABAC_DATASTREAM_HPP
#define GABAC_DATASTREAM_HPP

#include <vector>
#include <cstdint>
#include <stdexcept>
#include <cassert>
#include <cstring>

namespace gabac {

class DataStream
{
 private:
    uint8_t wordSize;
    std::vector<uint8_t> data;
 public:
    bool operator==(const DataStream& d) const{
        return wordSize == d.wordSize && data == d.data;
    }
    DataStream& operator=(const std::initializer_list<uint64_t >& il){
        resize(il.size());
        size_t ctr = 0;
        for(const auto& v : il) {
            set(ctr, v);
            ++ctr;
        }
        return *this;
    }

    inline uint64_t get(size_t index) const {
        // return (this->*getptr)(index);
        uint64_t ret = 0;
        memcpy(&ret, data.data()+index*wordSize, wordSize);
        return ret;
    }

    inline void set(size_t index, uint64_t val) {
        //  (this->*setptr)(index, val);
        memcpy(data.data()+index*wordSize, &val, wordSize);
    }

    template<typename T>
    class ProxyCore {
     private:
        T stream;
        size_t position;
     public:
        inline ProxyCore(T str, size_t pos) : stream(str), position(pos) {
        }

        inline operator uint64_t () const{
            return stream->get(position);
        }

        inline ProxyCore& operator= (uint64_t val) {
            stream->set(position, val);
            return *this;
        }

    };

    using Proxy = ProxyCore<DataStream*>;
    using ConstProxy = ProxyCore<const DataStream*>;

    template<typename T>
    class IteratorCore {
     private:
        T stream;
        size_t position;
     public:
        IteratorCore(T str, size_t pos) : stream(str), position(pos) {
        }

        inline IteratorCore operator+ (size_t offset) const {
            return IteratorCore(stream, position + offset);
        }

        inline IteratorCore operator- (size_t offset) const {
            return IteratorCore(stream, position - offset);
        }

        inline IteratorCore& operator++ () {
            *this = *this + 1;
            return *this;
        }

        inline IteratorCore& operator-- () {
            *this = *this + 1;
            return *this;
        }

        inline const IteratorCore operator++ (int) {
            IteratorCore ret = *this;
            ++(*this);
            return ret;
        }

        inline const IteratorCore operator-- (int) {
            IteratorCore ret = *this;
            ++(*this);
            return ret;
        }

        inline size_t getOffset() const{
            return position;
        }

        inline T getStream() const{
            return stream;
        }

        inline ProxyCore<T> operator* () const{
            return {stream, position};
        }

        inline bool operator== (const IteratorCore& c) const {
            return this->stream == c.stream && this->position == c.position;
        }

        inline bool operator!= (const IteratorCore& c) const {
            return !(*this == c);
        }
    };

    using Iterator = IteratorCore<DataStream*>;
    using ConstIterator = IteratorCore<const DataStream*>;


/*    Proxy operator[] (size_t index) {
        return {this, index};
    }

    ConstProxy operator[] (size_t index) const {
        return {this, index};
    }*/

  /*  Proxy at (size_t index) {
        //TODO: check
        return {this, index};
    }

    ConstProxy at (size_t index) const {
        //TODO: check
        return {this, index};
    } */

 /*   Proxy front () {
        return {this, 0};
    }

    ConstProxy front () const{
        return {this, 0};
    }*/

    /*Proxy back () {
        return {this, data.size() / wordSize - 1};
    }

    ConstProxy back () const{
        return {this, data.size() / wordSize - 1};
    }*/

    size_t size() const {
        return data.size() / wordSize;
    }

    void reserve (size_t size) {
        data.reserve(size * wordSize);
    }

    void shrink_to_fit () {
        data.shrink_to_fit();
    }

    void clear() {
        data.clear();
    }

    void resize(size_t size) {
        data.resize(size * wordSize);
    }

    bool empty() const {
        return data.empty();
    }

    inline ConstIterator begin () const {
        return {this, 0};
    }

    inline Iterator begin () {
        return {this, 0};
    }

    inline ConstIterator end () const {
        return {this, data.size() / wordSize};
    }

    inline Iterator end () {
        return {this, data.size() / wordSize};
    }

    inline void push_back (uint64_t val) {
        data.resize(data.size() + wordSize);
        set(data.size() / wordSize - 1, val);
    }

    inline void emplace_back (uint64_t val) {
        push_back(val);
    }

    inline const void* getData() const{
        return data.data();
    }

    inline void* getData() {
        return data.data();
    }

    inline size_t getWordSize() const {
        return wordSize;
    }

    inline void setWordSize(uint8_t size) {
        wordSize = size;
        if(data.size() % size)
            throw std::runtime_error("Could not resize");
    }

    void swap (DataStream* const d) {
        size_t tmp = wordSize;
        wordSize = d->wordSize;
        d->wordSize = tmp;
        data.swap(d->data);
    }


    template<typename IT1, typename IT2>
    void insert(const IT1& pos, const IT2& start, const IT2& end) {
        if(pos.getStream() != this || start.getStream() != end.getStream())
            return;
        data.insert(data.begin() + pos.getOffset(), start.getStream()->data.begin() + start.getOffset(), end.getStream()->data.begin() + end.getOffset());
    }

    DataStream (size_t size = 0, size_t wsize = 1) : wordSize(wsize), data(size * wsize) {
    }

};

}


#endif //GABAC_DATASTREAM_HPP
