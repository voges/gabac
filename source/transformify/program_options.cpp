#include "transformify/program_options.h"

#include "transformify/exceptions.h"
#include "transformify/log.h"


namespace transformify {


ProgramOptions::ProgramOptions(int argc, char* argv[])
    : inputFilePath()
    , inputFileWordSize(0)
    , transformation()
    , transformationParameter(0)
{
    processCommandLine(argc, argv);
}


ProgramOptions::~ProgramOptions() = default;


void ProgramOptions::processCommandLine(int argc, char* argv[])
{
    try
    {
        namespace po = boost::program_options;

        // Declare the supported options.
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
                "input_file_word_size,w",
                po::value<size_t>(&(this->inputFileWordSize))->required(),
                "Input file path"
            )
            (
                "transformation,t",
                po::value<std::string>(&(this->transformation))->required(),
                "Transformation (equality', 'match' or 'rle')"
            )
            (
                "transformation_parameter,p",
                po::value<uint64_t>(&(this->transformationParameter)),
                "Transformation parameter (window size for 'match' or guard for 'rle')"
            );

        // Parse the command line.
        po::variables_map optionsMap;
        po::store(po::command_line_parser(argc, argv).options(options).run(), optionsMap);

        // First thing to do is to print the help.
        if (optionsMap.count("help") || optionsMap.count("h"))
        {
            std::stringstream optionsStringStream;
            optionsStringStream << options;
            std::string optionsLine;
            while (std::getline(optionsStringStream, optionsLine))
            {
                TRANSFORMIFY_LOG_INFO << optionsLine;
            }
            exit(0);  // Just get out here, quickly!
        }

        // po::notify() will // throw on erroneous program options, that's why
        // we call it after printing the help.
        po::notify(optionsMap);
    }
    catch (const boost::program_options::error& e)
    {
        TRANSFORMIFY_DIE("Program options error: " + std::string(e.what()));
    }
}


}  // namespace transformify
