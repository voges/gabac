#ifndef TRANSFORMIFY_OUTPUT_FILE_H_
#define TRANSFORMIFY_OUTPUT_FILE_H_


#include <string>

#include "transformify/file.h"


namespace transformify {


class OutputFile : public File
{
 public:
    explicit OutputFile(const std::string& path);
    ~OutputFile() override;

    void write(void* items, size_t itemSize, size_t numItems);
};


}  // namespace transformify


#endif  // TRANSFORMIFY_OUTPUT_FILE_H_
