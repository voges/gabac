#ifndef GABACIFY_LOG_H_
#define GABACIFY_LOG_H_


#include <iostream>
#include <string>


struct GabacifyLogTmp {
    ~GabacifyLogTmp() { std::cout << std::endl; }
};


#define GABACIFY_LOG_TRACE (GabacifyLogTmp(), std::cout << "[" << gabacify::currentDateAndTime() << "] [trace] ")

#define GABACIFY_LOG_DEBUG (GabacifyLogTmp(), std::cout << "[" << gabacify::currentDateAndTime() << "] [debug] ")

#define GABACIFY_LOG_INFO (GabacifyLogTmp(), std::cout << "[" << gabacify::currentDateAndTime() << "] [info] ")

#define GABACIFY_LOG_WARNING (GabacifyLogTmp(), std::cerr << "[" << gabacify::currentDateAndTime() << "] [warning] ")

#define GABACIFY_LOG_ERROR (GabacifyLogTmp(), std::cerr << "[" << gabacify::currentDateAndTime() << "] [error] ")

#define GABACIFY_LOG_FATAL (GabacifyLogTmp(), std::cerr << "[" << gabacify::currentDateAndTime() << "] [fatal] ")


namespace gabacify {


std::string currentDateAndTime();


}  // namespace gabacify


#endif  // GABACIFY_LOG_H_
