ADD_SUBDIRECTORY(agate)
ADD_SUBDIRECTORY(circuit)
ADD_SUBDIRECTORY(examples)
ADD_SUBDIRECTORY(tutorial)

ADD_EXECUTABLE(av av.cpp)
ADD_SANITIZERS(av)
TARGET_LINK_LIBRARIES(av openamulet)

ADD_EXECUTABLE(checkers checkers.cpp)
ADD_SANITIZERS(checkers)
TARGET_LINK_LIBRARIES(checkers openamulet)

ADD_EXECUTABLE(color color.cpp)
ADD_SANITIZERS(color)
TARGET_LINK_LIBRARIES(color openamulet)

ADD_EXECUTABLE(hsv hsv.cpp)
ADD_SANITIZERS(hsv)
TARGET_LINK_LIBRARIES(hsv openamulet)

ADD_EXECUTABLE(netpong netpong.cpp)
ADD_SANITIZERS(netpong)
TARGET_LINK_LIBRARIES(netpong openamulet)

ADD_EXECUTABLE(pong pong.cpp)
ADD_SANITIZERS(pong)
TARGET_LINK_LIBRARIES(pong openamulet)

ADD_EXECUTABLE(space space.cpp)
ADD_SANITIZERS(space)
TARGET_LINK_LIBRARIES(space openamulet)

ADD_EXECUTABLE(space2 space2.cpp)
ADD_SANITIZERS(space2)
TARGET_LINK_LIBRARIES(space2 openamulet)

ADD_EXECUTABLE(tree tree.cpp)
ADD_SANITIZERS(tree)
TARGET_LINK_LIBRARIES(tree openamulet)
