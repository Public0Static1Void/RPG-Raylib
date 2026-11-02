add_library(rpg_core STATIC
    src/dice.cpp
)
target_include_directories(rpg_core PUBLIC include)
target_link_libraries(rpg_core PUBLIC nlohmann_json::nlohmann_json)

if (MSVC)
    target_compile_options(rpg_core PRIVATE /W4)
else()
    target_compile_options(rpg_core PRIVATE -Wall -Wextra)
endif()