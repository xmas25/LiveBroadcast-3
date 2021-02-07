cmake_minimum_required(VERSION 2.8)

project("LiveBroadcastServer")

set(CMAKE_CXX_FLAGS "-Wall -Werror -std=c++17 -g -o0")

include_directories(${CMAKE_SOURCE_DIR})

link_directories(/usr/lib64/mysql)

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
		network/Callback.h
		network/multiplexing/Epoll.cpp
		network/multiplexing/MultiplexingBase.cpp
		network/multiplexing/Select.cpp
		network/PlatformNetwork.cpp
        network/protocol/RtmpServerConnection.cpp
        network/protocol/RtmpClientConnection.cpp
		network/TcpClient.cpp
		network/Connector.cpp
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

set(DB_MYSQL_SRC
		mysql/DbMysql.cpp
		mysql/QueryResult.cpp
		mysql/Field.cpp
		)

set(MAPPER_SRCS
		mapper/UserMapper.cpp
)

set(THREAD_SRCS
		thread/Thread.cpp
		thread/ThreadPool.cpp
		thread/Mutex.cpp
		thread/Condition.cpp
		)

set(ALL_SRCS
		${NETWORK_SRCS}
		${UTILS_SRCS}
		${DB_MYSQL_SRC}
		${MAPPER_SRCS}
		${THREAD_SRCS}
)

set(ALL_SRCS_NO_DB
		${NETWORK_SRCS}
		${UTILS_SRCS}
		${THREAD_SRCS}
		)

# 后续会修改为模块化 因为过早模块化 会导致每次修改代码后需要rebuild才能更新 故临时使用这种方式

add_executable(RtmpManagerTest utils/codec/test/RtmpManagerTest.cpp ${UTILS_SRCS} ${UTILS_HEADERS})

add_executable(LiveBroadcastServer main.cpp ${ALL_SRCS})
target_link_libraries(LiveBroadcastServer mysqlclient)

add_executable(LiveBroadcastServerTest test/MainTest.cpp ${ALL_SRCS_NO_DB})
target_link_libraries(LiveBroadcastServerTest mysqlclient)

add_executable(LoggerTest ${UTILS_SRCS})

add_executable(TcpServerTest network/test/TcpServerTest.cpp ${ALL_SRCS_NO_DB})

add_executable(ConnectorTest network/test/ConnectorTest.cpp ${ALL_SRCS_NO_DB})

add_executable(ThreadTest thread/test/ThreadTest.cpp ${ALL_SRCS_NO_DB})
target_link_libraries(ThreadTest pthread)

if(WIN32)
	target_link_libraries(LiveBroadcastServer ws2_32)
	target_link_libraries(TcpServerTest ws2_32)
	target_link_libraries(ConnectorTest ws2_32)
endif()


add_executable(DbMysqlTest mysql/test/DbMysqlTest.cpp ${DB_MYSQL_SRC})
target_link_libraries(DbMysqlTest mysqlclient)

add_executable(UserMapperTest mapper/test/UserMapperTest.cpp ${DB_MYSQL_SRC} ${MAPPER_SRCS} ${UTILS_SRCS})
target_link_libraries(UserMapperTest mysqlclient)