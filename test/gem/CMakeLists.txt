# Does not build
# ADD_EXECUTABLE(testMS1 testMS1.cpp)
# TARGET_LINK_LIBRARIES(testMS1 openamulet)

ADD_EXECUTABLE(testclip testclip.cpp)
TARGET_LINK_LIBRARIES(testclip openamulet)

ADD_EXECUTABLE(testcopy testcopy.cpp)
TARGET_LINK_LIBRARIES(testcopy openamulet)

ADD_EXECUTABLE(testdpy
    testdpy.cpp
    testcolor.cpp
    testfonts.cpp
    testbits.cpp
    testwinprops.cpp
    test_utils.cpp)
TARGET_LINK_LIBRARIES(testdpy openamulet)

ADD_EXECUTABLE(testgem
    testgem.cpp
    testlineprops.cpp
    test_utils.cpp)
TARGET_LINK_LIBRARIES(testgem openamulet)

ADD_EXECUTABLE(testinput testinput.cpp)
TARGET_LINK_LIBRARIES(testinput openamulet)

ADD_EXECUTABLE(testloop testloop.cpp)
TARGET_LINK_LIBRARIES(testloop openamulet)

ADD_EXECUTABLE(testpixmap testpixmap.cpp)
TARGET_LINK_LIBRARIES(testpixmap openamulet)

ADD_EXECUTABLE(testpoints testpoints.cpp)
TARGET_LINK_LIBRARIES(testpoints openamulet)

ADD_EXECUTABLE(testsubwins testsubwins.cpp)
TARGET_LINK_LIBRARIES(testsubwins openamulet)

ADD_EXECUTABLE(testtrans testtrans.cpp)
TARGET_LINK_LIBRARIES(testtrans openamulet)

ADD_EXECUTABLE(testwinsizes testwinsizes.cpp)
TARGET_LINK_LIBRARIES(testwinsizes openamulet)
