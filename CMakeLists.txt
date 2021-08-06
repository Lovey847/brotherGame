cmake_minimum_required(VERSION 3.16.2)
project(gameTest CXX)

include(TestBigEndian)

# Enable extra warnings and error on warnings
# in debug builds, also enable tests
if (CMAKE_BUILD_TYPE STREQUAL Debug)
	  message(VERBOSE "Enabling extra warnings, and treating them as errors...")
	  
	  if (CMAKE_CXX_COMPILER_ID STREQUAL Clang OR CMAKE_CXX_COMPILER_ID STREQUAL GNU)
		    add_compile_options(-pedantic -Wall -Wextra -Werror)
		    message(VERBOSE "Added '-pedantic -Wall -Wextra -Werror'")
	  endif ()
	  enable_testing()
endif ()

add_executable(app)

# Set C++ standard
message(VERBOSE "Setting C++ standard to C++11")
set_target_properties(app PROPERTIES
	  CXX_STANDARD 11
	  CXX_STANDARD_REQUIRED ON
	  CXX_EXTENSIONS OFF
    )

# Link libm, if it exists
find_library(FOUND_LIBM m)
if (FOUND_LIBM)
	  message(VERBOSE "libm exists: linking libm")
	  link_libraries(${FOUND_LIBM})
endif ()

# Make config.h
set(PLAT_E_LITTLE OFF)
set(PLAT_E_BIG OFF)

set(PLAT_OS_WIN OFF)
set(PLAT_OS_LINUX OFF)

set(PLAT_C_GNU OFF)
set(PLAT_C_MSVC OFF)

set(PLAT_S_SSE OFF)

set(PLAT_B_OPENGL OFF)

set(PLAT_B_ALSA OFF)

# Project name and version
set(PROJ_NAME "I Wanna Be The C++")
set(PROJ_VER "1.0.0")

# Endian detection
TEST_BIG_ENDIAN(PLAT_E_BIG)
if (NOT PLAT_E_BIG)
	  set(PLAT_E_LITTLE ON)
endif ()

# OS detection
if (WIN32)
	  set(PLAT_OS_WIN ON)
elseif (UNIX)
	  if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
		    set(PLAT_OS_LINUX ON)
	  else ()
		    message(FATAL_ERROR "Can't determine unix system!")
	  endif ()
else ()
	  message(FATAL_ERROR "Unknown OS!")
endif ()

# Complier detection
if (
	      # All compilers with GNU extensions
	      CMAKE_CXX_COMPILER_ID STREQUAL Clang OR
	      CMAKE_CXX_COMPILER_ID STREQUAL AppleClang OR
	      CMAKE_CXX_COMPILER_ID STREQUAL GNU
        )
	  set(PLAT_C_GNU ON)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL MSVC)
	  set(PLAT_C_MSVC ON)
endif ()

# SIMD detection
cmake_host_system_information(RESULT SSE_AVAILABLE QUERY HAS_SSE)
cmake_host_system_information(RESULT SSE2_AVAILABLE QUERY HAS_SSE2)

if (SSE_AVAILABLE AND SSE2_AVAILABLE)
	  set(PLAT_S_SSE2 ON)
endif ()

# Don't detect backends here, some platforms
# could have base GL but not something
# like GLX to actually utilize it

target_include_directories(app PRIVATE
	  "${CMAKE_SOURCE_DIR}/src"
	  "${CMAKE_SOURCE_DIR}/src/plat"
	  "${CMAKE_SOURCE_DIR}/src/plat/dummy"
	  #"${CMAKE_SOURCE_DIR}/src/game"
	  "${CMAKE_BINARY_DIR}")

#target_include_directories(app PRIVATE "${CMAKE_SOURCE_DIR}/src")
#target_include_directories(app PRIVATE "${CMAKE_SOURCE_DIR}/src/plat")
#target_include_directories(app PRIVATE "${CMAKE_SOURCE_DIR}/src/game")
#target_include_directories(app PRIVATE "${CMAKE_BINARY_DIR}")

target_sources(app PRIVATE
	  "${CMAKE_SOURCE_DIR}/src/log.cpp"
	  "${CMAKE_SOURCE_DIR}/src/game/game.cpp"
	  "${CMAKE_SOURCE_DIR}/src/args.cpp"
	  "${CMAKE_SOURCE_DIR}/src/str.cpp"
	  "${CMAKE_SOURCE_DIR}/src/key.cpp"
	  "${CMAKE_SOURCE_DIR}/src/rng.cpp"
    "${CMAKE_SOURCE_DIR}/src/game/pak.cpp"
    "${CMAKE_SOURCE_DIR}/src/game/atlas.cpp"
    "${CMAKE_SOURCE_DIR}/src/vector.cpp"

	  # Interfaces
	  "${CMAKE_SOURCE_DIR}/src/plat/mem.cpp"

	  # Modules
	  "${CMAKE_SOURCE_DIR}/src/plat/audio.cpp"
    )

# Detect backends here
if (PLAT_OS_LINUX)
	  target_include_directories(app PRIVATE "${CMAKE_SOURCE_DIR}/src/plat/linux")
	  find_package(X11 REQUIRED)
	  if (NOT X11_FOUND OR NOT X11_Xfixes_FOUND)
		    message(FATAL_ERROR "Cannot find Xlib!")
	  endif ()
	  
	  # Detect render backend API's
	  # Right now, the only one is opengl
	  # TODO: I'm gonna have to redesign this so that
	  #       there can be multiple render backend API's at once
	  find_package(OpenGL)
	  if (OPENGL_FOUND AND OpenGL_GLX_FOUND)
		    target_include_directories(app PRIVATE "${CMAKE_SOURCE_DIR}/src/plat/gl")
		    target_include_directories(app PRIVATE "${CMAKE_SOURCE_DIR}/src/plat/linux/gl")
		    
		    set(PLAT_B_OPENGL ON)
		    
		    target_sources(app PRIVATE
			      "${CMAKE_SOURCE_DIR}/src/plat/linux/gl/linux_gl_window.cpp"
		        "${CMAKE_SOURCE_DIR}/src/plat/gl/gl_render.cpp"
			      "${CMAKE_SOURCE_DIR}/src/plat/gl/gl_shader.cpp"
			      "${CMAKE_SOURCE_DIR}/src/plat/linux/gl/linux_opengl.cpp"
            "${CMAKE_SOURCE_DIR}/src/plat/gl/gl_texture.cpp"
            "${CMAKE_SOURCE_DIR}/src/plat/gl/gl_buffer.cpp"
		        )
		    target_include_directories(app PRIVATE
			      ${OPENGL_INCLUDE_DIR}
			      ${OPENGL_GLX_INCLUDE_DIR}
		        )
		    target_link_libraries(app
			      OpenGL::GL
			      OpenGL::GLX
		        )
	  else ()
	      message(FATAL_ERROR "No available window module backends!")
	  endif ()

	  # Detect audio backend API
	  # TODO: I'm gonna have to redesign this as well
	  find_package(ALSA)
	  if (ALSA_FOUND)
		    # ALSA uses pthread, get that as well
		    find_package(Threads REQUIRED)
		    
		    if (NOT CMAKE_USE_PTHREADS_INIT)
			      message(FATAL_ERROR "Cannot find pthread!")
		    endif ()
		    
	      target_include_directories(app PRIVATE "${CMAKE_SOURCE_DIR}/src/plat/alsa")

	      set(PLAT_B_ALSA ON)

	      target_sources(app PRIVATE
			      "${CMAKE_SOURCE_DIR}/src/plat/alsa/alsa.cpp"
	          )

		    # API inlcudes and linkage
	      target_include_directories(app PRIVATE ${ALSA_INCLUDE_DIRS})
	      target_link_libraries(app ALSA::ALSA Threads::Threads)
	  else ()
	      message(FATAL_ERROR "No available audio module backends!")
	  endif ()
	  
	  target_sources(app PRIVATE
		    "${CMAKE_SOURCE_DIR}/src/plat/linux/linux_main.cpp"
		    "${CMAKE_SOURCE_DIR}/src/plat/linux/linux_file.cpp"
		    "${CMAKE_SOURCE_DIR}/src/plat/linux/linux_mem.cpp"
		    "${CMAKE_SOURCE_DIR}/src/plat/linux/linux_window.cpp"
		    "${CMAKE_SOURCE_DIR}/src/plat/linux/linux_countTimer.cpp"
	      )
	  target_include_directories(app PRIVATE
		    ${X11_X11_INCLUDE_PATH}
		    ${X11_Xfixes_INCLUDE_PATH}
	      )
	  target_link_libraries(app
		    X11::X11
		    X11::Xfixes
	      )
else ()
	  message(FATAL_ERROR "Unknown platform!")
endif ()

# Create config.h
configure_file("${CMAKE_SOURCE_DIR}/src/config.h.in" "${CMAKE_BINARY_DIR}/config.h")

# Data generation
file(STRINGS "${CMAKE_SOURCE_DIR}/gen/generators.txt" GENERATOR_LIST)

foreach (GENERATOR ${GENERATOR_LIST})
	  # Only run the generator on directories that end with slashes
	  string(LENGTH ${GENERATOR} GENLENGTH)
	  string(FIND ${GENERATOR} "/" GENSLASH REVERSE)
	  math(EXPR GENVALID "${GENLENGTH}-${GENSLASH}")

	  if (GENVALID EQUAL 1)
		    # Run generator in directory
		    string(SUBSTRING ${GENERATOR} 0 ${GENSLASH} GENNAME)
		    string(CONCAT GENTARGET ${GENNAME} "_target")
		    string(CONCAT GENRUNTARGET ${GENTARGET} "_run")
		    
		    message(VERBOSE "Generating ${GENNAME}")

		    add_executable(${GENTARGET})

		    target_include_directories(${GENTARGET} PRIVATE
			      "${CMAKE_SOURCE_DIR}/gen"
			      "${CMAKE_SOURCE_DIR}/src"
			      "${CMAKE_SOURCE_DIR}/src/plat"
            "${CMAKE_SOURCE_DIR}/src/game"
			      "${CMAKE_BINARY_DIR}"
			      )
		    target_sources(${GENTARGET} PRIVATE
			      "${CMAKE_SOURCE_DIR}/gen/${GENNAME}/gen.cpp"
			      "${CMAKE_SOURCE_DIR}/src/plat/mem.cpp"
			      "${CMAKE_SOURCE_DIR}/src/log.cpp"
			      "${CMAKE_SOURCE_DIR}/src/str.cpp"
			      )

		    # Platform layers for interfaces
		    if (PLAT_OS_LINUX)
			      target_include_directories(${GENTARGET} PRIVATE
				        "${CMAKE_SOURCE_DIR}/src/plat/linux"
				        )
			      target_sources(${GENTARGET} PRIVATE
				        "${CMAKE_SOURCE_DIR}/gen/linux.cpp"
				        "${CMAKE_SOURCE_DIR}/src/plat/linux/linux_file.cpp"
				        "${CMAKE_SOURCE_DIR}/src/plat/linux/linux_mem.cpp"
				        "${CMAKE_SOURCE_DIR}/src/plat/linux/linux_countTimer.cpp"
				        )
		    else ()
		    endif()
		    
		    add_custom_target(${GENRUNTARGET}
			      COMMAND ${GENTARGET} "${CMAKE_SOURCE_DIR}/gen/${GENNAME}.txt" "${CMAKE_BINARY_DIR}/"
			      WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/gen/${GENNAME}"
			      COMMENT "Generating ${GENNAME}..."
			      SOURCES "${CMAKE_SOURCE_DIR}/gen/${GENNAME}/gen.cpp"
			      )

		    add_dependencies(app ${GENRUNTARGET})
	  endif ()
endforeach ()

# Print include directories, source files and libraries linked
# Could be helpful in detecting some sort of error
get_property(APP_INCLUDE_DIRECTORIES TARGET app PROPERTY INCLUDE_DIRECTORIES)
get_property(APP_SOURCES TARGET app PROPERTY SOURCES)
get_property(APP_LINK_LIBRARIES TARGET app PROPERTY LINK_LIBRARIES)

string(REPLACE ";" "\n     " APP_INCLUDE_DIRECTORIES "${APP_INCLUDE_DIRECTORIES}")
string(REPLACE ";" "\n     " APP_SOURCES "${APP_SOURCES}")
string(REPLACE ";" "\n     " APP_LINK_LIBRARIES "${APP_LINK_LIBRARIES}")

message(VERBOSE "Source files:\n     ${APP_SOURCES}")
message(VERBOSE "Include directories:\n     ${APP_INCLUDE_DIRECTORIES}")
message(VERBOSE "Libraries linked:\n     ${APP_LINK_LIBRARIES}")
