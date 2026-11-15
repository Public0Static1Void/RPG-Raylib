#pragma once
#include "Utensils.h"
#include "Grid.h"
#include "ObjectHandler.h"
#include "Assets.h"
#include "Animator.h"
#include "SoundManager.h"
#include "MouseActions.h"

#include <raymath.h>
#include <iostream>
#include <string>
#include <optional>
#include <functional>
#include <algorithm>
#include <deque>

enum class Facing { UP, DOWN, RIGHT, LEFT };

struct Action {
	enum class Type { MOVE, ROTATE, TRANSLATE, NONE } type = Type::NONE;
	Vector2 dir{};
	std::vector<Vector2> path;
	int degrees = 0; // For rotations
};
class Object;

class LifeSystem {
public:
	LifeSystem(int max_life) : max_life(max_life), life(max_life) {}
	virtual ~LifeSystem() = default;

	void Damage(int amount) {
		life -= amount;
		if (life <= 0) Die();
	}
	void SetLife(int amount) {
		if (amount > max_life) amount = max_life;
		life = amount;
		if (life <= 0) Die();
	}
	virtual void Die() { if (on_die) on_die(); }

	const int GetMaxLife() const { return max_life; }

	bool IsAlive() const { return life > 0; }

	std::function<void()> on_die;

private:
	int max_life = 3, life = 3;
};

class Object {
public:
	Object(bool isPlayer, EntityType type, Grid* grid, int max_life)
		: isPlayer(isPlayer), type(type), grid(grid), lifeSystem(max_life)
	{
	}
	virtual ~Object() = default;

	virtual Action Decide(const Vector2& player_pos, ObjectHandler& objectHandler, MouseActions::Action mouseAction) = 0;
	virtual void Act(std::vector<std::unique_ptr<Object>>& objects) {
		if (anim.IsAnimating() || !path_queue.empty()) return;

		if (queued_action.type != Action::Type::NONE) {
			current_action = queued_action;
			queued_action.type = Action::Type::NONE;
		}

		if (current_action.type == Action::Type::NONE) return;

		bool action_succeeded = false;

		switch (current_action.type) {
		case Action::Type::MOVE: {
			Vector2 new_pos = objectTransform.position + current_action.dir * (grid->tile_size / 2);
			Vector2 snapped_pos = Grid::SnapToTile(new_pos, grid->tile_size);
			Vector2 move_dir = new_pos - objectTransform.position;

			CollisionInfo hit = ObjectHandler::ObjectCollision(new_pos, *this, objects);
			if (hit.collided && hit.collided_object && hit.collided_object->type != EntityType::BULLET) {
				OnCollision(*hit.collided_object);
				hit.collided_object->OnHit(*this);
			}
			else if (grid->IsWalkable(snapped_pos, type != EntityType::BULLET)) {
				objectTransform.position += current_action.dir * 2;
				action_succeeded = true;
			}

			// Rotate the object to face the movement direction
			AnimateRotation(ObjectTransform::RotationFromDir(current_action.dir), DEFAULT_MOVE_DURATION, Easing::OutQuad);

			if (on_move) on_move();
			break;
		}
		case Action::Type::ROTATE: {
			AnimateRotation(objectTransform.rotation + current_action.degrees, DEFAULT_MOVE_DURATION, Easing::OutQuad);

			if (on_rotate) on_rotate();

			action_succeeded = true;

			break;
		}
		case Action::Type::TRANSLATE: {
			path_queue.clear();
			// BFS path is stored end -> start, so walk it from the back
			for (size_t i = current_action.path.size(); i-- > 0;)
				path_queue.push_back(current_action.path[i]);
			StepPath();
			break;}
		}

		// Avoid repetitive actions
		current_action.type = Action::Type::NONE;

		// Remove an action if it's an interactable object
		if (moves_left != -1 && action_succeeded) moves_left--;
	}

	LifeSystem* GetLifeSystem() { return &lifeSystem; }
	const LifeSystem* GetLifeSystem() const { return &lifeSystem; }

	Grid* GetGrid() { return grid; }

	Sprite GetSprite() const { return sprite; }

	void UpdateMovement(float deltaTime) {
		anim.Update(deltaTime, objectTransform);
	}

	void SetPosition(Vector2 pos) {
		if (IsAnimating()) return;
		objectTransform.position = pos;
	}
	void SetPosition(int x, int y) {
		objectTransform.position.x = x;
		objectTransform.position.y = y;
	}
	void SetSize(int size) {
		objectTransform.size.x = size;
		objectTransform.size.y = size;
	}
	void SetSize(int x, int y) {
		objectTransform.size.x = x;
		objectTransform.size.y = y;
	}
	void SetSize(Vector2 size) {
		objectTransform.size.x = size.x;
		objectTransform.size.y = size.y;
	}
	void SetId(int new_id) { id = new_id; }
    int GetId() const { 
		return id;
	}

	virtual void OnCollision(Object& other) {}
	virtual void OnHit(Object& other) {
		if (on_hit) on_hit();
		AnimateSize(objectTransform.size * 1.4f, 0.35f, Easing::OutBack, true);
	}
	virtual void OnWallHit() {}

	void QueueAction(Action& ac) { queued_action = ac; }
	void ExecuteAction(Action& ac, std::vector<std::unique_ptr<Object>>& objects) {
		if (moves_left == -1 || moves_left > 0) {
			if (ac.type == Action::Type::MOVE && Vector2Equals(ac.dir, { 0, 0 })) {
				ac.dir = ObjectTransform::DirFromRotation(objectTransform.rotation);
			}
			queued_action = ac;
			Act(objects);
		}
	}

	void SetInteractable(bool in) { is_interactable = in; moves_left = AVALIABLE_MOVES; }
	bool IsInteractabe() { return is_interactable; }

	void SetMoves(int moves) { moves_left = moves; }
	const int GetMoves() const { return moves_left; }

	void AnimateTo(Vector2 target_pos, float duration, EaseFn fn, bool returns = false, std::function<void()> end_event = nullptr) {
		if (!anim.IsMoving())
			anim.MoveTo(objectTransform.position, target_pos, duration, fn, returns, end_event);
	}
	void AnimateSize(Vector2 target_size, float duration, EaseFn fn, bool returns = false, std::function<void()> end_event = nullptr) {
		if (!anim.IsScaling())
			anim.ScaleTo(objectTransform.size, target_size, duration, fn, returns, end_event);
	}
	void AnimateRotation(float target_rotation, float duration, EaseFn fn, bool returns = false, std::function<void()> end_event = nullptr) {
		if (!anim.IsRotating())
			anim.RotateTo(objectTransform.rotation, target_rotation, duration, fn, returns, end_event);
	}

	bool IsAnimating() { return anim.IsAnimating(); }

	virtual void Interact(int override_value = -1) {}

	ObjectTransform objectTransform;
	bool isPlayer = false;
	EntityType type;

	static constexpr int AVALIABLE_MOVES = 3;

	std::function<void()> on_hit, on_move, on_rotate;

	const bool GetCanRender() const { return can_render; }

protected:
	Action current_action, queued_action;
	Grid* grid;

	Action SetUpActionType(Vector2 dir) {
		Action ac;
		ac.dir = dir;
		ac.type = Vector2Equals(dir, Vector2{ 0, 0 })
			? Action::Type::NONE : Action::Type::MOVE;
		return ac;
	}

	void SetCanCollide(bool v) { can_collide = v; }
	const bool GetCanCollide() const { return can_collide; }
	void SetCanRender(bool v) { can_render = v; }

	Sprite sprite = Sprite::LAST_NO_USE;
	
private:
	int id = -1;
	bool is_interactable = false;
	int moves_left = -1;

	const float DEFAULT_MOVE_DURATION = 0.25f;
	Animator anim;

	bool can_collide = true, can_render = true;

	LifeSystem lifeSystem;

	std::deque<Vector2> path_queue;

	void StepPath() {
		if (path_queue.empty()) return;
		Vector2 target = path_queue.front();
		path_queue.pop_front();
		AnimateTo(target, DEFAULT_MOVE_DURATION, Easing::Linear, false,
			[this]() { StepPath(); });
	}
};