cmake_minimum_required( VERSION 3.2.0 )

project( node )

include(FetchContent)
FetchContent_Declare(
    vrm
    GIT_REPOSITORY "https://git.martian.ag/verdant/vrm.git"
    )
FetchContent_GetProperties(vrm)
if(NOT vrm_POPULATED)
    execute_process(COMMAND ${CMAKE_COMMAND} -E echo_append "-- Downloading vrm... ")
    FetchContent_Populate(vrm) # does the actual download (I think)
    message(" into ${vrm_SOURCE_DIR}... done. ")
    add_subdirectory( ${vrm_SOURCE_DIR} ${vrm_BINARY_DIR} )
endif()


set( NODE_SRCS 
    src/nodelib.cpp src/node_registry.cpp src/process.cpp 
)

add_custom_command(
    OUTPUT image.h
    COMMAND ${vrm_BINARY_DIR}/vrm ${CMAKE_CURRENT_SOURCE_DIR}/samples/image.vrm > image.h
    DEPENDS vrm ${CMAKE_CURRENT_SOURCE_DIR}/vrm/image.vrm
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)


add_library( node STATIC ${VRM_SRCS} image.h )
target_include_directories(node PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src )
target_link_libraries(node PRIVATE vrm_lib)