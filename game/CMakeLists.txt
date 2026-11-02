add_executable(rpg src/main.cpp)
target_link_libraries(rpg PRIVATE rpg_core raylib)
target_compile_definitions(rpg PRIVATE DATA_DIR="${CMAKE_SOURCE_DIR}/data")