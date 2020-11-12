# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("LiveBroadcastServer")

set(CMAKE_CXX_FLAGS "-g -Wall -Werror")

include_directories(${CMAKE_SOURCE_DIR})

set(NETWORK_SRCS
	"network/TcpServer.cpp"
	"network/Socket.cpp"
	"network/TcpConnection.cpp"
	"network/EventLoop.cpp"
	"network/Acceptor.cpp"
	"network/Epoll.cpp"
	"network/Channel.cpp"
	"network/InetAddress.cpp"
	"network/SocketOps.cpp"
)

set(NETWORK_HEADERS
	"network/TcpServer.h"
	"network/Socket.h"
	"network/TcpConnection.h"
	"network/EventLoop.h"
	"network/Acceptor.h"
	"network/Epoll.h"
	"network/Channel.h"
	"network/InetAddress.h"
	"network/SocketOps.h"
	"network/Callback.h"
)

add_executable (LiveBroadcastServer "main.cpp" "utils/Codec.cpp" "utils/Codec.h" ${NETWORK_SRCS} ${NETWORK_HEADERS})