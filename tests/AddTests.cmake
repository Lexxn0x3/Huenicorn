add_executable(HuenicornTests 
  include/Huenicorn/ColorUtils.hpp
  include/Huenicorn/Color.hpp
  tests/src/main.cpp
)


find_package(Catch2 REQUIRED)
target_link_libraries(HuenicornTests Catch2::Catch2)


target_include_directories(HuenicornTests PUBLIC
  include
)
