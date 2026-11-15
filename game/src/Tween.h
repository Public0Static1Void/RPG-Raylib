#pragma once
#include <algorithm>
#include <functional>
#include <vector>

using EaseFn = float(*)(float);

namespace Easing {
	inline float Linear(float t) { return t; }
	inline float OutQuad(float t) { return 1.0f - (1.0f - t) * (1.0f - t); }
	inline float OutBack(float t) {
		const float c1 = 1.70158f, c3 = c1 + 1.0f;
		float u = t - 1.0f;
		return 1.0f + c3 * u * u * u + c1 * u * u;
	}
}

struct Tween {
	float start = 0, end = 0;
	float duration = 1, timer = 0;
	EaseFn ease = Easing::OutQuad;
	bool active = false;
	bool returns_to_start = false;
	bool just_finished = false;
	std::vector<std::function<void()>> on_end;

	void Begin(float from, float to, float dur, EaseFn fn, bool returns = false) {
		start = from;
		end = to;
		duration = dur;
		timer = 0;
		ease = fn;
		returns_to_start = returns;
		just_finished = false;
		on_end.clear();

		active = true;
	}

	void AddEnd(std::function<void()> fn) {
		if (fn) on_end.push_back(std::move(fn));
	}

	void FireEnd() {
		just_finished = false;
		auto events = std::move(on_end);
		on_end.clear();
		for (auto& fn : events)
			if (fn) fn();
	}

	float Update(float deltaTime) {
		timer += deltaTime;
		float t = timer / duration;
		if (t >= 1.0f) {
			active = false;
			just_finished = true;
			return returns_to_start ? start : end;
		}
		t = ease(t);
		if (returns_to_start) t = 4.0f * t * (1.0f - t);
		return start + (end - start) * t;
	}
};