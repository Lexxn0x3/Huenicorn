cmake_minimum_required(VERSION 3.5.0)

set(PROJECT_NAME huenicorn)
set(BINARY_NAME huenicorn)

project(${PROJECT_NAME} VERSION 1.0.4)
#set(RC_NUMBER 1)


set(BUILD_DOCUMENTATION FALSE CACHE BOOL "Build documentation")
set(BUILD_TESTS FALSE CACHE BOOL "Build tests")


set(DISPLAYED_VERSION ${PROJECT_VERSION})

if(RC_NUMBER)
  set(DISPLAYED_VERSION ${DISPLAYED_VERSION}-rc${RC_NUMBER})
endif()


# Check mandatory libraries
find_package(PkgConfig REQUIRED)
find_package(OpenCV REQUIRED COMPONENTS imgproc)
find_package(CURL REQUIRED)
find_package(nlohmann_json 3.2.0 REQUIRED)
find_library(RESTBED_LIBS restbed REQUIRED)
find_library(CURLPP_LIBS curlpp REQUIRED)
find_library(MBEDTLS_LIBS mbedtls REQUIRED)
find_library(MBEDTLS_X509 mbedx509 REQUIRED)
find_library(MBEDTLS_CRYPTO mbedcrypto REQUIRED)


# Check grabber-related libraries
# Begin X11-related
find_package(X11)
# End X11-related

# Begin Pipewire-related
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")
find_package(Gio)
pkg_search_module(LIB_PIPEWIRE OPTIONAL libpipewire-0.3)
pkg_check_modules(LIBGLIB glib-2.0 gio-2.0)
# End Pipewire-related


if(${X11_FOUND})
  set(X11_GRABBER_AVAILABLE TRUE)
  message("Able to build X11 Grabber !")

  set(X11_GRABBER_SOURCES
    src/X11Grabber.cpp
    include/Huenicorn/X11Grabber.hpp
  )
endif()

if(LIB_PIPEWIRE_FOUND AND GIO_FOUND AND LIBGLIB_FOUND)
  set(PIPEWIRE_GRABBER_AVAILABLE TRUE)
  message("Able to build Pipewire Grabber !")

  set(PIPEWIRE_GRABBER_SOURCES
    src/PipewireGrabber.cpp
    src/XdgDesktopPortal.cpp
    include/Huenicorn/PipewireGrabber.hpp
    include/Huenicorn/XdgDesktopPortal.hpp
  )
endif()


if(NOT (PIPEWIRER_GRABBER_AVAILABLE OR X11_GRABBER_AVAILABLE))
  message(FATAL_ERROR "Missing dependencies to compile a least one grabber")
endif()


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
  src/Logger.cpp
  src/Streamer.cpp
  src/RequestUtils.cpp
  src/SetupBackend.cpp
  src/TickSynchronizer.cpp
  src/WebUIBackend.cpp
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
  include/Huenicorn/Logger.hpp
  include/Huenicorn/RequestUtils.hpp
  include/Huenicorn/SetupBackend.hpp
  include/Huenicorn/Streamer.hpp
  include/Huenicorn/TickSynchronizer.hpp
  include/Huenicorn/UV.hpp
  include/Huenicorn/WebUIBackend.hpp
)

target_compile_definitions(${BINARY_NAME} PUBLIC PROJECT_VERSION=${DISPLAYED_VERSION})

set_target_properties(${BINARY_NAME} PROPERTIES CXX_STANDARD 20)

target_include_directories(${BINARY_NAME} PUBLIC
  ${OpenCV_INCLUDE_DIRS}
  include
)

target_link_libraries(${BINARY_NAME} PUBLIC
  ${OpenCV_LIBS}
  ${CURL_LIBRARIES}
  ${CURLPP_LIBS}
  ${RESTBED_LIBS}
  ${MBEDTLS_LIBS}
  ${MBEDTLS_X509}
  ${MBEDTLS_CRYPTO}
  nlohmann_json::nlohmann_json
)

if(X11_GRABBER_AVAILABLE)
  target_compile_definitions(${BINARY_NAME} PUBLIC X11_GRABBER_AVAILABLE=1)
  target_sources(${BINARY_NAME} PRIVATE
    ${X11_GRABBER_SOURCES}
  )

  target_link_libraries(${BINARY_NAME} PUBLIC
    X11::X11
    X11::Xext
    X11::Xrandr
  )
endif()


if(PIPEWIRE_GRABBER_AVAILABLE)
  target_compile_definitions(${BINARY_NAME} PUBLIC PIPEWIRE_GRABBER_AVAILABLE=1)
  target_sources(${BINARY_NAME} PRIVATE
    ${PIPEWIRE_GRABBER_SOURCES}
  )
  
  target_include_directories(${BINARY_NAME} PUBLIC
    ${LIB_PIPEWIRE_INCLUDE_DIRS}
    ${GIO_INCLUDE_DIRS}
    ${GLIB_INCLUDE_DIRS}
  )

  target_link_libraries(${BINARY_NAME} PUBLIC
    GIO::GIO
    ${LIB_PIPEWIRE_LIBRARIES}
    ${LIBGLIB_LIBRARIES}
  )
endif()


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
