###############################################################################

cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
	message(STATUS "Setting build type to 'Release' as none was specified.")
	set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

include(FindPkgConfig)
include(InstallRequiredSystemLibraries)

################################################################ Project config

project(ansio VERSION 0.1)

configure_file(ansio-config.h.in ansio-config.h)

list(APPEND SOURCES main.cpp ansio.cpp merx.cpp parser.cpp)

############################################################## Everything ImGui

set(IMGUI_DIR ${PROJECT_SOURCE_DIR}/thirdparty/imgui)
list(APPEND INCLUDES ${IMGUI_DIR})
list(APPEND INCLUDES ${IMGUI_DIR}/backends)
list(APPEND SOURCES ${IMGUI_DIR}/imgui.cpp)
list(APPEND SOURCES ${IMGUI_DIR}/imgui_draw.cpp)
list(APPEND SOURCES ${IMGUI_DIR}/backends/imgui_impl_opengl2.cpp)
list(APPEND SOURCES ${IMGUI_DIR}/backends/imgui_impl_sdl.cpp)
list(APPEND SOURCES ${IMGUI_DIR}/imgui_widgets.cpp)

################################################### Everything NativeFileDialog

set(NFD_DIR ${PROJECT_SOURCE_DIR}/thirdparty/nativefiledialog/src)
list(APPEND INCLUDES ${NFD_DIR}/include)
list(APPEND SOURCES ${NFD_DIR}/nfd_common.c)
if(WINDOWS)
	list(APPEND SOURCES ${NFD_DIR}/nfd_win.cpp)
elseif(APPLE)
	list(APPEND SOURCES ${NFD_DIR}/nfd_cocoa.m)
else()
	list(APPEND SOURCES ${NFD_DIR}/nfd_gtk.c)

	if (PKG_CONFIG_FOUND)
		pkg_search_module(GTK3 REQUIRED gtk+-3.0)
		list(APPEND INCLUDES ${GTK3_INCLUDE_DIRS})
		list(APPEND LIBS ${GTK3_LIBRARIES})
	else()
		find_package(GTK3 REQUIRED)
		list(APPEND INCLUDES ${GTK3_INCLUDE_DIRS})
		list(APPEND LIBS ${GTK3_LIBRARIES})
	endif()
endif()

############################################################### Everything SDL2

if(PKG_CONFIG_FOUND)
	pkg_search_module(SDL2 REQUIRED sdl2)
	pkg_search_module(SDL2IMAGE REQUIRED SDL2_image)
	list(APPEND INCLUDES ${SDL2_INCLUDE_DIRS} ${SDL2IMAGE_INCLUDE_DIRS})
	list(APPEND LIBS ${SDL2_LIBRARIES} ${SDL2IMAGE_LIBRARIES})
else()
	find_package(SDL2 REQUIRED)
	find_package(SDL2_image REQUIRED)
	list(APPEND INCLUDES ${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS})
	list(APPEND LIBS ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES})
endif()

############################################################# Everything OpenGL

set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)
list(APPEND INCLUDES ${OPENGL_INCLUDE_DIRS})
list(APPEND LIBS ${OPENGL_LIBRARIES})

################################################################## Build config

add_executable(ansio ${SOURCES})
list(APPEND LIBS pthread)
target_link_libraries(ansio PUBLIC ${LIBS})
target_include_directories(ansio PUBLIC ${PROJECT_BINARY_DIR} ${INCLUDES})

if(CMAKE_BUILD_TYPE STREQUAL Release)
	add_custom_command(
		TARGET ansio POST_BUILD
		COMMAND ${CMAKE_STRIP} ansio
	)
endif()

################################################################ Install config

install(TARGETS ansio
	RUNTIME DESTINATION bin)
#install(FILES "${PROJECT_BINARY_DIR}/ansio-config.h" DESTINATION include)

set(CPACK_GEERATOR "TGZ;DEB")
set(CPACK_PACKAGE_NAME "ansio")
set(CPACK_PACKAGE_VENDOR "Tero Mäyränen")
set(CPACK_PACKAGE_CONTACT "tero@tero.fi")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Ansio is a character mode editor")
#set(CPACK_PACKAGE_DESCRIPTION_FILE "")
#set(CPACK_RESOURCE_FILE_WELCOME "")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_VERSION_MAJOR "${ansio_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${ansio_VERSION_MINOR}")
set(CPACK_SOURCE_GENERATOR "STGZ")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_FILE_NAME}_Source.tar.gz")
set(CPACK_SOURCE_IGNORE_FILES "/build/;.git*")
set(CPACK_SOURCE_PACKAGE_FILE_NAME
	"${CMAKE_PROJECT_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}")
set(CPACK_VERBATIM_VARIABLES true)

set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS true)
include (CPack)

###############################################################################
