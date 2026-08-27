cmake_minimum_required(VERSION 3.19)

set(CMAKE_CXX_STANDARD 20)
project(Main)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

include(FetchContent)

FetchContent_Declare(
  bulletphysics
  URL      https://github.com/bulletphysics/bullet3/archive/refs/tags/3.24.tar.gz
)

FetchContent_Declare(
    miniaudio
    URL https://raw.githubusercontent.com/mackron/miniaudio/0.11.21/miniaudio.h
    DOWNLOAD_NO_EXTRACT TRUE
)

FetchContent_MakeAvailable(miniaudio)

set(INSTALL_LIBS OFF CACHE BOOL "" FORCE)
set(BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(BUILD_BULLET2_DEMOS OFF CACHE BOOL "" FORCE)
set(BUILD_EXTRAS OFF CACHE BOOL "" FORCE)
set(BUILD_CPU_DEMOS OFF CACHE BOOL "" FORCE)
set(USE_MSVC_RUNTIME_LIBRARY_DLL ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(bulletphysics)


if(WIN32)
  list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
endif()


if(CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CMAKE_CXX_FLAGS "-O3")
endif()

file(GLOB SOURCES
  Main.cpp
  src/glad.c
  window/*.cpp
  tools/*.cpp
  opengl/*.cpp
  model/*.cpp
  imgui/*.cpp
  tinygltf/*.cc
)

if(WIN32)
    list(APPEND SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/Logo.rc")
endif()


add_executable(Main ${SOURCES} "opengl/Localization.cpp" "opengl/Localization.h"  "model/Enemy.h" "model/Enemy.cpp" "model/AIStates.h" "model/AIStates.cpp" "opengl/GraphicsManager.h" "opengl/GraphicsManager.cpp" "opengl/SkyBox.h" "opengl/SkyBox.cpp" "opengl/GameStateMachine.h" "opengl/MenuController.h" "opengl/MenuController.cpp" "model/EnemyTypes.h" "model/Bossconway.h" "model/Bossconway.cpp" "model/Bossbismark.h" "model/Bossbismark.cpp" "model/JigglePhysics.h" "model/JigglePhysics.cpp" "opengl/audio.cpp" "opengl/audio.h")

target_include_directories(Main 
PUBLIC include src window tools opengl model imgui tinygltf ${bulletphysics_SOURCE_DIR}/src ${bulletphysics_BINARY_DIR}/src ${miniaudio_SOURCE_DIR})

find_package(glfw3 3.3 REQUIRED)
set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)
find_package(glm REQUIRED)


file(GLOB GLSL_SOURCE_FILES
  shader/*.frag
  shader/*.vert
)

add_custom_target(
  Shaders
  DEPENDS ${GLSL_SOURCE_FILES}
)
add_dependencies(Main Shaders)

add_custom_command(TARGET Shaders POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  "$<TARGET_PROPERTY:Main,SOURCE_DIR>/shader"
  "$<TARGET_PROPERTY:Main,BINARY_DIR>/$<CONFIGURATION>/shader"
)


file(GLOB TEX_SOURCE_FILES
  textures/*
)

add_custom_target(
  Textures
  DEPENDS ${TEX_SOURCE_FILES}
)
add_dependencies(Main Textures)

add_custom_command(TARGET Textures POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  "$<TARGET_PROPERTY:Main,SOURCE_DIR>/textures"
  "$<TARGET_PROPERTY:Main,BINARY_DIR>/$<CONFIGURATION>/textures"
)


file(GLOB ASSET_FILES
  assets/*
)

add_custom_target(
  Assets
  DEPENDS ${ASSET_FILES}
)

add_dependencies(Main Assets)

add_custom_command(TARGET Assets POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  "$<TARGET_PROPERTY:Main,SOURCE_DIR>/assets"
  "$<TARGET_PROPERTY:Main,BINARY_DIR>/$<CONFIGURATION>/assets"
)

if(UNIX)
  set(GLFW3_LIBRARY glfw)
endif()

include_directories(${GLFW3_INCLUDE_DIR} ${GLM_INCLUDE_DIR} ${bulletphysics_SOURCE_DIR}/src ${bulletphysics_BINARY_DIR}/src)

if(MSVC)

  target_compile_options(Main PRIVATE /source-charset:windows-1251 /execution-charset:utf-8 /Zc:char8_t-)
  target_link_libraries(Main ${GLFW3_LIBRARY} OpenGL::GL BulletDynamics BulletCollision LinearMath winmm) 
else()
 
  target_link_libraries(Main ${GLFW3_LIBRARY} OpenGL::GL stdc++ m BulletDynamics  BulletCollision LinearMath winmm)
endif()

if(MSVC)
   set_target_properties(Main PROPERTIES 
       WIN32_EXECUTABLE TRUE
        LINK_FLAGS "/ENTRY:mainCRTStartup"
    )
endif()

target_compile_definitions(Main PRIVATE $<$<CONFIG:Release>:NDEBUG>)

if(MSVC)
   
    target_compile_options(Main PRIVATE $<$<CONFIG:Release>:/GL>)

    target_link_options(Main PRIVATE $<$<CONFIG:Release>:/LTCG /DEBUG:NONE>)

    add_custom_command(TARGET Main POST_BUILD
        COMMAND editbin /RELEASE $<TARGET_FILE:Main>
        COMMENT "Stripping debug directory from release binary"
    )
endif()

install(TARGETS Main DESTINATION .)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/assets/" DESTINATION assets)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/textures/" DESTINATION textures)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/shader/" DESTINATION shader)


set(CPACK_PACKAGE_NAME "Tentacle Shippuden")
set(CPACK_PACKAGE_VENDOR "Dev")
set(CPACK_PACKAGE_VERSION "1.0.15")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Enty Furry Beast")

set(CPACK_GENERATOR "ZIP")
set(CPACK_NSIS_PACKAGE_NAME "Tentacle Shippuden")

set(CPACK_NSIS_MENU_LINKS "Main.exe" "Tentacle Shippuden")


set(CPACK_NSIS_CREATE_ICONS_EXTRA "
  SetOutPath \\\"$INSTDIR\\\"
  CreateShortCut \\\"$DESKTOP\\\\Tentacle Shippuden.lnk\\\" \\\"$INSTDIR\\\\Main.exe\\\" \\\"\\\" \\\"$INSTDIR\\\\Main.exe\\\" 0
")

set(CPACK_NSIS_DELETE_ICONS_EXTRA "
  Delete \\\"$DESKTOP\\\\Tentacle Shippuden.lnk\\\"
")

set(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}/Logo.ico")
set(CPACK_NSIS_MUI_UNIICON "${CMAKE_CURRENT_SOURCE_DIR}/Logo.ico")

set(CPACK_NSIS_INSTALLED_ICON_NAME "Main.exe")

include(CPack)