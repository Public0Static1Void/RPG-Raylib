#pragma once
#include <memory>

class Scene
{
public:
	virtual ~Scene() = default;

	virtual bool Start() = 0;
	virtual void Update(float deltaTime) = 0;
	virtual void Render() = 0;

	// Returns a new scene to change to if overwrited
	virtual std::unique_ptr<Scene> NextScene() { return nullptr; }
};