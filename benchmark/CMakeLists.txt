add_executable(opencv_benchmark src/opencv_benchmark.cpp)
add_executable(gaussian_filter_benchmark src/gaussian_filter_benchmark.cpp)
add_executable(gradient_benchmark src/gradient_benchmark.cpp)
add_executable(double_threshold_benchmark src/double_threshold_benchmark.cpp)
add_executable(non_maxima_suppression_benchmark src/non_maxima_suppression_benchmark.cpp)
add_executable(hysteresis_benchmark src/hysteresis_benchmark.cpp)
add_executable(fast_canny_benchmark src/fast_canny_benchmark.cpp)

add_dependencies(opencv_benchmark opencv_project)
add_dependencies(gaussian_filter_benchmark opencv_project)
add_dependencies(gradient_benchmark opencv_project)
add_dependencies(double_threshold_benchmark opencv_project)
add_dependencies(non_maxima_suppression_benchmark opencv_project)
add_dependencies(hysteresis_benchmark opencv_project)
add_dependencies(fast_canny_benchmark opencv_project)


target_include_directories(opencv_benchmark PRIVATE ${CMAKE_BINARY_DIR}/external/opencv_install/include/opencv4)
target_include_directories(gaussian_filter_benchmark PRIVATE ${CMAKE_BINARY_DIR}/external/opencv_install/include/opencv4)
target_include_directories(hysteresis_benchmark PRIVATE ${CMAKE_BINARY_DIR}/external/opencv_install/include/opencv4)
target_include_directories(gradient_benchmark PRIVATE ${CMAKE_BINARY_DIR}/external/opencv_install/include/opencv4)
target_include_directories(double_threshold_benchmark PRIVATE ${CMAKE_BINARY_DIR}/external/opencv_install/include/opencv4)
target_include_directories(non_maxima_suppression_benchmark PRIVATE ${CMAKE_BINARY_DIR}/external/opencv_install/include/opencv4)
target_include_directories(fast_canny_benchmark PRIVATE ${CMAKE_BINARY_DIR}/external/opencv_install/include/opencv4)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9)
    target_link_libraries(opencv_benchmark stdc++fs)
    target_link_libraries(gaussian_filter_benchmark stdc++fs)
    target_link_libraries(gradient_benchmark stdc++fs)
    target_link_libraries(double_threshold_benchmark stdc++fs)
    target_link_libraries(non_maxima_suppression_benchmark stdc++fs)
    target_link_libraries(hysteresis_benchmark stdc++fs)
    target_link_libraries(fast_canny_benchmark stdc++fs)
endif()

target_link_libraries(opencv_benchmark
${OpenCV_LIB_DIR}/libopencv_core.so
${OpenCV_LIB_DIR}/libopencv_imgproc.so
${OpenCV_LIB_DIR}/libopencv_imgcodecs.so
)

target_link_libraries(gaussian_filter_benchmark
${OpenCV_LIB_DIR}/libopencv_core.so
${OpenCV_LIB_DIR}/libopencv_imgproc.so
${OpenCV_LIB_DIR}/libopencv_imgcodecs.so
)

target_link_libraries(hysteresis_benchmark
${OpenCV_LIB_DIR}/libopencv_core.so
${OpenCV_LIB_DIR}/libopencv_imgproc.so
${OpenCV_LIB_DIR}/libopencv_imgcodecs.so
)

target_link_libraries(gradient_benchmark
${OpenCV_LIB_DIR}/libopencv_core.so
${OpenCV_LIB_DIR}/libopencv_imgproc.so
${OpenCV_LIB_DIR}/libopencv_imgcodecs.so
)

target_link_libraries(double_threshold_benchmark
${OpenCV_LIB_DIR}/libopencv_core.so
${OpenCV_LIB_DIR}/libopencv_imgproc.so
${OpenCV_LIB_DIR}/libopencv_imgcodecs.so
)

target_link_libraries(non_maxima_suppression_benchmark
${OpenCV_LIB_DIR}/libopencv_core.so
${OpenCV_LIB_DIR}/libopencv_imgproc.so
${OpenCV_LIB_DIR}/libopencv_imgcodecs.so
)

target_link_libraries(fast_canny_benchmark
${OpenCV_LIB_DIR}/libopencv_core.so
${OpenCV_LIB_DIR}/libopencv_imgproc.so
${OpenCV_LIB_DIR}/libopencv_imgcodecs.so
)


target_link_libraries(opencv_benchmark core)
target_link_libraries(gaussian_filter_benchmark core)
target_link_libraries(hysteresis_benchmark core)
target_link_libraries(gradient_benchmark core)
target_link_libraries(double_threshold_benchmark core)
target_link_libraries(non_maxima_suppression_benchmark core)
target_link_libraries(fast_canny_benchmark core)
