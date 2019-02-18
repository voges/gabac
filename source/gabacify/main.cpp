#include <cassert>
#include <csignal>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "gabacify/decode.h"
#include "gabacify/encode.h"
#include "gabacify/exceptions.h"
#include "gabacify/log.h"
#include "gabacify/program_options.h"
#include "gabacify/tmp_file.h"


static void writeCommandLine(
        int argc,
        char *argv[]
){
    std::vector<std::string> args(argv, (argv + argc));
    std::stringstream commandLine;
    for (const auto& arg : args)
    {
        commandLine << arg << " ";
    }
    GABACIFY_LOG_DEBUG << "Command line: " << commandLine.str();
}


static int gabacify_main(
        int argc,
        char *argv[]
){
    try
    {
        gabacify::ProgramOptions programOptions(argc, argv);
        // TODO(Jan): enable log level setting again
        // gabacify::setLogLevel(programOptions.logLevel);
        writeCommandLine(argc, argv);

        if (programOptions.task == "encode")
        {
            gabacify::encode(
                    programOptions.inputFilePath,
                    programOptions.analyze,
                    programOptions.configurationFilePath,
                    programOptions.outputFilePath
            );
        }
        else if (programOptions.task == "decode")
        {
            gabacify::decode(
                    programOptions.inputFilePath,
                    programOptions.configurationFilePath,
                    programOptions.outputFilePath
            );
        }
        else
        {
            GABACIFY_DIE("Invalid task: " + std::string(programOptions.task));
        }
    }
    catch (const gabacify::RuntimeException& e)
    {
        GABACIFY_LOG_ERROR << "Runtime error: " << e.message();
        gabacify::TmpFile::closeAll();
        return -1;
    }
    catch (const std::exception& e)
    {
        GABACIFY_LOG_ERROR << "Standard library error: " << e.what();
        gabacify::TmpFile::closeAll();
        return -1;
    }
    catch (...)
    {
        GABACIFY_LOG_ERROR << "Unkown error occurred";
        gabacify::TmpFile::closeAll();
        return -1;
    }

    gabacify::TmpFile::closeAll();
    return 0;
}


extern "C" void handleSignal(
        int sig
){
    std::signal(sig, SIG_IGN);  // Ignore the signal
    GABACIFY_LOG_WARNING << "Caught signal: " << sig;
    switch (sig)
    {
        case SIGINT:
        case SIGILL:
        case SIGABRT:
        case SIGFPE:
        case SIGSEGV:
        case SIGTERM:
        case SIGHUP:
        case SIGQUIT:
        case SIGTRAP:
        case SIGKILL:
        case SIGBUS:
        case SIGSYS:
        case SIGPIPE:
            GABACIFY_LOG_WARNING << "Process killed";
            gabacify::TmpFile::closeAll();
    }
    std::signal(sig, SIG_DFL);  // Invoke the default signal action
    std::raise(sig);
}


int main(
        int argc,
        char *argv[]
){
    // Install signal handler for the following signal types:
    //   SIGTERM  termination request, sent to the program
    //   SIGSEGV  invalid memory access (segmentation fault)
    //   SIGINT   external interrupt, usually initiated by the user
    //   SIGILL   invalid program image, such as invalid instruction
    //   SIGABRT  abnormal termination condition, as is e.g. initiated by
    //            std::abort()
    //   SIGFPE   erroneous arithmetic operation such as divide by zero
    std::signal(SIGABRT, handleSignal);
    std::signal(SIGFPE, handleSignal);
    std::signal(SIGILL, handleSignal);
    std::signal(SIGINT, handleSignal);
    std::signal(SIGSEGV, handleSignal);
    std::signal(SIGTERM, handleSignal);

    // Fire up main method
    int rc = gabacify_main(argc, argv);
    if (rc != 0)
    {
        GABACIFY_LOG_FATAL << "Failed to run";
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
