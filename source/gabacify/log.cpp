#include "gabacify/log.h"

#include <cassert>

#include <string>

#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>

#include "gabacify/exceptions.h"


namespace gabacify {


void initLog(){
    // With this call attributes "LineID", "TimeStamp", "ProcessID" and
    // "ThreadID" are registered globally. The "LineID" attribute is a counter
    // that increments for each record being made, the first record gets
    // identifier 1. The "TimeStamp" attribute always yields the current time
    // (i.e. the time when the log record is created, not the time it was
    // written to a sink). The last two attributes identify the process and
    // the thread in which every log record is emitted.
    // Note: In single-threaded builds the "ThreadID" attribute is not
    // registered.
    boost::log::add_common_attributes();

    // LineID format
    // auto formatLineID
    //     = boost::log::expressions::attr
    //       <boost::log::attributes::current_line_id::value_type>(
    //           "LineID"
    //       );

    // TimeStamp format
    auto formatTimeStamp =
        boost::log::expressions::format_date_time<boost::posix_time::ptime>(
            "TimeStamp", "%Y-%m-%d %H:%M:%S"
            // "TimeStamp", "%Y-%m-%d %H:%M:%S.%f"
        );

    // ProcessID format
    // auto formatProcessID =
    //     boost::log::expressions::attr<boost::log::attributes::current_process_id::value_type>("ProcessID");

    // ThreadID format
    // auto formatThreadID =
    //     boost::log::expressions::attr<boost::log::attributes::current_thread_id::value_type>("ThreadID");

    // Severity format
    auto formatSeverity
        = boost::log::expressions::attr<boost::log::trivial::severity_level>("Severity");

    // Log format:
    // [TimeStamp] [Severity Level] Log message
    boost::log::formatter logFormat =
        boost::log::expressions::format("[%1%] [%2%] %3%")
        % formatTimeStamp
        // % formatLineID
        // % formatProcessID
        // % formatThreadID
        % formatSeverity
        % boost::log::expressions::smessage;

    // Console sink
    auto consoleSink = boost::log::add_console_log(std::clog);
    consoleSink->set_formatter(logFormat);
}


void setLogLevel(
        const std::string& logLevel
){
    boost::log::trivial::severity_level severityLevel;

    if (logLevel == "trace")
    {
        severityLevel = boost::log::trivial::trace;
    }
    else if (logLevel == "debug")
    {
        severityLevel = boost::log::trivial::debug;
    }
    else if (logLevel == "info")
    {
        severityLevel = boost::log::trivial::info;
    }
    else if (logLevel == "warning")
    {
        severityLevel = boost::log::trivial::warning;
    }
    else if (logLevel == "error")
    {
        severityLevel = boost::log::trivial::error;
    }
    else if (logLevel == "fatal")
    {
        severityLevel = boost::log::trivial::fatal;
    }
    else
    {
        GABACIFY_DIE("Invalid log level");
    }

    boost::log::core::get()->set_filter(
        boost::log::trivial::severity >= severityLevel
    );
}


}  // namespace gabacify
