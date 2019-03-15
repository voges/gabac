#ifndef TRANSFORMIFY_FILE_H_
#define TRANSFORMIFY_FILE_H_


#include <string>
#include <cstddef>

using std::size_t;

namespace transformify {


class File
{
 public:
    File(
            const std::string& path,
            const char *mode
    );

    virtual ~File();

    void advance(
            off_t offset
    );

    bool eof() const;

    FILE *handle() const;

    void seekFromCur(
            off_t offset
    );

    void seekFromEnd(
            off_t offset
    );

    void seekFromSet(
            off_t offset
    );

    size_t size();

    off_t tell() const;

 protected:
    FILE *m_fp;
    std::string m_path;

    void close();

    void open(
            const std::string& path,
            const char *mode
    );

    void seek(
            off_t offset,
            int whence
    );
};


}  // namespace transformify


#endif  // TRANSFORMIFY_FILE_H_
