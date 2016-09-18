ADD_EXECUTABLE(example1 example1.cpp example1mu.cpp)
TARGET_LINK_LIBRARIES(example1 openamulet)

ADD_EXECUTABLE(example2 example2.cpp example2db.cpp)
TARGET_LINK_LIBRARIES(example2 openamulet)

ADD_EXECUTABLE(goodbye_button goodbye_button.cpp)
TARGET_LINK_LIBRARIES(goodbye_button openamulet)

ADD_EXECUTABLE(goodbye_inter goodbye_inter.cpp)
TARGET_LINK_LIBRARIES(goodbye_inter openamulet)

ADD_EXECUTABLE(hello hello.cpp)
TARGET_LINK_LIBRARIES(hello openamulet)
