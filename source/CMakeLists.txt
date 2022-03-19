
########
# precompiled head file
########
set(DV_PRECOMPILED_INCLUDE_FILE
	dv_precompiled.h
)

set(DV_PRECOMPILED_SOURCE_FILE
	dv_precompiled.cpp
)


########
# davinci math source files
########
set(DV_MATH_SOURCE_FILES
	math/dv_vector2.h
	math/dv_vector2.cpp
	math/dv_vector3.h
	math/dv_vector3.cpp
	math/dv_vector4.h
	math/dv_vector4.cpp
	math/dv_matrix3.h
	math/dv_matrix3.cpp
	math/dv_matrix4.h
	math/dv_matrix4.cpp
	math/dv_math_util.h
	math/dv_math_util.cpp
)
source_group("math" FILES ${DV_MATH_SOURCE_FILES})

########
# davinci asset source files
########
set(DV_ASSET_SOURCE_FILES
	asset/dv_model.h
	asset/dv_model.cpp
	asset/dv_texture.h
	asset/dv_texture.cpp
	asset/dv_asset_utility.h
	asset/dv_asset_utility.cpp
)
source_group("asset" FILES ${DV_ASSET_SOURCE_FILES})

########
# davinci scene source files
########
set(DV_SCENE_SOURCE_FILES
	scene/dv_scene.h
	scene/dv_scene.cpp
	scene/dv_scene_object.h
	scene/dv_scene_object.cpp
	scene/dv_camera.h
	scene/dv_camera.cpp
	scene/dv_entity.h
	scene/dv_entity.cpp
)
source_group("scene" FILES ${DV_SCENE_SOURCE_FILES})

########
# davinci device source files
########
set(DV_DEVICE_SOURCE_FILES
	device/dv_render_device.h
	device/dv_render_device.cpp
	device/dv_device_buffer.h
	device/dv_device_buffer.cpp
	device/dv_vertex_buffer.h
	device/dv_vertex_buffer.cpp
	device/dv_vertex_desc.h
	device/dv_vertex_desc.cpp
	device/dv_index_buffer.h
	device/dv_index_buffer.cpp
	device/dv_pixel_buffer.h
	device/dv_pixel_buffer.cpp
	device/dv_constant_buffer.h
	device/dv_constant_buffer.cpp
	device/dv_render_target.h
	device/dv_render_target.cpp
)
source_group("device" FILES ${DV_DEVICE_SOURCE_FILES})

########
# source files
########
set(DV_INCLUDE_FILES
	dv_prerequisites.h
	dv_common.h
	dv_pipe_IA.h
	dv_pipe_PS.h
	dv_pipe_VS.h
	dv_rasterizer.h	 
	dv_render_queue.h   
)

set(DV_SOURCE_FILES 
	dv_pipe_IA.cpp
	dv_pipe_PS.cpp
	dv_pipe_VS.cpp
	dv_rasterizer.cpp
	dv_rasterizer_larrabee.cpp
	dv_render_queue.cpp
)

########
#libDavinci
########

include_directories(
	${DV_AUTO_INCLUDE_PATH}
	${DV_SOURCE_PATH}
	${RAPIDJSON_INCLUDE_DIRS}
	${PNG_INCLUDE_DIRS}
)

if(MSVC)
foreach(src_file ${DV_SOURCE_FILES})
	set_source_files_properties(
		${src_file}
		PROPERTIES
		COMPILE_FLAGS "/Yudv_precompiled.h"
	)
endforeach()
	
set_source_files_properties(${DV_PRECOMPILED_SOURCE_FILE}
	PROPERTIES
	COMPILE_FLAGS "/Ycdv_precompiled.h"
)
endif()

add_library(davinci 
	${DV_PRECOMPILED_INCLUDE_FILE}
	${DV_PRECOMPILED_SOURCE_FILE}
	${DV_MATH_SOURCE_FILES}
	${DV_ASSET_SOURCE_FILES}
	${DV_SCENE_SOURCE_FILES}
	${DV_DEVICE_SOURCE_FILES}
	${DV_SOURCE_FILES}
	${DV_INCLUDE_FILES}
)
