#ifndef GABACIFY_LOG_H_
#define GABACIFY_LOG_H_


#include <iostream>
#include <string>


struct GabacifyLogTmpStdout {
    ~GabacifyLogTmpStdout() { std::cout << std::endl; }
};


struct GabacifyLogTmpStderr {
    ~GabacifyLogTmpStderr() { std::cout << std::endl; }
};


#define GABACIFY_LOG_TRACE (GabacifyLogTmpStdout(), std::cout << "[" << gabacify::currentDateAndTime() << "] [trace] ")

#define GABACIFY_LOG_DEBUG (GabacifyLogTmpStdout(), std::cout << "[" << gabacify::currentDateAndTime() << "] [debug] ")

#define GABACIFY_LOG_INFO (GabacifyLogTmpStdout(), std::cout << "[" << gabacify::currentDateAndTime() << "] [info] ")

#define GABACIFY_LOG_WARNING (GabacifyLogTmpStderr(), std::cerr << "[" << gabacify::currentDateAndTime() << "] [warning] ")

#define GABACIFY_LOG_ERROR (GabacifyLogTmpStderr(), std::cerr << "[" << gabacify::currentDateAndTime() << "] [error] ")

#define GABACIFY_LOG_FATAL (GabacifyLogTmpStderr(), std::cerr << "[" << gabacify::currentDateAndTime() << "] [fatal] ")


namespace gabacify {


std::string currentDateAndTime();


}  // namespace gabacify


#endif  // GABACIFY_LOG_H_
