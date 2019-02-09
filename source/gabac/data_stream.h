#ifndef GABAC_DATASTREAM_HPP
#define GABAC_DATASTREAM_HPP

#include <vector>
#include <cstdint>
#include <stdexcept>
#include <cassert>
#include <cstring>

namespace gabac {


struct StreamReader {
    uint8_t* curr;
    uint8_t* end;
    uint8_t wordSize;
    inline bool isValid() const{
        return curr != end;
    }
    inline void inc() {
        curr += wordSize;
    }
    inline uint64_t get() const {
        switch(wordSize) {
            case 1:
                return *(uint8_t*) (curr);
            case 2:
                return *(uint16_t*) (curr);
            case 4:
                return *(uint32_t*) (curr);
            case 8:
                return *(uint64_t*) (curr);
        }
        return 0;
    }

    inline void set(uint64_t val) const {
        switch(wordSize) {
            case 1:
                *(uint8_t*) (curr) = val;
                return;
            case 2:
                *(uint16_t*) (curr)= val;
                return;
            case 4:
                *(uint32_t*) (curr)= val;
                return;
            case 8:
                *(uint64_t*) (curr)= val;
                return;
        }
    }
};

class DataStream
{
 private:
    uint8_t wordSize;

    std::vector<uint8_t> data;
 public:

    StreamReader getReader () const {
        return {(uint8_t*)data.data(), (uint8_t*)data.end().base(), wordSize};
    }

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
        switch(wordSize) {
            case 1:
                return *(uint8_t*) (data.data()+index);
            case 2:
                return *(uint16_t*) (data.data()+(index<<1));
            case 4:
                return *(uint32_t*) (data.data()+(index<<2));
            case 8:
                return *(uint64_t*) (data.data()+(index<<3));
            default:
                return 0;
        }
    }

    inline void set(size_t index, uint64_t val) {
        switch(wordSize) {
            case 1:
                *(uint8_t*) (data.data()+index) = val;
                return;
            case 2:
                *(uint16_t*) (data.data()+(index<<1))= val;
                return;
            case 4:
                *(uint32_t*) (data.data()+(index<<2))= val;
                return;
            case 8:
                *(uint64_t*) (data.data()+(index<<3))= val;
                return;
            default:
                return;
        }
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
      /*
        set(data.size() / wordSize - 1, val);*/
        data.resize(data.size() + wordSize);
        switch(wordSize) {
            case 1:
                *(uint8_t*) (data.end().base()-1) = val;
                return;
            case 2:
                *(uint16_t*) (data.end().base()-2) = val;
                return;
            case 4:
                *(uint32_t*) (data.end().base()-4) = val;
                return;
            case 8:
                *(uint64_t*) (data.end().base()-8) = val;
                return;
        }
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

class InputStream {
 public:
    virtual size_t readStream (DataStream* buffer) = 0;
    virtual size_t readBytes(size_t size, DataStream* buffer) = 0;
    virtual size_t readFull(DataStream* buffer) = 0;
    virtual bool isValid() = 0;
    virtual ~InputStream() = default;
    virtual size_t getTotalSize() = 0;
    virtual size_t getRemainingSize() = 0;
};

class BufferInputStream : public InputStream {
 private:
    DataStream mainBuffer;
    StreamReader r;
 public:
    BufferInputStream(DataStream* stream) : mainBuffer(0, 1){
        mainBuffer.swap(stream);
        mainBuffer.setWordSize(1);
        r = mainBuffer.getReader();
    }
    size_t readStream (DataStream* buffer) override{
        uint32_t streamSize = 0;
        auto* streamPtr = (uint8_t*)&streamSize;
        *streamPtr = (uint8_t) r.get();
        r.inc();
        *streamPtr = (uint8_t) r.get();
        r.inc();
        *streamPtr = (uint8_t) r.get();
        r.inc();
        *streamPtr = (uint8_t) r.get();
        r.inc();

        return readBytes(streamSize, buffer);
    }
    size_t readBytes(size_t size, DataStream* buffer) override{
        *buffer = DataStream(0,1);
        buffer->resize(size);
        memcpy(buffer->getData(), this->mainBuffer.getData(), size);
        r.curr += size;
        return size;
    }

    size_t readFull(DataStream* buffer) override {
        mainBuffer.swap(buffer);
        return buffer->size();
    }

    bool isValid() override{
        return r.isValid();
    }

    size_t getTotalSize() override{
        return mainBuffer.size();
    }
    size_t getRemainingSize() override {
        return r.end - r.curr;
    }
};

class FileInputStream  : public InputStream{
    FILE* input;
    size_t fpos;
    size_t fsize;
 public:
    FileInputStream(FILE* file) : input(file) {
        fpos = ftell(input);
        fseek(input, 0, SEEK_END);
        fsize = ftell(input);
        fseek(input, fpos, SEEK_SET);
    }
    size_t readStream (DataStream* buffer) override{
        uint32_t streamSize = 0;
        if(fread(&streamSize, sizeof(uint32_t), 1, input) != 1){
            throw std::runtime_error("Unexpected file end");
        }
        return readBytes(streamSize, buffer);
    }
    size_t readBytes(size_t size, DataStream* buffer) override{
        buffer->resize(size);
        if(fread(buffer->getData(), 1, size, input) != size){
            throw std::runtime_error("Unexpected file end");
        }
        return fpos += size;
    }

    size_t readFull(DataStream* buffer) override {
        return readBytes(getRemainingSize(), buffer);
    }

    bool isValid() override{
        return fpos < fsize;
    }

    size_t getTotalSize() override{
        return fsize;
    }
    size_t getRemainingSize() override {
        return fsize - fpos;
    }
};

class OutputStream {
 public:
    virtual size_t writeStream (DataStream* buffer) = 0;
    virtual size_t writeBytes(DataStream* buffer) = 0;
    virtual size_t bytesWritten() = 0;
    virtual ~OutputStream() = default;
};

class BufferOutputStream : public OutputStream{
    DataStream buffer;
 public:
    BufferOutputStream() : buffer(0,1) {

    }
    size_t writeStream (DataStream* inbuffer) override {
        uint32_t size = inbuffer->size() * inbuffer->getWordSize();
        auto* ptr = (uint8_t*) &size;
        buffer.push_back(*(ptr++));
        buffer.push_back(*(ptr++));
        buffer.push_back(*(ptr++));
        buffer.push_back(*(ptr));

        return writeBytes(inbuffer) + sizeof(uint32_t);
    }
    size_t writeBytes(DataStream* inbuffer) override {
        size_t oldSize = buffer.size();
        size_t newSize = inbuffer->size()* inbuffer->getWordSize();
        buffer.resize(oldSize + newSize);
        memcpy((uint8_t*)buffer.getData() + oldSize, inbuffer->getData(), newSize);
        inbuffer->clear();
        inbuffer->shrink_to_fit();
        return newSize;
    }

    void flush (DataStream* outbuffer) {
        outbuffer->swap(&buffer);
        buffer = DataStream(0,1);
    }

    size_t bytesWritten() override {
        return buffer.size();
    }
};

class FileOutputStream : public OutputStream{
    FILE* file;
 public:
    FileOutputStream(FILE* f) : file(f) {

    }
    size_t writeStream (DataStream* inbuffer) override {
        uint32_t size = inbuffer->size() * inbuffer->getWordSize();
        auto* ptr = (uint8_t*) &size;
        fwrite(ptr,sizeof(uint32_t),1, file);

        return writeBytes(inbuffer) + sizeof(uint32_t);
    }
    size_t writeBytes(DataStream* inbuffer) override {
        size_t ret = fwrite(inbuffer->getData(), 1, inbuffer->size() * inbuffer->getWordSize(), file);
        inbuffer->clear();
        inbuffer->shrink_to_fit();
        return ret;
    }

    size_t bytesWritten() override {
        return ftell(file);
    }
};

}


#endif //GABAC_DATASTREAM_HPP
