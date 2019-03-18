#ifndef TRANSFORMIFY_PROGRAM_OPTIONS_H_
#define TRANSFORMIFY_PROGRAM_OPTIONS_H_


#include <boost/program_options.hpp>

#include <string>


namespace transformify {


class ProgramOptions
{
 public:
    ProgramOptions(int argc, char* argv[]);
    ~ProgramOptions();

    std::string inputFilePath;
    size_t inputFileWordSize;
    std::string transformation;
    uint64_t transformationParameter;

 private:
    void processCommandLine(int argc, char *argv[]);
};


}  // namespace transformify


#endif  // TRANSFORMIFY_PROGRAM_OPTIONS_H_
