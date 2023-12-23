#include "ice_logger.h"

ice_logger::LOG_LISTENER_TYPE ice_logger::log_listener = nullptr;

ice_logger::LOG_LISTENER_TYPE ice_logger::log_error_listener = nullptr;

void ice_logger::log(std::string header, std::string text)
{
    std::string r = ("[" + header + "]: " + text);

    if (log_listener) log_listener(r);
}

void ice_logger::log_error(std::string header, std::string text)
{
    std::string r = ("[" + header + "]: " + text);

    if (log_error_listener) log_error_listener(r);
}
