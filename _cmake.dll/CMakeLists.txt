cmake_minimum_required(VERSION 3.9)

project(ice_net)

add_library(ice_net SHARED 
    "../src/_ice.net.dll/ice_net.h"
    "../src/_ice.net.dll/ice_net.cpp"

    "../src/ice.core/ice_data.h"
    "../src/ice.core/ice_data.cpp"
    "../src/ice.core/ice_logger.h"
    "../src/ice.core/ice_logger.cpp" 

    "../src/ice.rudp/common/transport/a_client.h"
    "../src/ice.rudp/common/transport/a_client.cpp" 
    "../src/ice.rudp/common/transport/a_server.h" 
    "../src/ice.rudp/common/transport/a_server.cpp"
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

    "../src/ice.sock/udp_client.h"
    "../src/ice.sock/udp_client.cpp"
    "../src/ice.sock/udp_server.h"
    "../src/ice.sock/udp_server.cpp" 
)

target_compile_definitions(ice_net PRIVATE ICE_NET_EXPORTS)

if (CMAKE_VERSION VERSION_GREATER 3.12)
    set_property(TARGET ice_net PROPERTY CXX_STANDARD 20)
endif()
