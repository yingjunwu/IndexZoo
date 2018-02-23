PROJECT (LearnedIndex)

FILE (GLOB_RECURSE SRC_LIST "*.cpp")

ADD_LIBRARY (learnedindex SHARED ${SRC_LIST})
ADD_EXECUTABLE (main main.cxx)

TARGET_LINK_LIBRARIES (main learnedindex)
TARGET_LINK_LIBRARIES (main jemalloc pthread)

INSTALL (TARGETS main
    RUNTIME DESTINATION bin
    )
