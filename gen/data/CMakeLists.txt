# Create dataGen
add_executable(dataGen)

target_include_directories(dataGen PRIVATE
	"${CMAKE_SOURCE_DIR}/src"
	"${CMAKE_BINARY_DIR}"
)
target_sources(dataGen PRIVATE
	"${CMAKE_SOURCE_DIR}/gen/data/main.cpp"
	"${CMAKE_SOURCE_DIR}/src/str.cpp"
	"${CMAKE_SOURCE_DIR}/gen/data/log.cpp"
)

add_custom_target(dataGen_run
	COMMAND dataGen "images/xor.img" "rooms/test.rm" "${CMAKE_BINARY_DIR}/data.pak"
	WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/gen/data/files"
	COMMENT "Generating data..."
	SOURCES "${CMAKE_SOURCE_DIR}/gen/data/main.cpp"
)

# App depends on dataGen's execution
add_dependencies(app dataGen_run)
