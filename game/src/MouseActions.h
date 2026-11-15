#pragma once
#include <raymath.h>

namespace MouseActions {
	enum Type { MOVE, NONE };

	struct Action {
		Vector2 position{};
		Type type = Type::NONE;
		bool IsActive() {
			return type != Type::NONE;
		}
	};
}