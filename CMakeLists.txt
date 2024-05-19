cmake_minimum_required(VERSION 3.27)
project(Regolith VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(BUILD_SHARED_LIBS "Build using shared libraries" OFF)


# Import Bedrock
add_subdirectory(extern/bedrock)

if(WIN32 AND BUILD_SHARED_LIBS AND (MSVC OR CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
add_definitions(
        ${BEDROCK_DEFINITIONS}
        -DREGOLITH_DLL=1
)
endif ()

if (BUILD_SHARED_LIBS)
    set(LIBRARY_TYPE SHARED)
else()
    set(LIBRARY_TYPE STATIC)
endif()

add_library(Regolith STATIC
        src/world.cpp
        src/zone.cpp
        src/id_generator.cpp
        src/player.cpp
        src/entity.cpp
        ${BEDROCK_FILES}
)
target_include_directories(Regolith PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/extern/bedrock/include
)
target_link_libraries(Regolith PUBLIC ${BEDROCK_LIBS})

#Install in binary dir
#set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/${PROJECT_NAME}-${PROJECT_VERSION}")
#Install straight to godot
set(CMAKE_INSTALL_PREFIX  "C:/Users/aruem/Desktop/godot-4.2.2-stable/modules/TEST_MODULE/${PROJECT_NAME}-${PROJECT_VERSION}")

install(TARGETS Regolith
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/static)
install(DIRECTORY include/ DESTINATION include)
install(DIRECTORY ${BEDROCK_INCLUDE_DIR}/ DESTINATION include)

