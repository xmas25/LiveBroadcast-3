cmake_minimum_required(VERSION 2.8)

project("LiveBroadcastServer")

set(CMAKE_CXX_FLAGS "-Wall -Werror -std=c++17")

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

set(ALL_SRCS
		${NETWORK_SRCS}
		${UTILS_SRCS}
		${DB_MYSQL_SRC}
		${MAPPER_SRCS}
)

# 后续会修改为模块化 因为过早模块化 会导致每次修改代码后需要rebuild才能更新 故临时使用这种方式

add_executable(RtmpManagerTest utils/codec/test/RtmpManagerTest.cpp ${UTILS_SRCS} ${UTILS_HEADERS})

add_executable(LiveBroadcastServer main.cpp ${NETWORK_SRCS} ${NETWORK_HEADERS} ${UTILS_SRCS} ${UTILS_HEADERS})

add_executable(LoggerTest utils/test/LoggerTest.cpp utils/Logger.h utils/Logger.cpp utils/PlatformBase.h utils/PlatformBase.cpp)

add_executable(TcpServerTest network/test/TcpServerTest.cpp ${NETWORK_SRCS} ${NETWORK_HEADERS} ${UTILS_SRCS} ${UTILS_HEADERS})

add_executable(ConnectorTest network/test/ConnectorTest.cpp ${NETWORK_SRCS} ${NETWORK_HEADERS} ${UTILS_SRCS} ${UTILS_HEADERS})

if(WIN32)
	target_link_libraries(LiveBroadcastServer ws2_32)
	target_link_libraries(TcpServerTest ws2_32)
	target_link_libraries(ConnectorTest ws2_32)
endif()


add_executable(DbMysqlTest mysql/test/DbMysqlTest.cpp ${DB_MYSQL_SRC})
target_link_libraries(DbMysqlTest mysqlclient)

add_executable(UserMapperTest mapper/test/UserMapperTest.cpp ${DB_MYSQL_SRC} ${MAPPER_SRCS} ${UTILS_SRCS})
target_link_libraries(UserMapperTest mysqlclient)