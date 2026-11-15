#pragma once
#include "Scene.h"

#include "Grid.h"
#include "Assets.h"
#include "CameraUtils.h"
#include "Object.h"
#include "ObjectHandler.h"
#include "ObjectRenderer.h"
#include "SoundManager.h"

#include "ObjectControllers.h"

#include "MouseActions.h"

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

		SpawnObjectsFromPositions(positions);

		return true;
	}

	void Update(float deltaTime) override {
		auto& objects = objectHandler.Objects();

		MouseActions::Action mouseAction;
		if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
			Vector2 mouse_pos = GetScreenToWorld2D(GetMousePosition(), camera);
			mouseAction = { mouse_pos, MouseActions::Type::MOVE };
		}

		for (auto& ob : objects) {
			ob->Decide(player_ptr->objectTransform.position, objectHandler, mouseAction);
			ob->Act(objects);
			ob->UpdateMovement(deltaTime);
		}

		objectHandler.EndTurn();
	}

	void Render() override {
		camera = MakeFitCamera(grid.GetBounds());

		BeginMode2D(camera);

		grid.DrawGrid(assets);

		ObjectRenderer::DrawAllObjects(objectHandler.Objects(), assets, grid);

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

	Object* player_ptr;

	ObjectHandler objectHandler;
	SoundManager soundManager;

	void SpawnObjectsFromPositions(std::vector<Grid::ObjectPosition>& positions) {
		for (int i = 0; i < positions.size(); i++) {
			Object* ob = nullptr;
			switch (positions[i].type) {
				case EntityType::PLAYER:
				{
					auto& player = objectHandler.Spawn<PlayerController>(&grid, 3);
					player_ptr = &player; // Store the player ptr
					ob = &player;
					break;
				}
			}
			if (ob != nullptr) {
				if (!ob->on_move) ob->on_move = [this]() { soundManager.PlayStoredSound(SoundManager::SoundType::MOVE); };
				if (!ob->on_rotate) ob->on_rotate = [this]() { soundManager.PlayStoredSound(SoundManager::SoundType::ROTATE); };
				if (!ob->on_hit) ob->on_hit = [this]() { soundManager.PlayStoredSound(SoundManager::SoundType::HIT); };

				if (ob->IsInteractabe()) ob->SetMoves(9);

				ob->objectTransform.position = positions[i].position * grid.tile_size;
				ob->objectTransform.size = Vector2{ static_cast<float>(grid.tile_size), static_cast<float>(grid.tile_size) };
			}
		}
	}
};