# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/afs/andrew.cmu.edu/usr14/hahmadi/public/fast-canny/external/opencv"
  "/afs/andrew.cmu.edu/usr14/hahmadi/public/fast-canny/external/opencv_build"
  "/afs/andrew.cmu.edu/usr14/hahmadi/public/fast-canny/external/opencv_install"
  "/afs/andrew.cmu.edu/usr14/hahmadi/public/fast-canny/opencv_project-prefix/tmp"
  "/afs/andrew.cmu.edu/usr14/hahmadi/public/fast-canny/opencv_project-prefix/src/opencv_project-stamp"
  "/afs/andrew.cmu.edu/usr14/hahmadi/public/fast-canny/opencv_project-prefix/src"
  "/afs/andrew.cmu.edu/usr14/hahmadi/public/fast-canny/opencv_project-prefix/src/opencv_project-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/afs/andrew.cmu.edu/usr14/hahmadi/public/fast-canny/opencv_project-prefix/src/opencv_project-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/afs/andrew.cmu.edu/usr14/hahmadi/public/fast-canny/opencv_project-prefix/src/opencv_project-stamp${cfgdir}") # cfgdir has leading slash
endif()
