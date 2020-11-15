set(UTILS_SRCS
	"File.cpp"
	"Buffer.cpp"
)

set(UTILS_HEADERS
	"File.h"
	"Buffer.h"
)

add_library(utils ${UTILS_SRCS} ${UTILS_HEADERS})