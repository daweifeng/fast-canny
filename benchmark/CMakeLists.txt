include(GNUInstallDirs)
include(ExternalProject)
include(CMakePrintHelpers)

# Download and build OpenCV
ExternalProject_Add(
    opencv_project
    GIT_REPOSITORY https://github.com/opencv/opencv.git
    GIT_TAG 4.10.0
    SOURCE_DIR ${CMAKE_BINARY_DIR}/external/opencv
    BINARY_DIR ${CMAKE_BINARY_DIR}/external/opencv_build
    INSTALL_DIR ${CMAKE_BINARY_DIR}/external/opencv_install
    CMAKE_ARGS
        -DCMAKE_BUILD_TYPE=Release
        -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/external/opencv_install
        -DBUILD_LIST=core,imgproc,imgcodecs
)

set(OpenCV_LIB_DIR ${CMAKE_BINARY_DIR}/external/opencv_install/${CMAKE_INSTALL_LIBDIR})


add_executable(opencv_benchmark src/opencv_benchmark.cpp)

add_dependencies(opencv_benchmark opencv_project)


target_include_directories(opencv_benchmark PRIVATE ${CMAKE_BINARY_DIR}/external/opencv_install/include/opencv4)

target_link_libraries(opencv_benchmark
${OpenCV_LIB_DIR}/libopencv_core.so
${OpenCV_LIB_DIR}/libopencv_imgproc.so
${OpenCV_LIB_DIR}/libopencv_imgcodecs.so
)
