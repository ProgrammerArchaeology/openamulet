ADD_EXECUTABLE(tutorial tutorial.cpp)
ADD_SANITIZERS(tutorial)
TARGET_LINK_LIBRARIES(tutorial openamulet)
