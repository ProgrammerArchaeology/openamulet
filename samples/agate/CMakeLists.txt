ADD_EXECUTABLE(agate agate.cpp)
ADD_SANITIZERS(agate)
TARGET_LINK_LIBRARIES(agate openamulet)
