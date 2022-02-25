add_executable(testApp src/main.cpp)

set(CMAKE_CXX_STANDARD 17)

target_link_libraries(testApp PRIVATE lib)
