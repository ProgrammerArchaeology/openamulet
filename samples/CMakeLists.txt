ADD_SUBDIRECTORY(agate)
ADD_SUBDIRECTORY(circuit)
ADD_SUBDIRECTORY(examples)
ADD_SUBDIRECTORY(tutorial)

ADD_EXECUTABLE(av av.cpp)
TARGET_LINK_LIBRARIES(av openamulet)

ADD_EXECUTABLE(checkers checkers.cpp)
TARGET_LINK_LIBRARIES(checkers openamulet)

ADD_EXECUTABLE(color color.cpp)
TARGET_LINK_LIBRARIES(color openamulet)

ADD_EXECUTABLE(hsv hsv.cpp)
TARGET_LINK_LIBRARIES(hsv openamulet)

ADD_EXECUTABLE(netpong netpong.cpp)
TARGET_LINK_LIBRARIES(netpong openamulet)

ADD_EXECUTABLE(pong pong.cpp)
TARGET_LINK_LIBRARIES(pong openamulet)

ADD_EXECUTABLE(space space.cpp)
TARGET_LINK_LIBRARIES(space openamulet)

ADD_EXECUTABLE(space2 space2.cpp)
TARGET_LINK_LIBRARIES(space2 openamulet)

ADD_EXECUTABLE(tree tree.cpp)
TARGET_LINK_LIBRARIES(tree openamulet)
