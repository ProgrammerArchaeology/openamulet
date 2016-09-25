ADD_EXECUTABLE(example1 example1.cpp)
ADD_SANITIZERS(example1)
TARGET_LINK_LIBRARIES(example1 openamulet)

# We don't have pebbles in this version yet.
# ADD_EXECUTABLE(example1mu example1mu.cpp)
# ADD_SANITIZERS(example1mu)
# TARGET_LINK_LIBRARIES(example1mu openamulet)

ADD_EXECUTABLE(example2 example2.cpp example2db.cpp)
ADD_SANITIZERS(example2)
TARGET_LINK_LIBRARIES(example2 openamulet)

ADD_EXECUTABLE(goodbye_button goodbye_button.cpp)
ADD_SANITIZERS(goodbye_button)
TARGET_LINK_LIBRARIES(goodbye_button openamulet)

ADD_EXECUTABLE(goodbye_inter goodbye_inter.cpp)
ADD_SANITIZERS(goodbye_inter)
TARGET_LINK_LIBRARIES(goodbye_inter openamulet)

ADD_EXECUTABLE(hello hello.cpp)
ADD_SANITIZERS(hello)
TARGET_LINK_LIBRARIES(hello openamulet)
