#ifndef GABACIFY_LOG_H_
#define GABACIFY_LOG_H_


#define GABACIFY_LOG_TRACE BOOST_LOG_TRIVIAL(trace)

#define GABACIFY_LOG_DEBUG BOOST_LOG_TRIVIAL(debug)

#define GABACIFY_LOG_INFO BOOST_LOG_TRIVIAL(info)

#define GABACIFY_LOG_WARNING BOOST_LOG_TRIVIAL(warning)

#define GABACIFY_LOG_ERROR BOOST_LOG_TRIVIAL(error)

#define GABACIFY_LOG_FATAL BOOST_LOG_TRIVIAL(fatal)


#include <string>
#include <boost/log/trivial.hpp>


namespace gabacify {


void initLog();


void setLogLevel(
        const std::string& logLevel
);


}  // namespace gabacify


#endif  // GABACIFY_LOG_H_
