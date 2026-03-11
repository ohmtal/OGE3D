# NOTE: do not use it! Not working so far.
# oge3d.cmake

set(OGE3D_DIR ${CMAKE_CURRENT_LIST_DIR}/..)

macro(configure_project_target TARGET_NAME PROJECT_DIR ASSETS_DIR)
    # Save the sources (everything after ASSETS_DIR)
    set(PROJECT_SOURCES ${ARGN})

    # Initialize the project
    project(${TARGET_NAME})

    set(TORQUE_APP_NAME ${TARGET_NAME})
    set(TORQUE_APP_DIR ${PROJECT_DIR})
    
    # We want generated files to go into the build directory
    set(projectOutDir "${CMAKE_CURRENT_BINARY_DIR}")
    set(projectSrcDir "${CMAKE_CURRENT_BINARY_DIR}")

    include(${OGE3D_DIR}/Tools/CMake/basics.cmake)
    setupVersionNumbers()

    # Delay the final executable creation so we can add our own files
    set(OGE3D_DELAY_FINISH ON)
    include(${OGE3D_DIR}/Tools/CMake/torque3d.cmake)
    unset(OGE3D_DELAY_FINISH)

    # Add the project-specific source files
    foreach(src_file ${PROJECT_SOURCES})
        addFile(${PROJECT_DIR}/${src_file})
    endforeach()

    # Add the project directory to include paths
    addInclude("${PROJECT_DIR}")

    # Finalize the executable
    finishExecutable()
    
    # Handle assets: for now we just log it, but typically one might want to 
    # symlink the assets directory into the build folder so the app can find them.
    if(NOT "${ASSETS_DIR}" STREQUAL "")
        message(STATUS "Assets for ${TARGET_NAME} are located at ${PROJECT_DIR}/${ASSETS_DIR}")
        # Option: Create a symlink or copy assets to projectOutDir/assets
        # file(CREATE_LINK "${PROJECT_DIR}/${ASSETS_DIR}" "${projectOutDir}/${ASSETS_DIR}" SYMBOLIC)
    endif()

endmacro()
