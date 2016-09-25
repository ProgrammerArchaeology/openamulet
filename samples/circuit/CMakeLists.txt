ADD_EXECUTABLE(circuit circuit.cpp)
ADD_SANITIZERS(circuit)
TARGET_LINK_LIBRARIES(circuit openamulet)
