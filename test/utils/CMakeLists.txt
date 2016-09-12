ADD_EXECUTABLE(filesearching filesearching.cpp)
TARGET_LINK_LIBRARIES(filesearching openamulet)

ADD_EXECUTABLE(testloadsave testloadsave.cpp)
TARGET_LINK_LIBRARIES(testloadsave openamulet)

# Doesn't build
# ADD_EXECUTABLE(testmaps testmaps.cpp)
# TARGET_LINK_LIBRARIES(testmaps openamulet)
