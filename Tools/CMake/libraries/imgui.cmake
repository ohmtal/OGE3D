# -----------------------------------------------------------------------------
#  Copyright (c) 2014 GarageGames, LLC
#  Copyright (c) 2025 Thomas Hühn (XXTH)
#  SPDX-License-Identifier: MIT
# -----------------------------------------------------------------------------

project(imgui)

# set(IMGGUI_DIR "${libDir}/imgui")

# set(IMGUI_SOURCES
# #  --- imggui ---
#     ${IMGGUI_DIR}/imgui.cpp
#     ${IMGGUI_DIR}/imgui_draw.cpp
#     ${IMGGUI_DIR}/imgui_widgets.cpp
#     ${IMGGUI_DIR}/imgui_tables.cpp
#     # also demo ?
#     ${IMGGUI_DIR}/imgui_demo.cpp
# # -- fixme SDL2 ----
#     ${IMGGUI_DIR}/backends/imgui_impl_sdl2.cpp
#     ${IMGGUI_DIR}/backends/imgui_impl_opengl3.cpp
# )
#
# LIST(APPEND ${PROJECT_NAME}_files "${IMGUI_SOURCES}")
# LIST(APPEND ${PROJECT_NAME}_paths "${IMGUI_DIR}")
#
#
# addInclude( "${IMGUI_DIR}" )
# finishLibrary()

addFile("${libDir}/imgui/backends/imgui_impl_sdl2.cpp")
addFile("${libDir}/imgui/backends/imgui_impl_opengl3.cpp")
addInclude( "${libDir}/imgui" )
finishLibrary("${libDir}/imgui")

