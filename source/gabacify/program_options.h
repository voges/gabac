#ifndef GABACIFY_PROGRAM_OPTIONS_H_
#define GABACIFY_PROGRAM_OPTIONS_H_


#include <boost/program_options.hpp>

#include <string>
#include <vector>


namespace gabacify {


class ProgramOptions
{
 public:
    ProgramOptions(
            int argc,
            char *argv[]
    );

    ~ProgramOptions();

 public:
    bool analyze;
    std::string configurationFilePath;
    std::string logLevel;
    std::string inputFilePath;
    std::string outputFilePath;
    std::string task;

 private:
    void processCommandLine(
            int argc,
            char *argv[]
    );

    void validate();

    static const std::string m_defaultBytestreamFilePathExtension;
    static const std::string m_defaultConfigurationFilePathExtension;
    static const std::string m_defaultUncompressedFilePathExtension;
};


}  // namespace gabacify


#endif  // GABACIFY_PROGRAM_OPTIONS_H_
