#ifndef GABAC_STREAM_HANDLER_H
#define GABAC_STREAM_HANDLER_H

#include <cstddef>
#include <istream>
#include <ostream>

namespace gabac {

class DataBlock;

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

class IFileStream : public FileBuffer, public std::istream
{
 public:
    IFileStream (FILE *f) : FileBuffer(f), std::istream(this) {}
};

class OFileStream : public FileBuffer, public std::ostream
{
 public:
    OFileStream (FILE *f) : FileBuffer(f), std::ostream(this) {}
};

class StreamHandler
{
 public:
    static size_t readStream(std::istream& input, DataBlock *buffer);
    static size_t readBytes(std::istream& input, size_t bytes, DataBlock *buffer);
    static size_t readFull(std::istream& input, DataBlock *buffer);
    static size_t readBlock(std::istream& input, size_t bytes, DataBlock *buffer);
    static size_t writeStream(std::ostream& output, DataBlock *buffer);
    static size_t writeBytes(std::ostream& output, DataBlock *buffer);
};
}

#endif //GABAC_STREAM_HANDLER_H
