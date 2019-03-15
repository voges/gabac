#include "transformify/program_options.h"

#include <cassert>

#include "transformify/exceptions.h"
#include "transformify/helpers.h"
#include "transformify/log.h"


namespace transformify {


const std::string ProgramOptions::m_defaultEncodedFilePathExtension = ".transformify_encoded";
const std::string ProgramOptions::m_defaultDecodedFilePathExtension = ".transformify_decoded";


ProgramOptions::ProgramOptions(int argc, char* argv[])
        : inputFilePath()
        , outputFilePath()
        , task()
{
    processCommandLine(argc, argv);
}


ProgramOptions::~ProgramOptions() = default;


void ProgramOptions::processCommandLine(int argc, char* argv[])
{
    try
    {
        namespace po = boost::program_options;

        // Declare the supported options
        po::options_description options("Options");
        options.add_options()
            (
                "help,h",
                "Help"
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
            )
            (
                "transformation,t",
                po::value<std::string>(&(this->transformation))->required(),
                "Transformation ('diff', 'equality', 'match' or 'rle')"
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
                TRANSFORMIFY_LOG_INFO << optionsLine;
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
        TRANSFORMIFY_DIE("Program options error: " + std::string(e.what()));
    }
}


void ProgramOptions::validate()
{
    // Do stuff depending on the task
    if (this->task == "encode")
    {
        // We need an output file path - generate one if not provided by the
        // user
        if (this->outputFilePath.empty())
        {
            TRANSFORMIFY_LOG_INFO << "No output file path provided";
            this->outputFilePath = this->inputFilePath + m_defaultEncodedFilePathExtension;
            TRANSFORMIFY_LOG_INFO << "Using generated output file path: " << this->outputFilePath;
        }
        if (fileExists(this->outputFilePath)) {
            TRANSFORMIFY_DIE("Output file already existing: " + this->outputFilePath);
        }
    }
    else if (this->task == "decode")
    {
        // We need an output file path - generate one if not provided by the
        // user
        if (this->outputFilePath.empty())
        {
            TRANSFORMIFY_LOG_INFO << "No output file path provided";
            this->outputFilePath = this->inputFilePath;
            size_t pos = this->outputFilePath.find(m_defaultEncodedFilePathExtension);
            this->outputFilePath.erase(pos, std::string::npos);
            this->outputFilePath += m_defaultDecodedFilePathExtension;
            TRANSFORMIFY_LOG_INFO << "Using generated output file path: " << this->outputFilePath;
        }
        if (fileExists(this->outputFilePath)) {
            TRANSFORMIFY_DIE("Output file already existing: " + this->outputFilePath);
        }
    }
    else
    {
        TRANSFORMIFY_DIE("Task '" + this->task + "' is invalid");
    }
}


}  // namespace transformify
