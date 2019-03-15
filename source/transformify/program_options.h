#ifndef TRANSFORMIFY_PROGRAM_OPTIONS_H_
#define TRANSFORMIFY_PROGRAM_OPTIONS_H_


#include <boost/program_options.hpp>

#include <string>
#include <vector>


namespace transformify {


class ProgramOptions
{
 public:
    ProgramOptions(int argc, char* argv[]);
    ~ProgramOptions();

 public:
    std::string inputFilePath;
    std::string outputFilePath;
    std::string task;
    std::string transformation;

 private:
    void processCommandLine(int argc, char *argv[]);
    void validate();

 private:
    static const std::string m_defaultEncodedFilePathExtension;
    static const std::string m_defaultDecodedFilePathExtension;
};


}  // namespace transformify


#endif  // TRANSFORMIFY_PROGRAM_OPTIONS_H_
