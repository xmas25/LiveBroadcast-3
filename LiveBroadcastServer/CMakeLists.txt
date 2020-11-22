cmake_minimum_required (VERSION 2.8)

project ("LiveBroadcastServer")

set(CMAKE_CXX_FLAGS "-Wall -Werror -std=c++17")

include_directories(${CMAKE_SOURCE_DIR})

set(NETWORK_SRCS
		network/TcpServer.cpp
		network/Socket.cpp
		network/TcpConnection.cpp
		network/EventLoop.cpp
		network/Acceptor.cpp
		network/multiplexing/Epoll.cpp
		network/Channel.cpp
		network/InetAddress.cpp
		network/SocketOps.cpp
		network/multiplexing/Epoll.cpp
		network/multiplexing/MultiplexingBase.cpp
		)

set(NETWORK_HEADERS
		network/TcpServer.h
		network/Socket.h
		network/TcpConnection.h
		network/EventLoop.h
		network/Acceptor.h
		network/multiplexing/Epoll.h
		network/Channel.h
		network/InetAddress.h
		network/SocketOps.h
		network/Callback.h
		network/Platform.h
		network/multiplexing/Epoll.h
		network/multiplexing/MultiplexingBase.h
		network/multiplexing/Select.cpp network/multiplexing/Select.h network/Platform.cpp)

set(UTILS_SRCS
		utils/codec/FlvManager.cpp
		utils/codec/RtmpManager.cpp
		utils/codec/RtmpCodec.cpp
		utils/codec/FlvCodec.cpp
		utils/Buffer.cpp
		utils/File.cpp
)

set(UTILS_HEADERS
		utils/codec/FlvManager.h
		utils/codec/RtmpManager.h
		utils/codec/RtmpCodec.h
		utils/codec/FlvCodec.h
		utils/Buffer.h
		utils/File.h
)

add_executable(RtmpManagerTest "utils/codec/test/RtmpManagerTest.cpp" ${UTILS_SRCS} ${UTILS_HEADERS})

add_executable(main "main.cpp" ${NETWORK_SRCS} ${NETWORK_HEADERS} ${UTILS_SRCS} ${UTILS_HEADERS})
if(WIN32)
	target_link_libraries(main ws2_32)
endif()
