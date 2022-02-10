include_directories(
    ${DV_AUTO_INCLUDE_PATH}
    ${DV_SOURCE_PATH}
)


set(DVT_UNIT_SOURCE_FILES
    dvt_unit_main.cpp
    dvt_unit_vector2.cpp
	dvt_unit_vector3.cpp
	dvt_unit_vector4.cpp
	dvt_unit_matrix3.cpp
	dvt_unit_matrix4.cpp
	dvt_unit_rasterizer.cpp
)

add_definitions(-DGLM_FORCE_PURE -DGLM_FORCE_LEFT_HANDED -DGLM_FORCE_DEPTH_ZERO_TO_ONE)

add_executable(dvt_unit 
    dvt_unit_common.h
    ${DVT_UNIT_SOURCE_FILES}
)

set_property(TARGET dvt_unit PROPERTY FOLDER "test")

target_link_libraries(dvt_unit
    davinci
    ${GTEST_LIBRARIES}
    ${PNG_LIBRARIES}
    glm::glm
)

GTEST_ADD_TESTS(dvt_unit "" ${DVT_UNIT_SOURCE_FILES})
