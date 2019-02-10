#include "gabacify/tmp_file.h"

#include <cassert>
#include <string>

#include "gabac/exceptions.h"


namespace gabacify {

std::vector<std::string> TmpFile::fileList;

TmpFile::TmpFile(
        const std::string& path
) : OutputFile(path){
    if (path.size() < 4)
    {
        return;
    }
    if (path.substr(path.size() - 4, 4) == ".tmp")
    {
        //GABACIFY_LOG_TRACE << "Tmp file opened: " << path;
        fileList.emplace_back(path);
    }
}

void TmpFile::closeAll(){
    for (const auto& s : fileList)
    {
        int result = std::remove(s.c_str());
        if (result == 0)
        {
            //GABACIFY_LOG_TRACE << "Tmp file successfully closed: " << s;
        }
        else
        {
            //GABACIFY_LOG_WARNING << "Failed to close tmp file: " << s;
        }
    }
    fileList.clear();
}


}  // namespace gabacify
