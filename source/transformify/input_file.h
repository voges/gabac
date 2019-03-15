#ifndef TRANSFORMIFY_INPUT_FILE_H_
#define TRANSFORMIFY_INPUT_FILE_H_


#include <string>

#include "transformify/file.h"


namespace transformify {


class InputFile : public File
{
 public:
    explicit InputFile(
            const std::string& path
    );

    ~InputFile() override;

    void read(
            void *items,
            size_t itemSize,
            size_t numItems
    );
};


}  // namespace transformify


#endif  // TRANSFORMIFY_INPUT_FILE_H_
