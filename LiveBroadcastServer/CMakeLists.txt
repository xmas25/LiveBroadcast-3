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
		network/multiplexing/Select.cpp
		network/PlatformNetwork.cpp
        network/protocol/RtmpServerConnection.cpp
        network/protocol/RtmpClientConnection.cpp
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
		network/multiplexing/Epoll.h
		network/multiplexing/MultiplexingBase.h
		network/multiplexing/Select.h
		network/PlatformNetwork.h
        network/protocol/RtmpServerConnection.h
        network/protocol/RtmpClientConnection.h
		)

set(UTILS_SRCS
		utils/codec/FlvManager.cpp
		utils/codec/RtmpManager.cpp
		utils/codec/RtmpCodec.cpp
		utils/codec/FlvCodec.cpp
		utils/Buffer.cpp
		utils/File.cpp
		utils/Timestamp.cpp
		utils/Logger.cpp
		utils/Format.cpp
		)

set(UTILS_HEADERS
		utils/codec/FlvManager.h
		utils/codec/RtmpManager.h
		utils/codec/RtmpCodec.h
		utils/codec/FlvCodec.h
		utils/Buffer.h
		utils/File.h
		utils/Timestamp.h
		utils/Logger.h
		utils/Format.cpp
)

# 后续会修改为模块化 因为过早模块化 会导致每次修改代码后需要rebuild才能更新 故临时使用这种方式

add_executable(RtmpManagerTest utils/codec/test/RtmpManagerTest.cpp ${UTILS_SRCS} ${UTILS_HEADERS})

add_executable(main main.cpp ${NETWORK_SRCS} ${NETWORK_HEADERS} ${UTILS_SRCS} ${UTILS_HEADERS})

add_executable(LoggerTest utils/test/LoggerTest.cpp utils/Logger.h utils/Logger.cpp utils/PlatformBase.h utils/PlatformBase.cpp)

add_executable(TcpServerTest network/test/TcpServerTest.cpp ${NETWORK_SRCS} ${NETWORK_HEADERS} ${UTILS_SRCS} ${UTILS_HEADERS})

if(WIN32)
	target_link_libraries(main ws2_32)
	target_link_libraries(TcpServerTest ws2_32)
endif()
