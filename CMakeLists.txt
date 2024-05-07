cmake_minimum_required(VERSION 3.27)
project(regolith VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)

set(BUILD_SHARED_LIBS ON)

# Import Bedrock
add_subdirectory(extern/bedrock)

add_definitions(
        ${BEDROCK_DEFINITIONS}
        -DREGOLITH_DLL=1
)

add_library(regolith
        src/world.cpp
        src/zone.cpp
        src/id_generator.cpp
        src/player.cpp
        src/entity.cpp
        ${BEDROCK_FILES}
)
target_include_directories(regolith PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/extern/bedrock/include
)
target_link_libraries(regolith PRIVATE ${BEDROCK_LIBS})

set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/${PROJECT_NAME}-${PROJECT_VERSION}")

install(TARGETS regolith
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/static)
install(DIRECTORY include/ DESTINATION include)
install(DIRECTORY ${BEDROCK_INCLUDE_DIR}/ DESTINATION include)

