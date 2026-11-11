#include <iostream>
#include "Game.h"
#include "LevelScene.h"

int main() {
    std::cout
        << "==================" << std::endl
        << "RPG project --- by Daniel Gonzalez" << std::endl
        << "==================" << std::endl;

    Game game(1500, 1280, "RPG project");
    game.SetScene(std::make_unique<LevelScene>(0));
    return game.Run() ? 0 : 1;
}