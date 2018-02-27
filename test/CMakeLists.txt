PROJECT (LearnedIndex)

FILE (GLOB_RECURSE SRC_LIST "*.cpp")

ADD_EXECUTABLE (test_driver test_driver.cxx ${SRC_LIST})

# TARGET_LINK_LIBRARIES (test_driver learnedindex)
TARGET_LINK_LIBRARIES (test_driver gtest pthread)

INSTALL (TARGETS test_driver 
    RUNTIME DESTINATION bin
    )
