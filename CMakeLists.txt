project(SOMCPU)
cmake_minimum_required(VERSION 3.00)

# Root dir
set(PROJECT_ROOT_DIR ${PROJECT_SOURCE_DIR})

# Binaries directory
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_ROOT_DIR})

# Update sources directory
set(PROJECT_SOURCE_DIR ${PROJECT_SOURCE_DIR}/src)

# Include directories
include_directories(
    ${PROJECT_SOURCE_DIR}
)

# Generate Gengetopt files
add_custom_target(run ALL)
# Custom target will always cause its dependencies to be evaluated and is
# run by default
add_custom_target(dummy_target ALL
    DEPENDS
        ${PROJECT_SOURCE_DIR}/cmdline.c ${PROJECT_SOURCE_DIR}/cmdline.h
    )

# custom_output will always be rebuilt because it depends on always_rebuild
add_custom_command(
    OUTPUT ${PROJECT_SOURCE_DIR}/cmdline.c ${PROJECT_SOURCE_DIR}/cmdline.h
    COMMAND gengetopt -i ${PROJECT_SOURCE_DIR}/lineparser.ggo --output-dir=${PROJECT_SOURCE_DIR}
    DEPENDS
        always_rebuild
    )

# Dummy output which is never actually produced. Anything that depends on
# this will always be rebuilt.
add_custom_command(
    OUTPUT always_rebuild
    COMMAND cmake -E echo
    )

# Source files
add_executable(
    SOMCPU
    ${PROJECT_SOURCE_DIR}/SOM.cpp
	${PROJECT_SOURCE_DIR}/cmdline.c
	${PROJECT_SOURCE_DIR}/cmdline.h
	${PROJECT_SOURCE_DIR}/distance_functions.cpp
	${PROJECT_SOURCE_DIR}/distance_functions.h
	${PROJECT_SOURCE_DIR}/update_functions.cpp
	${PROJECT_SOURCE_DIR}/update_functions.h
	${PROJECT_SOURCE_DIR}/utility_functions.cpp
	${PROJECT_SOURCE_DIR}/utility_functions.h
)

# C++11
# target_compile_features(SOMCPU PUBLIC cxx_std_11)

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(SOMCPU PUBLIC OpenMP::OpenMP_CXX)
endif()