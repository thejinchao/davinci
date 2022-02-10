enable_testing()

set(GTEST_ROOT $ENV{LIBRARIES_SDK_ROOT}/gtest)
find_package(GTest REQUIRED)

include_directories(${GTEST_INCLUDE_DIRS})

add_subdirectory(unit)
add_subdirectory(rasterization)
