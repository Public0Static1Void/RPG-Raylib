#include "Object.h"
#include "Pathfinding.h"

class PlayerController : public Object {
public:
	// This class will always be the player
	PlayerController(Grid* grid, int max_life)
		: Object(true, EntityType::PLAYER, grid, max_life)
	{
		sprite = Sprite::PLAYER;
	}

	Action Decide(const Vector2& player_pos, ObjectHandler& objectHandler, MouseActions::Action mouseAction) override {
		Action ac;
		// Stop the logic if the object is dead
		if (!GetLifeSystem()->IsAlive()) return ac;

		Vector2 dir{ 0, 0 };

		if (mouseAction.IsActive()) {
			PathResult result = FindPathBFS(grid->PointToTilePos(objectTransform.position) , grid->PointToTilePos(mouseAction.position) , grid, objectHandler.Objects());

			ac.path = result.path;
			ac.type = Action::Type::TRANSLATE;
		}
		else {
			if (IsKeyDown(KEY_A)) dir.x = -1;
			if (IsKeyDown(KEY_D)) dir.x = 1;
			if (IsKeyDown(KEY_W)) dir.y = -1;
			if (IsKeyDown(KEY_S)) dir.y = 1;

			ac = SetUpActionType(dir);
		}

		current_action = ac;
		return ac;
	}

	void OnCollision(Object& other) override {
		Vector2 dir = ObjectTransform::DirFromRotation(objectTransform.rotation);
		AnimateTo(objectTransform.position + dir * (grid->tile_size / 4), 0.75f, Easing::OutBack, true);
	}
	void OnWallHit() override {
		OnCollision(*this);
	}
};