set(SERVER_SRCS
	"FlvManager.cpp"
	"codec/RtmpCodec.cpp"
	"codec/FlvCodec.cpp"
)

set(SERVER_HEADERS
	"FlvManager.h"
	"codec/RtmpCodec.h"
	"codec/FlvCodec.h"
)


add_executable(FlvManagerTest "test/FlvManagerTest.cpp" ${SERVER_SRCS} ${SERVER_HEADERS})
target_link_libraries(FlvManagerTest utils)
