set(NETWORK_SRCS
	"TcpServer.cpp"
	"Socket.cpp"
	"TcpConnection.cpp"
	"EventLoop.cpp"
	"Acceptor.cpp"
	"Epoll.cpp"
	"Channel.cpp"
	"InetAddress.cpp"
	"SocketOps.cpp"
)

set(NETWORK_HEADERS
	"TcpServer.h"
	"Socket.h"
	"TcpConnection.h"
	"EventLoop.h"
	"Acceptor.h"
	"Epoll.h"
	"Channel.h"
	"InetAddress.h"
	"SocketOps.h"
	"Callback.h"
)

add_library(net ${NETWORK_SRCS} ${NETWORK_HEADERS})