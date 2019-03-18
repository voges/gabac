#include <cassert>
#include <csignal>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "gabac/equality_coding.h"
#include "gabac/match_coding.h"
#include "gabac/rle_coding.h"
#include "transformify/exceptions.h"
#include "transformify/helpers.h"
#include "transformify/input_file.h"
#include "transformify/log.h"
#include "transformify/output_file.h"
#include "transformify/program_options.h"


static void writeCommandLine(int argc, char* argv[])
{
    std::vector<std::string> args(argv, (argv + argc));
    std::stringstream commandLine;
    for (const auto& arg : args)
    {
        commandLine << arg << " ";
    }
    TRANSFORMIFY_LOG_DEBUG << "Command line: " << commandLine.str();
}


static int transformify_main(int argc, char* argv[])
{
    try
    {
        transformify::ProgramOptions programOptions(argc, argv);
        writeCommandLine(argc, argv);

        // Read in the entire input file.
        transformify::InputFile inputFile(programOptions.inputFilePath);
        std::vector<unsigned char> inputBuffer(inputFile.size());
        inputFile.read(&inputBuffer[0], 1, inputBuffer.size());
        TRANSFORMIFY_LOG_INFO << "Read " << inputBuffer.size() << " byte(s) from '" << programOptions.inputFilePath << "'";

        // Generate symbol stream from byte buffer.
        std::vector<uint64_t> symbols;
        transformify::generateSymbolStream(inputBuffer, programOptions.inputFileWordSize, &symbols);
        inputBuffer.clear();
        inputBuffer.shrink_to_fit();
        TRANSFORMIFY_LOG_INFO << "Generated symbol stream containing " << symbols.size() << " symbol(s) using " << programOptions.inputFileWordSize << " byte(s) per symbol";

        if (programOptions.transformation == "equality")
        {
            TRANSFORMIFY_LOG_INFO << "Performing equality coding";
            std::vector<uint64_t> equalityFlags;
            std::vector<uint64_t> values;
            gabac::transformEqualityCoding(symbols, &equalityFlags, &values);
            symbols.clear();
            symbols.shrink_to_fit();
            TRANSFORMIFY_LOG_INFO << "Number of equality flags: " << equalityFlags.size();
            TRANSFORMIFY_LOG_INFO << "Number of values: " << values.size();

            // Generate output byte buffers.
            std::vector<unsigned char> equalityFlagsOutputBuffer;
            std::vector<unsigned char> valuesOutputBuffer;
            transformify::generateByteBuffer(equalityFlags, 4, &equalityFlagsOutputBuffer);
            transformify::generateByteBuffer(values, 4, &valuesOutputBuffer);
            TRANSFORMIFY_LOG_INFO << "Generated output buffers using 4 bytes per symbol";

            // Write the equality flags bytestream
            std::string equalityFlagsOutputFilePath = programOptions.inputFilePath + ".equality_coding.equality_flags";
            transformify::OutputFile equalityFlagsOutputFile(equalityFlagsOutputFilePath);
            equalityFlagsOutputFile.write(&equalityFlagsOutputBuffer[0], 1, equalityFlagsOutputBuffer.size());
            TRANSFORMIFY_LOG_INFO << "Wrote equality flags bytestream of size " << equalityFlagsOutputBuffer.size() << " to: " << equalityFlagsOutputFilePath;
            equalityFlagsOutputBuffer.clear();
            equalityFlagsOutputBuffer.shrink_to_fit();

            // Write the values bytestream
            std::string valuesOutputFilePath = programOptions.inputFilePath + ".equality_coding.values";
            transformify::OutputFile valuesOutputFile(valuesOutputFilePath);
            valuesOutputFile.write(&valuesOutputBuffer[0], 1, valuesOutputBuffer.size());
            TRANSFORMIFY_LOG_INFO << "Wrote values bytestream of size " << valuesOutputBuffer.size() << " to: " << valuesOutputFilePath;
            valuesOutputBuffer.clear();
            valuesOutputBuffer.shrink_to_fit();
        }
        else if (programOptions.transformation == "match")
        {
            uint32_t windowSize = programOptions.transformationParameter;
            TRANSFORMIFY_LOG_INFO << "Performing match coding using window size " << windowSize;
            std::vector<uint64_t> pointers;
            std::vector<uint64_t> lengths;
            std::vector<uint64_t> rawValues;
            gabac::transformMatchCoding(symbols, windowSize, &pointers, &lengths, &rawValues);
            symbols.clear();
            symbols.shrink_to_fit();
            TRANSFORMIFY_LOG_INFO << "Number of pointers: " << pointers.size();
            TRANSFORMIFY_LOG_INFO << "Number of lengths: " << lengths.size();
            TRANSFORMIFY_LOG_INFO << "Number of raw values: " << rawValues.size();

            // Generate output byte buffers.
            std::vector<unsigned char> pointersOutputBuffer;
            std::vector<unsigned char> lengthsOutputBuffer;
            std::vector<unsigned char> rawValuesOutputBuffer;
            transformify::generateByteBuffer(pointers, 4, &pointersOutputBuffer);
            transformify::generateByteBuffer(lengths, 4, &lengthsOutputBuffer);
            transformify::generateByteBuffer(rawValues, 4, &rawValuesOutputBuffer);
            TRANSFORMIFY_LOG_INFO << "Generated output buffers using 4 bytes per symbol";

            // Write the pointers bytestream
            std::string pointersOutputFilePath = programOptions.inputFilePath + ".match_coding.pointers";
            transformify::OutputFile pointersOutputFile(pointersOutputFilePath);
            pointersOutputFile.write(&pointersOutputBuffer[0], 1, pointersOutputBuffer.size());
            TRANSFORMIFY_LOG_INFO << "Wrote pointers bytestream of size " << pointersOutputBuffer.size() << " to: " << pointersOutputFilePath;
            pointersOutputBuffer.clear();
            pointersOutputBuffer.shrink_to_fit();

            // Write the lengths bytestream
            std::string lengthsOutputFilePath = programOptions.inputFilePath + ".match_coding.lengths";
            transformify::OutputFile lengthsOutputFile(lengthsOutputFilePath);
            lengthsOutputFile.write(&lengthsOutputBuffer[0], 1, lengthsOutputBuffer.size());
            TRANSFORMIFY_LOG_INFO << "Wrote lengths bytestream of size " << lengthsOutputBuffer.size() << " to: " << lengthsOutputFilePath;
            lengthsOutputBuffer.clear();
            lengthsOutputBuffer.shrink_to_fit();

            // Write the raw values bytestream
            std::string rawValuesOutputFilePath = programOptions.inputFilePath + ".match_coding.raw_values";
            transformify::OutputFile rawValuesOutputFile(rawValuesOutputFilePath);
            rawValuesOutputFile.write(&rawValuesOutputBuffer[0], 1, rawValuesOutputBuffer.size());
            TRANSFORMIFY_LOG_INFO << "Wrote raw values bytestream of size " << rawValuesOutputBuffer.size() << " to: " << rawValuesOutputFilePath;
            rawValuesOutputBuffer.clear();
            rawValuesOutputBuffer.shrink_to_fit();
        }
        else if (programOptions.transformation == "rle")
        {
            uint64_t guard = programOptions.transformationParameter;
            TRANSFORMIFY_LOG_INFO << "Performing rle coding using guard " << guard;
            std::vector<uint64_t> rawValues;
            std::vector<uint64_t> lengths;
            gabac::transformRleCoding(symbols, guard, &rawValues, &lengths);
            symbols.clear();
            symbols.shrink_to_fit();
            TRANSFORMIFY_LOG_INFO << "Number of raw values: " << rawValues.size();
            TRANSFORMIFY_LOG_INFO << "Number of lengths: " << lengths.size();

            // Generate output byte buffers.
            std::vector<unsigned char> rawValuesOutputBuffer;
            std::vector<unsigned char> lengthsOutputBuffer;
            transformify::generateByteBuffer(rawValues, 4, &rawValuesOutputBuffer);
            transformify::generateByteBuffer(lengths, 4, &lengthsOutputBuffer);
            TRANSFORMIFY_LOG_INFO << "Generated output buffers using 4 bytes per symbol";

            // Write the raw values bytestream
            std::string rawValuesOutputFilePath = programOptions.inputFilePath + ".rle_coding.raw_values";
            transformify::OutputFile rawValuesOutputFile(rawValuesOutputFilePath);
            rawValuesOutputFile.write(&rawValuesOutputBuffer[0], 1, rawValuesOutputBuffer.size());
            TRANSFORMIFY_LOG_INFO << "Wrote raw values bytestream of size " << rawValuesOutputBuffer.size() << " to: " << rawValuesOutputFilePath;
            rawValuesOutputBuffer.clear();
            rawValuesOutputBuffer.shrink_to_fit();

            // Write the lengths bytestream
            std::string lengthsOutputFilePath = programOptions.inputFilePath + ".rle_coding.raw_values";
            transformify::OutputFile lengthsOutputFile(lengthsOutputFilePath);
            lengthsOutputFile.write(&lengthsOutputBuffer[0], 1, lengthsOutputBuffer.size());
            TRANSFORMIFY_LOG_INFO << "Wrote lengths bytestream of size " << lengthsOutputBuffer.size() << " to: " << lengthsOutputFilePath;
            lengthsOutputBuffer.clear();
            lengthsOutputBuffer.shrink_to_fit();
        }
        else
        {
            TRANSFORMIFY_DIE("Invalid transformation: " + std::string(programOptions.transformation));
        }
    }
    catch (const transformify::RuntimeException& e)
    {
        TRANSFORMIFY_LOG_ERROR << "Runtime error: " << e.message();
        return -1;
    }
    catch (const std::exception& e)
    {
        TRANSFORMIFY_LOG_ERROR << "Standard library error: " << e.what();
        return -1;
    }
    catch (...)
    {
        TRANSFORMIFY_LOG_ERROR << "Unkown error occurred";
        return -1;
    }

    return 0;
}


extern "C" void handleSignal(int sig)
{
    std::signal(sig, SIG_IGN);  // Ignore the signal
    TRANSFORMIFY_LOG_WARNING << "Caught signal: " << sig;
    std::signal(sig, SIG_DFL);  // Invoke the default signal action
    std::raise(sig);
}


int main(int argc, char* argv[])
{
    // Install signal handler for the following signal types:
    //   SIGABRT  abnormal termination condition, as is e.g. initiated by
    //            std::abort()
    //   SIGFPE   erroneous arithmetic operation such as divide by zero
    //   SIGILL   invalid program image, such as invalid instruction
    //   SIGINT   external interrupt, usually initiated by the user
    //   SIGSEGV  invalid memory access (segmentation fault)
    //   SIGTERM  termination request, sent to the program
    std::signal(SIGABRT, handleSignal);
    std::signal(SIGFPE, handleSignal);
    std::signal(SIGILL, handleSignal);
    std::signal(SIGINT, handleSignal);
    std::signal(SIGSEGV, handleSignal);
    std::signal(SIGTERM, handleSignal);

    // Fire up main method
    int rc = transformify_main(argc, argv);
    if (rc != 0)
    {
        TRANSFORMIFY_LOG_FATAL << "Failed to run";
    }

    // The C standard makes no guarantees as to when output to stdout or
    // stderr (standard error) is actually flushed.
    // If e.g. stdout is directed to a file and an error occurs while flushing
    // the data (after program termination), then the output may be lost.
    // Thus we explicitly flush stdout and stderr. On failure, we notify the
    // operating system by returning with EXIT_FAILURE.
    if (fflush(stdout) == EOF)
    {
        return EXIT_FAILURE;
    }
    if (fflush(stderr) == EOF)
    {
        return EXIT_FAILURE;
    }

    // Return to the operating system
    return (rc == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
