#ifndef GABAC_DATASTREAM_HPP
#define GABAC_DATASTREAM_HPP

#include <vector>
#include <cstdint>
#include <stdexcept>
#include <cassert>

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

    inline void set1(size_t index, uint64_t val) {
        assert(index < data.size() / wordSize);
        void* ptr = &(data[wordSize*index]);
        *(static_cast<uint8_t *>(ptr)) = uint8_t(val);
    }

    inline void set2(size_t index, uint64_t val) {
        assert(index < data.size() / wordSize);
        void* ptr = &(data[wordSize*index]);
        *(static_cast<uint16_t *>(ptr)) = uint16_t(val);
    }

    inline void set4(size_t index, uint64_t val) {
        assert(index < data.size() / wordSize);
        void* ptr = &(data[wordSize*index]);
        *(static_cast<uint32_t *>(ptr)) = uint32_t(val);
    }

    inline void set8(size_t index, uint64_t val) {
        assert(index < data.size() / wordSize);
        void* ptr = &(data[wordSize*index]);
        *(static_cast<uint64_t *>(ptr)) = uint64_t(val);
    }

    inline uint64_t get1(size_t index) const {
        assert(index < data.size() / wordSize);
        const void* ptr = &(data[wordSize*index]);
        return *(static_cast<const uint8_t *>(ptr));
    }

    inline uint64_t get2(size_t index) const {
        assert(index < data.size() / wordSize);
        const void* ptr = &(data[wordSize*index]);
        return *(static_cast<const uint16_t *>(ptr));
    }

    inline uint64_t get4(size_t index) const {
        assert(index < data.size() / wordSize);
        const void* ptr = &(data[wordSize*index]);
        return *(static_cast<const uint32_t *>(ptr));
    }

    inline uint64_t get8(size_t index) const {
        assert(index < data.size() / wordSize);
        const void* ptr = &(data[wordSize*index]);
        return *(static_cast<const uint64_t *>(ptr));
    }

    void (DataStream::*setptr)(size_t index, uint64_t val);

    uint64_t (DataStream::*getptr)(size_t index) const;

    inline uint64_t get(size_t index) const {
        return (this->*getptr)(index);
    }

    inline void set(size_t index, uint64_t val) {
        (this->*setptr)(index, val);
    }

    template<typename T>
    class ProxyCore {
     private:
        T stream;
        size_t position;
     public:
        ProxyCore(T str, size_t pos) : stream(str), position(pos) {
        }

        operator uint64_t () const{
            return stream->get(position);
        }

        ProxyCore& operator= (uint64_t val) {
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

        IteratorCore operator+ (size_t offset) const {
            return IteratorCore(stream, position + offset);
        }

        IteratorCore operator- (size_t offset) const {
            return IteratorCore(stream, position - offset);
        }

        IteratorCore& operator++ () {
            *this = *this + 1;
            return *this;
        }

        IteratorCore& operator-- () {
            *this = *this + 1;
            return *this;
        }

        const IteratorCore operator++ (int) {
            IteratorCore ret = *this;
            ++(*this);
            return ret;
        }

        const IteratorCore operator-- (int) {
            IteratorCore ret = *this;
            ++(*this);
            return ret;
        }

        size_t getOffset() const{
            return position;
        }

        T getStream() const{
            return stream;
        }

        ProxyCore<T> operator* () const{
            return {stream, position};
        }

        bool operator== (const IteratorCore& c) const {
            return this->stream == c.stream && this->position == c.position;
        }

        bool operator!= (const IteratorCore& c) const {
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

    ConstIterator begin () const {
        return {this, 0};
    }

    Iterator begin () {
        return {this, 0};
    }

    ConstIterator end () const {
        return {this, data.size() / wordSize};
    }

    Iterator end () {
        return {this, data.size() / wordSize};
    }

    void push_back (uint64_t val) {
        data.resize(data.size() + wordSize);
        set(data.size() / wordSize - 1, val);
    }

    void emplace_back (uint64_t val) {
        push_back(val);
    }

    const void* getData() const{
        return data.data();
    }

    void* getData() {
        return data.data();
    }

    size_t getWordSize() const {
        return wordSize;
    }

    void swap (DataStream& d) {
        size_t tmp = wordSize;
        wordSize = d.wordSize;
        d.wordSize = tmp;
        data.swap(d.data);
    }


    template<typename IT1, typename IT2>
    void insert(const IT1& pos, const IT2& start, const IT2& end) {
        if(pos.getStream() != this || start.getStream() != end.getStream())
            return;
        data.insert(data.begin() + pos.getOffset(), start.getStream()->data.begin() + start.getOffset(), end.getStream()->data.begin() + end.getOffset());
    }

    DataStream (size_t size = 0, size_t wsize = 1) : wordSize(wsize), data(size * wsize) {
        switch(wordSize) {
            case 1:
                this->setptr = &DataStream::set1;
                this->getptr = &DataStream::get1;
                break;
            case 2:
                this->setptr = &DataStream::set2;
                this->getptr = &DataStream::get2;
                break;
            case 4:
                this->setptr = &DataStream::set4;
                this->getptr = &DataStream::get4;
                break;
            case 8:
                //Fall through
            default:
                this->setptr = &DataStream::set8;
                this->getptr = &DataStream::get8;
        }
    }


};

}


#endif //GABAC_DATASTREAM_HPP
