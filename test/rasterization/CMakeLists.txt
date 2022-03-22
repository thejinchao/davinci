
include_directories(
	${DV_AUTO_INCLUDE_PATH}
	${DV_SOURCE_PATH}
)

if(APPLE)
	set(CMAKE_EXE_LINKER_FLAGS "-framework Cocoa -framework IOKit -framework CoreFoundation -framework CoreVideo -framework OpenGL")
endif()

add_executable(dvt_raster
    dvr_main.cpp
    dvr_triangle.h
    dvr_triangle.cpp
)

set_property(TARGET dvt_raster PROPERTY FOLDER "test")
target_link_libraries(dvt_raster
	davinci
	glfw
	OpenGL::GL
)
