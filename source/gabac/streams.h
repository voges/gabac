#ifndef GABAC_STREAMS_H_
#define GABAC_STREAMS_H_

#include <istream>
#include <ostream>

#include "gabac/data_block.h"

namespace gabac {

class FileBuffer : public std::streambuf
{
 public:
    explicit FileBuffer(FILE *f);
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
    explicit DataBlockBuffer(DataBlock *d, size_t pos_i = 0);
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
    explicit IFileStream(FILE *f);
};

class OFileStream : public FileBuffer, public std::ostream
{
 public:
    explicit OFileStream(FILE *f);
};

class IBufferStream : public DataBlockBuffer, public std::istream
{
 public:
    explicit IBufferStream(DataBlock *d, size_t pos_i = 0);
};

class OBufferStream : public DataBlockBuffer, public std::ostream
{
 public:
    explicit OBufferStream(DataBlock *d);

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
    NullStream();

 private:
    NullBuffer m_sb;
};

}  // namespace gabac

#endif  // GABAC_STREAMS_H_
