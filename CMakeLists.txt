cmake_minimum_required(VERSION 3.12)
project(kivitree)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

# Source files (excluding test and main.cpp from reuse)
file(GLOB CORE_SRC "src/hash_store.cpp" "src/lru_store.cpp" "src/kivitree.cpp")
file(GLOB MAIN_SRC "src/main.cpp")
file(GLOB TEST_SRC "tests/kivitree_test.cpp")
file(GLOB SERVER_SRC "src/kivitree_server.cpp" "src/server.cpp")

# Main production binary
add_executable(kivitree ${CORE_SRC} ${MAIN_SRC})

# Test binary — separate main()
add_executable(kivitree_test ${CORE_SRC} ${TEST_SRC})

add_executable(kivitree_server ${CORE_SRC} ${SERVER_SRC})