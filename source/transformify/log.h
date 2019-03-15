#ifndef TRANSFORMIFY_LOG_H_
#define TRANSFORMIFY_LOG_H_


#include <iostream>
#include <string>


struct GabacifyLogTmpStdout {
    ~GabacifyLogTmpStdout() { std::cout << std::endl; }
};


struct GabacifyLogTmpStderr {
    ~GabacifyLogTmpStderr() { std::cout << std::endl; }
};


#define TRANSFORMIFY_LOG_TRACE (GabacifyLogTmpStdout(), std::cout << "[" << transformify::currentDateAndTime() << "] [trace] ")

#define TRANSFORMIFY_LOG_DEBUG (GabacifyLogTmpStdout(), std::cout << "[" << transformify::currentDateAndTime() << "] [debug] ")

#define TRANSFORMIFY_LOG_INFO (GabacifyLogTmpStdout(), std::cout << "[" << transformify::currentDateAndTime() << "] [info] ")

#define TRANSFORMIFY_LOG_WARNING (GabacifyLogTmpStderr(), std::cerr << "[" << transformify::currentDateAndTime() << "] [warning] ")

#define TRANSFORMIFY_LOG_ERROR (GabacifyLogTmpStderr(), std::cerr << "[" << transformify::currentDateAndTime() << "] [error] ")

#define TRANSFORMIFY_LOG_FATAL (GabacifyLogTmpStderr(), std::cerr << "[" << transformify::currentDateAndTime() << "] [fatal] ")


namespace transformify {


std::string currentDateAndTime();


}  // namespace transformify


#endif  // TRANSFORMIFY_LOG_H_
