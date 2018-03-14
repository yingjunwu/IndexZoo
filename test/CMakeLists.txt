PROJECT (LearnedIndex)

FILE (GLOB_RECURSE SRC_LIST "*.cpp")

ADD_EXECUTABLE (test_driver test_driver.cxx ${SRC_LIST})

TARGET_LINK_LIBRARIES (test_driver learnedindex)
TARGET_LINK_LIBRARIES (test_driver gtest pthread)


add_definitions(-DWORDS_BIGENDIAN_SET=1)
add_definitions(-DSTDC_HEADERS=1)
add_definitions(-DHAVE_SYS_TYPES_H=1)
add_definitions(-DHAVE_SYS_STAT_H=1)
add_definitions(-DCACHE_LINE_SIZE=64)
add_definitions(-DHAVE_CXX_STATIC_ASSERT=1)
add_definitions(-DHAVE_CXX_CONSTEXPR=1)
add_definitions(-DHAVE_TIME_H=1)
add_definitions(-DHAVE_CLOCK_GETTIME=1)
add_definitions(-DMASSTREE_ROW_TYPE_BAG=1)
add_definitions(-DMASSTREE_MAXKEYLEN=255)
add_definitions(-DHAVE_UNALIGNED_ACCESS=1)
add_definitions(-DHAVE___BUILTIN_CLZ=1)
add_definitions(-DHAVE___BUILTIN_CLZL=1)


INSTALL (TARGETS test_driver 
    RUNTIME DESTINATION bin
    )
