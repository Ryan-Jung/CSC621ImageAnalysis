# This is the root ITK CMakeLists file.
cmake_minimum_required(VERSION 2.8.12)
if(COMMAND CMAKE_POLICY)
  cmake_policy(SET CMP0003 NEW)
endif()


# This project is designed to be built outside the Insight source tree.
project(ImageAnaylsis)
set(CMAKE_PREFIX_PATH "C:/itk/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY C:/Users/Owner/source/repos/CMakeProject1/CMakeProject1/bin)
# Find ITK.
find_package(ITK CONFIG REQUIRED)
include(${ITK_USE_FILE})

add_executable(ImageAnaylsis Registration.cxx )

target_link_libraries(ImageAnaylsis ${ITK_LIBRARIES})
