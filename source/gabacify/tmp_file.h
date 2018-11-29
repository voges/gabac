#ifndef GABACIFY_TMP_FILE_H_
#define GABACIFY_TMP_FILE_H_


#include <string>
#include <vector>
#include <iostream>

#include "gabacify/file.h"
#include "gabacify/log.h"
#include "gabacify/output_file.h"

namespace gabacify {

class TmpFile : public OutputFile {
    static std::vector<std::string> fileList;
 public:
    explicit TmpFile(
            const std::string& path
    );

    static void closeAll();
};


}  // namespace gabacify


#endif  // GABACIFY_TMP_FILE_H_
