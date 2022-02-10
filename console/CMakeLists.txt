
include_directories(
    ${DV_AUTO_INCLUDE_PATH}
	${DV_SOURCE_PATH}
)

add_executable(dconsole WIN32
    dc_main.cpp
	dc_sample.h
	dc_sample.cpp
	dc_sample01.h
	dc_sample01.cpp
	dc_sample02.h
	dc_sample02.cpp
	dc_sample03.h
	dc_sample03.cpp
	dc_vs_standard.h
	dc_ps_dirlight.h
	dc_ps_solid.h
	dc_ps_texture.h
	dc_ps_material.h
	dc_arcball_camera.h
)

set_property(TARGET dconsole PROPERTY FOLDER "console")
target_link_libraries(dconsole 
	davinci 
	glfw 
	OpenGL::GL
	${PNG_LIBRARIES}
	${ZLIB_LIBRARIES}
)

if (MSVC)
	# Tell MSVC to use main instead of WinMain for Windows subsystem executables
	set_target_properties(dconsole PROPERTIES LINK_FLAGS "/ENTRY:mainCRTStartup")
endif()

if(APPLE)
	set(CMAKE_EXE_LINKER_FLAGS "-framework Cocoa -framework IOKit -framework CoreFoundation -framework CoreVideo -framework OpenGL")
endif()

