include(ExternalProject)



# Download and build OpenCV
ExternalProject_Add(
    opencv_project
    GIT_REPOSITORY https://github.com/opencv/opencv.git
    GIT_TAG 4.x
    SOURCE_DIR ${CMAKE_BINARY_DIR}/external/opencv
    BINARY_DIR ${CMAKE_BINARY_DIR}/external/opencv_build
    INSTALL_DIR ${CMAKE_BINARY_DIR}/external/opencv_install
    CMAKE_ARGS
        -DCMAKE_BUILD_TYPE=Release
        -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/external/opencv_install
        -DBUILD_LIST=core,imgproc,imgcodecs  # Build only opencv_core and opencv_imgproc
)


add_executable(opencv_benchmark src/opencv_benchmark.cpp)

add_dependencies(opencv_benchmark opencv_project)


target_include_directories(opencv_benchmark PRIVATE ${CMAKE_BINARY_DIR}/external/opencv_install/include/opencv4)

target_link_libraries(opencv_benchmark
${CMAKE_BINARY_DIR}/external/opencv_install/lib/libopencv_core.so
${CMAKE_BINARY_DIR}/external/opencv_install/lib/libopencv_imgproc.so
${CMAKE_BINARY_DIR}/external/opencv_install/lib/libopencv_imgcodecs.so
)
