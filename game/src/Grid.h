#pragma once
#include <iostream>
#include <vector>
#include <raylib.h>
#include <string>
#include <cmath>

class Assets;

enum class EntityType { PLAYER, ENEMY, CANNON, CRATE, BULLET, TARGET, DOOR, NONE };

class Grid
{
public:
	bool ScanLevel(const char* fileName);

	void DrawGrid(const Assets& assets);

	void SetSize(int tileSize) {
		tile_size = tileSize;
	}

	struct ObjectPosition {
		Vector2 position{0, 0};
		EntityType type = EntityType::NONE;
	};

	const std::vector<ObjectPosition>& GetStartPositions() const {
		return objects_start_positions;
	}

	const Vector2 GetEntityPosition(EntityType type) const {
		for (int i = 0; i < objects_start_positions.size(); i++) {
			if (objects_start_positions[i].type == type) {
				return objects_start_positions[i].position;
			}
		}
		return { 0, 0 };
	}

	const Vector2 GetGridOffset() const { return start_draw_position; }

	Vector2 SnapToTile(Vector2 p) const {
		return {
			roundf(p.x / tile_size) * tile_size,
			roundf(p.y / tile_size) * tile_size
		};
	}
	static Vector2 SnapToTile(Vector2 p, float size) {
		return {
			roundf(p.x / size) * size,
			roundf(p.y / size) * size
		};
	}

	Vector2 PointToTilePos(Vector2 p) const {
		const float lx = p.x - start_draw_position.x;
		const float ly = p.y - start_draw_position.y;
		return {
			floorf(lx / tile_size) * tile_size,
			floorf(ly / tile_size) * tile_size
		};
	}
	Vector2 PointToTileCenter(Vector2 p) const {
		const float lx = p.x - start_draw_position.x;
		const float ly = p.y - start_draw_position.y;
		const float offset = tile_size + tile_size * 0.5f;
		return {
			start_draw_position.x + floorf(lx / tile_size) * offset,
			start_draw_position.y + floorf(ly / tile_size) * offset
		};
	}

	bool IsWalkable(Vector2 position, bool terrestrial = true) {
		int x = static_cast<int>(position.x / tile_size);
		int y = static_cast<int>(position.y / tile_size);

		if (y >= tiles.size() || y < 0 || x >= tiles[y].size() || x < 0) return false;
		const char tile = tiles[y][x];
		if (terrestrial)
			return tile != '1' && tile != 'H';
		else
			return tile != '1';
	}

	Rectangle GetBounds() const {
		if (tiles.empty()) return { 0, 0, 1, 1 };
		Rectangle bounds = {
			(float)start_draw_position.x, (float)start_draw_position.y,
			(float)(tiles[0].size() * tile_size), (float)(tiles.size() * tile_size)
		};
		return bounds;
	}

	int tile_size = 100;
	std::vector<Vector2> path;
	std::vector<Vector2> searched_tiles;
	Vector2 start_position{}, end_position{};
private:
	Vector2 start_draw_position{ 0, 0 };

	std::vector<std::string> tiles;
	std::vector<ObjectPosition> objects_start_positions;

	int stored_map_size = -1;

	const int MAX_MAP_COLS = 256;
	const int MAX_MAP_ROWS = 256;
};