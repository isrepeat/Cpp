cmake_minimum_required(VERSION 3.10.0)

project (Linux_Test.App VERSION 1.1)

message("\nVariables:")
cmake_print_variables(PROJECT_NAME)
cmake_print_variables(PROJECT_SOURCE_DIR)
cmake_print_variables(PROJECT_BINARY_DIR)
cmake_print_variables(${PROJECT_NAME}_VERSION)
message("")

add_executable (${PROJECT_NAME}
	# .h
	# .cpp
	"main.cpp"
)

target_include_directories(${PROJECT_NAME} PUBLIC
	#"${PROJECT_BINARY_DIR}"
)