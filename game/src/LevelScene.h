#pragma once
#include "Scene.h"

#include "Grid.h"
#include "Assets.h"
#include "CameraUtils.h"

#include <raylib.h>
#include <iostream>
#include <functional>
#include <memory>

using SceneFactory = std::function<std::unique_ptr<Scene>()>;

class LevelScene : public Scene {
public:
	explicit LevelScene(int level_index) : level_index(level_index) {}

	bool Start() override {

		if (!assets.Load(ASSETS_DIR "Sprites/tilemap.png")) return false;

		grid.SetSize(64);
		if (!grid.ScanLevel(ASSETS_DIR "Levels/Level1.txt")) return false;

		std::vector<Grid::ObjectPosition> positions = grid.GetStartPositions();

		return true;
	}

	void Update(float deltaTime) override {

	}

	void Render() override {
		camera = MakeFitCamera(grid.GetBounds());

		BeginMode2D(camera);

		grid.DrawGrid(assets);

		EndMode2D();
	}

	std::unique_ptr<Scene> NextScene() override {
		if (!next_scene_factory) return nullptr;
		return next_scene_factory();
	}
private:
	int level_index = 0;

	std::function<std::unique_ptr<Scene>()> next_scene_factory;

	Grid grid;
	Assets assets;

	Camera2D camera{};
};