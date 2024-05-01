cmake_minimum_required(VERSION 3.27)
project(regolith)

set(CMAKE_CXX_STANDARD 17)

#add_library(regolith STATIC main.cpp)
# Import Bedrock
add_subdirectory(extern/bedrock)
include_directories(extern/bedrock/include)

add_executable(regolith_test
        main.cpp
        src/world.cpp
        src/zone.cpp
        src/id_generator.cpp
        src/player.cpp
        src/entity.cpp
)
target_include_directories(regolith_test PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(regolith_test PRIVATE Bedrock)

