ADD_EXECUTABLE(test_perobj test_perobj.cpp)
TARGET_LINK_LIBRARIES(test_perobj openamulet)

ADD_EXECUTABLE(testdemon testdemon.cpp)
TARGET_LINK_LIBRARIES(testdemon openamulet)

# Does not build
# ADD_EXECUTABLE(testmap_net testmap.cpp)
# TARGET_LINK_LIBRARIES(testmap_net openamulet)

ADD_EXECUTABLE(testobj testobj.cpp)
TARGET_LINK_LIBRARIES(testobj openamulet)

ADD_EXECUTABLE(testorder testorder.cpp)
TARGET_LINK_LIBRARIES(testorder openamulet)

# Does not build
# ADD_EXECUTABLE(testproto testproto.cpp)
# TARGET_LINK_LIBRARIES(testproto openamulet)

ADD_EXECUTABLE(testtype testtype.cpp)
TARGET_LINK_LIBRARIES(testtype openamulet)
