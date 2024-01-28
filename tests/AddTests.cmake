message("Building tests")

find_package(OpenCV REQUIRED COMPONENTS imgproc opencv_highgui)

add_executable(interpolationTests
  #Huenicorn
  include/Huenicorn/ImageProcessing.hpp
  include/Huenicorn/Interpolation.hpp
  include/Huenicorn/Logger.hpp
  src/ImageProcessing.cpp
  src/Interpolation.cpp
  src/Logger.cpp

  # Test
  tests/src/interpolationTests.cpp
)

target_include_directories(interpolationTests PUBLIC
  include
)

target_link_libraries(interpolationTests PUBLIC
  ${OpenCV_LIBS}
)