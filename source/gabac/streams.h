#ifndef PROJECT_STREAMS_H
#define PROJECT_STREAMS_H

#include "gabac/data_block.h"

#include <istream>
#include <ostream>

namespace gabac {

class FileBuffer : public std::streambuf
{
 public:
    FileBuffer(FILE *f);
 protected:
    int overflow(int c) override;
    std::streamsize xsputn(const char *s, std::streamsize n) override;
    int sync() override;

    std::streamsize xsgetn(char *s, std::streamsize n) override;
    int underflow() override;
 private:
    FILE *fileptr;
};

class DataBlockBuffer : public std::streambuf
{
 public:
    DataBlockBuffer(DataBlock *d, size_t pos_i = 0);
 protected:
    int overflow(int c) override;
    std::streamsize xsputn(const char *s, std::streamsize n) override;
    std::streamsize xsgetn(char *s, std::streamsize n) override;
    int underflow() override;
    int uflow() override;
    virtual void flush_block(gabac::DataBlock *blk);
 private:
    DataBlock block;
    size_t pos;
};

class IFileStream : public FileBuffer, public std::istream
{
 public:
    IFileStream(FILE *f) : FileBuffer(f), std::istream(this){
    }
};

class OFileStream : public FileBuffer, public std::ostream
{
 public:
    OFileStream(FILE *f) : FileBuffer(f), std::ostream(this){
    }
};

class IBufferStream : public DataBlockBuffer, public std::istream
{
 public:
    IBufferStream(DataBlock *d, size_t pos_i = 0) : DataBlockBuffer(d, pos_i), std::istream(this){
    }
};

class OBufferStream : public DataBlockBuffer, public std::ostream
{
 public:
    OBufferStream(DataBlock *d) : DataBlockBuffer(d, 0), std::ostream(this){
    }

    virtual void flush(gabac::DataBlock *blk){
        flush_block(blk);
    }
};

class NullBuffer : public std::streambuf
{
 public:
    int overflow(int c) override{
        return c;
    }
};

class NullStream : public std::ostream
{
 public:
    NullStream() : std::ostream(&m_sb){
    }

 private:
    NullBuffer m_sb;
};

}

#endif //PROJECT_STREAMS_H
