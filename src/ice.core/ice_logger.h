#pragma once

#ifndef ICE_LOGGER
#define ICE_LOGGER

#include <vector>
#include <string>
#include <functional>

typedef std::function<void(std::string&)> LOG_LISTENER_TYPE;

class ice_logger final
{

private:

    ice_logger() = delete;

public:

    using LOG_LISTENER_TYPE = std::function<void(std::string&)>;

    static LOG_LISTENER_TYPE log_listener;

    static LOG_LISTENER_TYPE log_error_listener;

    static void log(std::string header, std::string text);

    static void log_error(std::string header, std::string text);
};

#endif