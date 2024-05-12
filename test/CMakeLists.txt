enable_testing()

set(GTEST_ROOT ${DV_THIRDPARTY_ROOT}/gtest)
find_package(GTest REQUIRED)

include_directories(${GTEST_INCLUDE_DIRS})

add_subdirectory(unit)
add_subdirectory(rasterization)
