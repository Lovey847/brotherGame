# Memory test
add_executable(mem)

# Set C++ standard
message(VERBOSE "Setting C++ standard to C++11")
set_target_properties(mem PROPERTIES
	CXX_STANDARD 11
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
)

target_include_directories(mem PRIVATE "${CMAKE_SOURCE_DIR}/src")
target_include_directories(mem PRIVATE "${CMAKE_SOURCE_DIR}/src/plat")
target_include_directories(mem PRIVATE "${CMAKE_BINARY_DIR}")
target_include_directories(mem PRIVATE "${CMAKE_SOURCE_DIR}/tests")

target_sources(mem PRIVATE
	"${CMAKE_SOURCE_DIR}/tests/mem.cpp"
	"${CMAKE_SOURCE_DIR}/src/mem.cpp"
	"${CMAKE_SOURCE_DIR}/src/log.cpp"
)

add_test(NAME memTest COMMAND mem)
add_custom_command(TARGET mem COMMENT "Run memory test" POST_BUILD WORKING_DIRECTORY "${CMAKE_BINARY_DIR}" COMMAND ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION> -R "^mem$" --output-on-failures)
