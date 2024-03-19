cmake_minimum_required(VERSION 3.8)

project(ice_net)

add_library(${CMAKE_PROJECT_NAME} STATIC 
    "../src/_ice.net.lib/ice_net.h"
    "../src/_ice.net.lib/ice_net.cpp"

    "../src/ice.core/ice_data.h"
    "../src/ice.core/ice_data.cpp"
    "../src/ice.core/ice_logger.h"
    "../src/ice.core/ice_logger.cpp" 

    "../src/ice.rudp/common/transport/a_sock.h"
    "../src/ice.rudp/common/transport/a_sock.cpp" 
    "../src/ice.rudp/common/transport/end_point.h" 
    "../src/ice.rudp/common/transport/end_point.cpp" 

    "../src/ice.rudp/common/rudp.h" 
    "../src/ice.rudp/common/rudp.cpp" 
    "../src/ice.rudp/common/rudp_peer.h"
    "../src/ice.rudp/common/rudp_peer.cpp" 

    "../src/ice.rudp/rudp_client.h" 
    "../src/ice.rudp/rudp_client.cpp"
    "../src/ice.rudp/rudp_connection.h"
    "../src/ice.rudp/rudp_connection.cpp"
    "../src/ice.rudp/rudp_server.h" 
    "../src/ice.rudp/rudp_server.cpp"

    "../src/ice.sock/udp_sock.h"
    "../src/ice.sock/udp_sock.cpp"
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
    set_property(TARGET ice_net PROPERTY CXX_STANDARD 20)
endif()
