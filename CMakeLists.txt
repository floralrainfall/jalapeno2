cmake_minimum_required(VERSION 3.9)
set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/cmake")
add_subdirectory(deps/bgfx.cmake)
add_subdirectory(deps/assimp)
add_subdirectory(deps/GameNetworkingSockets)
find_package(SDL2 REQUIRED CONFIG)
find_package(PkgConfig REQUIRED)
find_package(easy_profiler CONFIG)
find_package(OpenAL REQUIRED)
pkg_check_modules(shaderc REQUIRED)
project(jalapeno2)
project(jpbsp)
project(jp2test)
project(jp2totalevo)
project(jp2scg)
project(jp2empireofages)
add_library(jalapeno2 STATIC
	engine/imgui/imgui_demo.cpp
	engine/imgui/imgui_draw.cpp
	engine/imgui/imgui_tables.cpp
	engine/imgui/imgui_widgets.cpp
	engine/imgui/imgui_impl_bgfx.cpp
	engine/imgui/imgui_impl_bgfx.h
	engine/imgui/imgui_impl_sdl.cpp
	engine/imgui/imgui_impl_sdl.h
	engine/imgui/imgui_internal.h
	engine/imgui/custom.cpp
	engine/imgui/custom.hpp
	engine/imgui/imgui.cpp
	engine/gns/gns.hpp
	engine/gns/gns.cpp
	engine/gns/gnsclient.cpp
	engine/gns/gnsserver.cpp
	engine/engine.cpp
	engine/engine.hpp
	engine/app.cpp
	engine/app.hpp
	engine/console.cpp
	engine/console.hpp
	engine/mesh.cpp
	engine/mesh.hpp
	engine/scene.cpp
	engine/scene.hpp
	engine/shader.cpp
	engine/shader.hpp
	engine/camera.cpp
	engine/camera.hpp
	engine/texture.cpp
	engine/texture.hpp
	engine/input.cpp
	engine/input.hpp
	engine/ui.cpp
	engine/ui.hpp
	engine/save.cpp
	engine/save.hpp
	engine/filesystem.cpp
	engine/filesystem.hpp
	engine/sound.cpp
	engine/sound.hpp
	engine/stb_image.h
)
add_library(jpbsp STATIC
	xtralibs/jpbsp/jpbsp.cpp
	xtralibs/jpbsp/collision.cpp
	xtralibs/jpbsp/jpbsp.hpp
)
add_executable(jp2test
	test/test.cpp
)
add_executable(jp2totalevo
	totalevo/imgui/implot.cpp
	totalevo/imgui/implot_items.cpp
	totalevo/imgui/implot.h
	totalevo/imgui/implot_internal.h
	totalevo/totalevo.cpp
	totalevo/organism.cpp
	totalevo/prop.cpp
	totalevo/organism.hpp
	totalevo/brain.cpp
	totalevo/brain.hpp
)
add_executable(jp2scg
	sweetcombatguys/scg.cpp
	sweetcombatguys/player.cpp
	sweetcombatguys/player.hpp
)
add_executable(jp2empireofages
	jp2empireofages/empireofages.cpp
	jp2empireofages/map.cpp
	jp2empireofages/map.hpp
	jp2empireofages/building.cpp
	jp2empireofages/building.hpp
	jp2empireofages/perlin.c
	jp2empireofages/perlin.h
)
target_include_directories(jalapeno2 PUBLIC
	engine/
	${shaderc_INCLUDE_DIR}
)
target_link_libraries(jalapeno2
	bgfx SDL2 assimp ${shaderc_LIBRARIES} GameNetworkingSockets ${OPENAL_LIBRARY} example-debugdraw
)
target_link_libraries(jpbsp
	jalapeno2
)
target_include_directories(jpbsp PUBLIC
	xtralibs/jpbsp/
)
target_include_directories(jp2test PUBLIC
	engine
)
target_link_libraries(jp2test 
	jalapeno2 jpbsp
)
target_include_directories(jp2totalevo PUBLIC
	engine
)
target_link_libraries(jp2totalevo 
	jalapeno2 
)
target_include_directories(jp2scg PUBLIC
	engine
)
target_link_libraries(jp2scg
	jalapeno2 jpbsp
)
target_include_directories(jp2empireofages PUBLIC
	engine
)
target_link_libraries(jp2empireofages
	jalapeno2
)
