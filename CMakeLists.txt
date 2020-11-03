find_package(OpenCV REQUIRED)
find_package(Threads REQUIRED)

add_executable(programs
  IDDetector.cpp
  MotionDetector.cpp)

set_property(TARGET programs PROPERTY CXX_STANDARD 14)
target_link_libraries(programs ${OpenCV_LIBS})
target_link_libraries(programs Threads::Threads)
