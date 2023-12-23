#pragma once

#ifndef ICE_A_CLIENT
#define ICE_A_CLIENT

#include "end_point.h"

class a_client
{

public:

    struct recv_result
    {
        char* recv_arr = nullptr;

        unsigned short recv_size = 0;
    };

public:

    virtual ~a_client() = default;

public:

    virtual end_point get_local_point() = 0; 

    virtual end_point get_remote_point() = 0;

public:

    virtual bool connect(end_point& remote_point, end_point& local_point) = 0;

    virtual bool receive_available() = 0;

    virtual recv_result receive() = 0;

    virtual bool send(char* data, unsigned short data_size) = 0;

    virtual void disconnect() = 0;
};

#endif