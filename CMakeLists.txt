cmake_minimum_required(VERSION 3.16)

set(PROJECT_NAME huenicorn)
set(BINARY_NAME huenicorn)

project(${PROJECT_NAME} VERSION 1.0.11 LANGUAGES CXX)


set(BUILD_DOCUMENTATION FALSE CACHE BOOL "Build documentation")
set(BUILD_TESTS FALSE CACHE BOOL "Build tests")


# Check mandatory libraries
find_package(Qt6 REQUIRED COMPONENTS Quick)
find_package(PkgConfig REQUIRED)
find_package(OpenCV REQUIRED COMPONENTS imgproc)
find_package(CURL REQUIRED)
find_package(Crow REQUIRED)
find_package(nlohmann_json 3.2.0 REQUIRED)
find_library(MBEDTLS_LIBS mbedtls REQUIRED)
find_library(MBEDTLS_X509 mbedx509 REQUIRED)
find_library(MBEDTLS_CRYPTO mbedcrypto REQUIRED)

qt_standard_project_setup(REQUIRES 6.8)


# Generate version header
set(VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/include/Huenicorn/Version.hpp")

add_custom_target(generate_version_header DEPENDS ${VERSION_HEADER})

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/config/Version.hpp.in
  ${VERSION_HEADER}
)

# Generate embedded webroot header
function(embed_directory TARGET_DIR)
  file(GLOB_RECURSE WEB_FILES "${TARGET_DIR}/*")
  set(CONCAT_CONTENT "")

  foreach(DATA_FILE ${WEB_FILES})
    get_filename_component(KEY ${DATA_FILE} NAME)
    file(READ ${DATA_FILE} CONTENTS)

    # Escape special characters
    string(REPLACE "\\" "\\\\" CONTENTS "${CONTENTS}")
    string(REPLACE "\"" "\\\"" CONTENTS "${CONTENTS}")
    string(REPLACE "\n" "\\n" CONTENTS "${CONTENTS}")

    set(KEY_VALUE "      {\"${KEY}\", \"${CONTENTS}\"},\n")
    set(CONCAT_CONTENT "${CONCAT_CONTENT}${KEY_VALUE}")
  endforeach()
  set(EMBEDDED_FILES_CONTENT "${EMBEDDED_FILES_CONTENT}${CONCAT_CONTENT}" PARENT_SCOPE)
endfunction()

set(EMBEDDED_FILES_CONTENT "")
embed_directory("${CMAKE_SOURCE_DIR}/webroot")
embed_directory("${CMAKE_SOURCE_DIR}/assets")


set(EMBEDDED_WEBROOT_HEADER "${CMAKE_CURRENT_BINARY_DIR}/include/Huenicorn/EmbeddedWebrootFiles.hpp")


# Custom command to ensure re-running when files change
add_custom_command(
  OUTPUT ${EMBEDDED_WEBROOT_HEADER}
  COMMAND ${CMAKE_COMMAND} -E echo "Regenerating embedded webroot header..."
  COMMAND ${CMAKE_COMMAND} -E touch ${EMBEDDED_WEBROOT_HEADER}  # Ensures file gets updated
  DEPENDS ${WEBROOT_FILES} ${ASSETS_FILES} ${CMAKE_CURRENT_SOURCE_DIR}/config/EmbeddedWebrootFiles.hpp.in
  COMMENT "Updating EmbeddedWebrootFiles.hpp because webroot or assets changed."
)


add_custom_target(generate_webroot_header DEPENDS ${EMBEDDED_WEBROOT_HEADER})

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/config/EmbeddedWebrootFiles.hpp.in
  ${EMBEDDED_WEBROOT_HEADER}
)

# Logo exposed to the QML GUI under a stable qrc alias
set_source_files_properties(
  assets/logo.svg
  PROPERTIES
  QT_RESOURCE_ALIAS "logo.svg"
)

# Theme.qml is a QML singleton
set_source_files_properties(
  src/Gui/Theme.qml
  PROPERTIES
  QT_QML_SINGLETON_TYPE TRUE
)

# Declare executable
qt_add_executable(${BINARY_NAME}
  ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ApiTools.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Channel.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Config.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Credentials.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/CurlHttpClient.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Device.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/DtlsClient.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/DummyGrabber.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/EntertainmentConfiguration.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/EntertainmentConfigurationSelector.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/HttpRequestUtils.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/HuenicornCore.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ImageProcessing.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Interpolation.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Logger.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/PlatformSelector.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/SetupBackend.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Streamer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/TickSynchronizer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/UV.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/WebUIBackend.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Gui/QtController.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Gui/ChannelListModel.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Gui/EntertainmentConfigurationModel.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/Huenicorn/ApiTools.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/Huenicorn/Channel.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/Huenicorn/Color.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/Huenicorn/Config.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/Huenicorn/Credentials.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/Huenicorn/CurlHttpClient.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/Huenicorn/Device.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/Huenicorn/DtlsClient.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/Huenicorn/DummyGrabber.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/Huenicorn/EntertainmentConfiguration.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/Huenicorn/EntertainmentConfigurationSelector.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/Huenicorn/HttpRequestUtils.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/Huenicorn/HuenicornCore.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/Huenicorn/IGrabber.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/Huenicorn/ImageProcessing.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/Huenicorn/Interpolation.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/Huenicorn/IRestServer.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/Huenicorn/Logger.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/Huenicorn/PlatformSelector.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/Huenicorn/SetupBackend.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/Huenicorn/Streamer.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/Huenicorn/TickSynchronizer.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/Huenicorn/TimingDefinitions.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/Huenicorn/UV.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/Huenicorn/WebUIBackend.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/Huenicorn/Gui/QtController.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/Huenicorn/Gui/ChannelListModel.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/Huenicorn/Gui/EntertainmentConfigurationModel.hpp

  ${VERSION_HEADER}
  ${EMBEDDED_WEBROOT_HEADER}
)

qt_add_qml_module(${BINARY_NAME}
  URI huenicorn_UI
  VERSION 1.0
  QML_FILES
  src/Gui/Main.qml
  src/Gui/Theme.qml
  src/Gui/SetupPage.qml
  src/Gui/MainPage.qml
  src/Gui/ScreenArea.qml
  RESOURCES
  assets/logo.svg
  SOURCES
  src/Gui/QtController.cpp
  src/Gui/ChannelListModel.cpp
  src/Gui/EntertainmentConfigurationModel.cpp
  include/Huenicorn/Gui/QtController.hpp
  include/Huenicorn/Gui/ChannelListModel.hpp
  include/Huenicorn/Gui/EntertainmentConfigurationModel.hpp
)




add_dependencies(${PROJECT_NAME} generate_version_header generate_webroot_header)

if(UNIX AND NOT APPLE) # For Gnu/Linux
  include(cmake/platforms/GnuLinux.cmake)
endif()

set_target_properties(${BINARY_NAME} PROPERTIES CXX_STANDARD 20)

target_include_directories(${BINARY_NAME} PUBLIC
  ${OpenCV_INCLUDE_DIRS}
  include
  ${CMAKE_CURRENT_BINARY_DIR}/include
)

target_link_libraries(${BINARY_NAME} PUBLIC
  ${OpenCV_LIBS}
  ${CURL_LIBRARIES}
  ${MBEDTLS_LIBS}
  ${MBEDTLS_X509}
  ${MBEDTLS_CRYPTO}
  nlohmann_json::nlohmann_json
  Qt6::Quick
)


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

include(GNUInstallDirs)
install(TARGETS ${BINARY_NAME}
  BUNDLE DESTINATION .
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)


# Post build commands

# Copy readme to build
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy
  ${CMAKE_CURRENT_SOURCE_DIR}/README.md $<TARGET_FILE_DIR:${PROJECT_NAME}>/README.md
)

# Copy screenshots to build
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_CURRENT_SOURCE_DIR}/screenshots $<TARGET_FILE_DIR:${PROJECT_NAME}>/screenshots
)

# Copy assets to build
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_CURRENT_SOURCE_DIR}/assets $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
)

# Copy license to build
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy
  ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt $<TARGET_FILE_DIR:${PROJECT_NAME}>/LICENSE.txt
)
