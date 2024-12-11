add_executable(detect_edge src/detect_edge.cpp)

add_dependencies(detect_edge opencv_project)

target_include_directories(detect_edge PRIVATE ${CMAKE_BINARY_DIR}/external/opencv_install/include/opencv4)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9)
    target_link_libraries(detect_edge stdc++fs)
endif()

target_link_libraries(detect_edge
${OpenCV_LIB_DIR}/libopencv_core.so
${OpenCV_LIB_DIR}/libopencv_imgproc.so
${OpenCV_LIB_DIR}/libopencv_imgcodecs.so
)

target_link_libraries(detect_edge core)
