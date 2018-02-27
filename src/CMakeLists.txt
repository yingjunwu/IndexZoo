PROJECT (LearnedIndex)

FILE (GLOB_RECURSE SRC_LIST "*.cpp")

# ADD_LIBRARY (learnedindex SHARED ${SRC_LIST})
ADD_EXECUTABLE (oltp_benchmark oltp_benchmark.cxx)
ADD_EXECUTABLE (olap_benchmark olap_benchmark.cxx)

# TARGET_LINK_LIBRARIES (oltp_benchmark learnedindex)
TARGET_LINK_LIBRARIES (oltp_benchmark jemalloc pthread)

# TARGET_LINK_LIBRARIES (olap_benchmark learnedindex)
TARGET_LINK_LIBRARIES (olap_benchmark jemalloc pthread)


INSTALL (TARGETS oltp_benchmark
    RUNTIME DESTINATION bin
    )

INSTALL (TARGETS olap_benchmark
    RUNTIME DESTINATION bin
    )