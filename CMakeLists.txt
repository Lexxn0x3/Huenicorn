cmake_minimum_required(VERSION 3.0.0)

set(PROJECT_NAME huenicorn)
set(BINARY_NAME huenicorn)

project(${PROJECT_NAME} VERSION 1.0.1)
#set(RC_NUMBER 1)


set(BUILD_DOCUMENTATION FALSE CACHE BOOL "Build documentation")
set(BUILD_TESTS FALSE CACHE BOOL "Build tests")


set(DISPLAYED_VERSION ${PROJECT_VERSION})

if(RC_NUMBER)
  set(DISPLAYED_VERSION ${DISPLAYED_VERSION}-rc${RC_NUMBER})
endif()


find_package(OpenCV REQUIRED)
find_package(CURL REQUIRED)


add_executable(${BINARY_NAME}
  src/main.cpp
  src/ApiTools.cpp
  src/Channel.cpp
  src/Config.cpp
  src/Credentials.cpp
  src/DtlsClient.cpp
  src/EntertainmentConfigurationSelector.cpp
  src/HuenicornCore.cpp
  src/ImageProcessing.cpp
  src/JsonSerializer.cpp
  src/Streamer.cpp
  src/RequestUtils.cpp
  src/SetupBackend.cpp
  src/TickSynchronizer.cpp
  src/WebUIBackend.cpp
  src/X11Grabber.cpp
  include/Huenicorn/ApiTools.hpp
  include/Huenicorn/Channel.hpp
  include/Huenicorn/Color.hpp
  include/Huenicorn/Config.hpp
  include/Huenicorn/Credentials.hpp
  include/Huenicorn/Device.hpp
  include/Huenicorn/DtlsClient.hpp
  include/Huenicorn/EntertainmentConfiguration.hpp
  include/Huenicorn/EntertainmentConfigurationSelector.hpp
  include/Huenicorn/HuenicornCore.hpp
  include/Huenicorn/IGrabber.hpp
  include/Huenicorn/ImageProcessing.hpp
  include/Huenicorn/IRestServer.hpp
  include/Huenicorn/JsonSerializer.hpp
  include/Huenicorn/RequestUtils.hpp
  include/Huenicorn/SetupBackend.hpp
  include/Huenicorn/Streamer.hpp
  include/Huenicorn/TickSynchronizer.hpp
  include/Huenicorn/UV.hpp
  include/Huenicorn/WebUIBackend.hpp
  include/Huenicorn/X11Grabber.hpp
)


target_include_directories(${BINARY_NAME} PUBLIC
  ${OpenCV_INCLUDE_DIRS}
  include
)


set_target_properties(${BINARY_NAME} PROPERTIES CXX_STANDARD 20)
target_link_libraries(${BINARY_NAME} PUBLIC X11 Xext Xrandr ${OpenCV_LIBS} ${CURL_LIBRARIES} curlpp restbed mbedtls mbedcrypto mbedx509)


target_compile_definitions(${BINARY_NAME} PUBLIC PROJECT_VERSION=${DISPLAYED_VERSION})


target_compile_options(${BINARY_NAME} PRIVATE
  -Werror
  -Wall
  -Wextra
  -Wnon-virtual-dtor
  -pedantic
  -Wno-deprecated-declarations
  -Wno-deprecated-copy
)


if(${BUILD_TESTS})
  include(tests/AddTests.cmake)
endif(${BUILD_TESTS})


if(${BUILD_DOCUMENTATION})
  # Initializing Doxygen
  find_package(Doxygen REQUIRED dot OPTIONAL_COMPONENTS mscgen dia)

  # Defining Doxyfile path
  set(doxyfile_in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.doxygen)

  # Setting Doxyfile properties
  set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doc)
  set(DOXYGEN_EXTRACT_ALL YES)
  set(DOXYGEN_FULL_PATH_NAMES NO)

  # Generating doxyfile
  doxygen_add_docs(
    doxygen
    ${PROJECT_SOURCE_DIR}
    COMMENT "Generate man pages"
  )

  # Building Doxygen documentation
  add_custom_target(documentationBuild ALL
    COMMAND ${DOXYGEN_EXECUTABLE} ${doxyfile_in}
    COMMENT "Generating API documentation with Doxygen"
    VERBATIM
  )
endif(${BUILD_DOCUMENTATION})


# Copy webroot to build
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_SOURCE_DIR}/webroot $<TARGET_FILE_DIR:${PROJECT_NAME}>/webroot
)

# Copy readme to build
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy
  ${CMAKE_SOURCE_DIR}/README.md $<TARGET_FILE_DIR:${PROJECT_NAME}>/README.md
)

# Copy screenshots to build
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_SOURCE_DIR}/screenshots $<TARGET_FILE_DIR:${PROJECT_NAME}>/screenshots
)

# Copy license to build
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy
  ${CMAKE_SOURCE_DIR}/LICENSE.txt $<TARGET_FILE_DIR:${PROJECT_NAME}>/LICENSE.txt
)
