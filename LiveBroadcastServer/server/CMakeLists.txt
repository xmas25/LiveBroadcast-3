set(SERVER_SRCS
	"FlvManager.cpp"
	"RtmpManager.cpp"
	"codec/RtmpCodec.cpp"
	"codec/FlvCodec.cpp"
)

set(SERVER_HEADERS
	"FlvManager.h"
	"RtmpManager.h"
	"codec/RtmpCodec.h"
	"codec/FlvCodec.h"
)


add_executable(FlvManagerTest "test/FlvManagerTest.cpp" ${SERVER_SRCS} ${SERVER_HEADERS})
target_link_libraries(FlvManagerTest utils)

add_executable(RtmpManagerTest "test/RtmpManagerTest.cpp" ${SERVER_SRCS} ${SERVER_HEADERS})
target_link_libraries(RtmpManagerTest utils)