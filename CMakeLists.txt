cmake_minimum_required(VERSION 3.12)
project(kivitree)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

# Source files (excluding test and main.cpp from reuse)
file(GLOB_RECURSE CORE_SRC "src/kivitree_core/*.cpp")
file(GLOB_RECURSE PAXOS_SRC "src/kivitree_paxos/*.cpp")
file(GLOB_RECURSE RPC_SRC "src/kivitree_rpc/*.cpp")
file(GLOB_RECURSE SERVER_SRC "src/kivitree_server/*.cpp")
file(GLOB_RECURSE KIVILEAF "src/kivileaf/*.cpp")

# Main production binary
add_executable(kivitree
    ${CORE_SRC}
    ${PAXOS_SRC}
    ${RPC_SRC}
    # ${SERVER_SRC}
    ${KIVILEAF}
    src/main.cpp
)

# Paxos Node app
add_executable(paxos_node
    ${CORE_SRC}
    ${PAXOS_SRC}
    ${RPC_SRC}
    src/main.cpp
)

target_link_libraries(paxos_node pthread)

# KiViTree server (handles incoming requests)
# add_executable(kivitree_server
#     ${CORE_SRC}
#     ${SERVER_SRC}
#     ${RPC_SRC}
# )