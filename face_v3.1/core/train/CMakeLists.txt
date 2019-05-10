project(train)
cmake_minimum_required(VERSION 3.0)

find_package(OpenCV REQUIRED)
add_executable(train train.cpp)
target_link_libraries(train ${OpenCV_LIBS})
