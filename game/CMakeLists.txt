add_executable(rpg src/main.cpp 
	src/LevelScene.h
	src/Game.cpp
	src/Grid.cpp
)

target_link_libraries(rpg PRIVATE raylib)
add_custom_command(TARGET rpg POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:rpg>/assets
)
target_compile_definitions(rpg PRIVATE ASSETS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets/")