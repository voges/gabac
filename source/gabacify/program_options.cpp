#include "gabacify/program_options.h"

#include <cassert>

#include "gabacify/exceptions.h"
#include "gabacify/helpers.h"
#include "gabacify/log.h"


namespace gabacify {


const std::string ProgramOptions::m_defaultBytestreamFilePathExtension = ".gabac_bytestream";
const std::string ProgramOptions::m_defaultConfigurationFilePathExtension = ".gabac_configuration.json";
const std::string ProgramOptions::m_defaultUncompressedFilePathExtension = ".gabac_uncompressed";


ProgramOptions::ProgramOptions(
        int argc,
        char *argv[]
)
        : analyze(false),
        configurationFilePath(),
        logLevel(),
        inputFilePath(),
        outputFilePath(),
        task()
{
    processCommandLine(argc, argv);
}


ProgramOptions::~ProgramOptions() = default;


void ProgramOptions::processCommandLine(
        int argc,
        char *argv[]
){
    try
    {
        namespace po = boost::program_options;

        // Declare the supported options
        po::options_description options("Options");
        options.add_options()
            (
                "configuration_file_path,c",
                po::value<std::string>(&(this->configurationFilePath)),
                "Configuration file path"
            )
            (
                "help,h",
                "Help"
            )
            (
                "log_level,l",
                po::value<std::string>(&(this->logLevel))->default_value("info"),
                "Log level: 'trace', 'info' (default), 'debug', 'warning', 'error', or 'fatal'"
            )
            (
                "input_file_path,i",
                po::value<std::string>(&(this->inputFilePath))->required(),
                "Input file path"
            )
            (
                "output_file_path,o",
                po::value<std::string>(&(this->outputFilePath)),
                "Output file path"
            )
            (
                "task",
                po::value<std::string>(&(this->task))->required(),
                "Task ('encode' or 'decode')"
            );

        // Declare 'task' as positional
        po::positional_options_description positional;
        positional.add("task", -1);

        // Parse the command line
        po::variables_map optionsMap;
        po::store(po::command_line_parser(argc, argv).options(options).positional(positional).run(), optionsMap);

        // First thing to do is to print the help
        if (optionsMap.count("help") || optionsMap.count("h"))
        {
            std::stringstream optionsStringStream;
            optionsStringStream << options;
            std::string optionsLine;
            while (std::getline(optionsStringStream, optionsLine))
            {
                GABACIFY_LOG_INFO << optionsLine;
            }
            exit(0);  // Just get out here, quickly
        }

        // po::notify() will // throw on erroneous program options, that's why we
        // call it after printing the help
        po::notify(optionsMap);

        // Validate the parsed options
        validate();
    }
    catch (const boost::program_options::error& e)
    {
        GABACIFY_DIE("Program options error: " + std::string(e.what()));
    }
}


void ProgramOptions::validate(void)
{
    // Do stuff depending on the task
    if (this->task == "encode")
    {
        // It's fine not to provide a configuration file path for encoding.
        // This will trigger the analysis.
        if (this->configurationFilePath.empty())
        {
            GABACIFY_LOG_INFO << "No configuration file path provided";
            this->analyze = true;
            GABACIFY_LOG_INFO << "This triggers the analysis before encoding";
            this->configurationFilePath = this->inputFilePath + m_defaultConfigurationFilePathExtension;
            GABACIFY_LOG_INFO << "Using generated configuration file path: " << this->configurationFilePath;
        }

        // We need an output file path - generate one if not provided by the
        // user
        if (this->outputFilePath.empty())
        {
            GABACIFY_LOG_INFO << "No output file path provided";
            this->outputFilePath = this->inputFilePath + m_defaultBytestreamFilePathExtension;
            GABACIFY_LOG_INFO << "Using generated output file path: " << this->outputFilePath;
        }

        if(fileExists(this->outputFilePath)) {
            GABACIFY_DIE("Output file already existing: " + this->outputFilePath);
        }
    }
    else if (this->task == "decode")
    {
        // We need a configuration file path - guess one if not provided by
        // the user
        if (this->configurationFilePath.empty())
        {
            GABACIFY_LOG_INFO << "No configuration file path provided";
            this->configurationFilePath = this->inputFilePath;
            size_t pos = this->configurationFilePath.find(m_defaultBytestreamFilePathExtension);
            this->configurationFilePath.erase(pos, std::string::npos);
            this->configurationFilePath += m_defaultConfigurationFilePathExtension;
            GABACIFY_LOG_INFO << "Trying generated configuration file path: " << this->configurationFilePath;
        }

        // We need an output file path - generate one if not provided by the
        // user
        if (this->outputFilePath.empty())
        {
            GABACIFY_LOG_INFO << "No output file path provided";
            this->outputFilePath = this->inputFilePath;
            size_t pos = this->outputFilePath.find(m_defaultBytestreamFilePathExtension);
            this->outputFilePath.erase(pos, std::string::npos);
            this->outputFilePath += m_defaultUncompressedFilePathExtension;
            GABACIFY_LOG_INFO << "Using generated output file path: " << this->outputFilePath;
        }

        if(fileExists(this->outputFilePath)) {
            GABACIFY_DIE("Output file already existing: " + this->outputFilePath);
        }
    }
    else
    {
        GABACIFY_DIE("Task '" + this->task + "' is invalid");
    }
}


}  // namespace gabacify
