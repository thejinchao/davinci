
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
# source files
########
set(DV_INCLUDE_FILES
    dv_prerequisites.h
    dv_math_util.h
    dv_vector2.h
    dv_vector3.h
    dv_vector4.h
    dv_matrix3.h
    dv_matrix4.h
    dv_common.h
    dv_constant_buffer.h
    dv_device_buffer.h  
    dv_pipe_IA.h
    dv_pipe_PS.h
    dv_pipe_VS.h
    dv_pixel_buffer.h   
    dv_rasterizer.h     
    dv_render_device.h  
    dv_render_queue.h   
    dv_render_target.h
    dv_vertex_desc.h
    dv_asset_utility.h
    dv_index_buffer.h 
    dv_model.h        
    dv_texture.h      
    dv_vertex_buffer.h
    dv_camera.h
    dv_entity.h
    dv_scene.h
    dv_scene_object.h
)

set(DV_SOURCE_FILES 
    dv_math_util.cpp
    dv_vector2.cpp
    dv_vector3.cpp
    dv_vector4.cpp
    dv_matrix3.cpp
    dv_matrix4.cpp
    dv_constant_buffer.cpp
    dv_device_buffer.cpp
    dv_pipe_IA.cpp
    dv_pipe_PS.cpp
    dv_pipe_VS.cpp
    dv_pixel_buffer.cpp
    dv_rasterizer.cpp
	dv_rasterizer_larrabee.cpp
    dv_render_device.cpp
    dv_render_queue.cpp
    dv_render_target.cpp
    dv_vertex_desc.cpp
    dv_asset_utility.cpp
    dv_index_buffer.cpp
    dv_model.cpp
    dv_texture.cpp
    dv_vertex_buffer.cpp
    dv_camera.cpp
    dv_entity.cpp
    dv_scene.cpp
    dv_scene_object.cpp
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
    ${DV_SOURCE_FILES} 
    ${DV_INCLUDE_FILES} 
)
