#pragma once

#ifndef ICE_A_SERVER
#define ICE_A_SERVER

#include "end_point.h"

class a_sock
{

public:

    struct recv_result
    {
        char* recv_arr = nullptr;

        unsigned short recv_size = 0;

        end_point recv_point = end_point(0, 0);
    };

public:

    enum recv_predicate_code
    {
        accept,
        reject,
        temp,
    };

    typedef recv_predicate_code (*recv_predicate)(char);

public:

    virtual ~a_sock() = default;

public:

    virtual end_point get_local_point() = 0;

public:

    virtual bool start(end_point local_point) = 0;

    virtual bool receive_available() = 0;

    virtual recv_result receive_from(recv_predicate predicate, end_point remote_point) = 0;

    virtual bool send(const char* data, unsigned short data_size, const end_point& remote_point) = 0;

    virtual void stop() = 0;
};

#endif