#pragma once
#pragma once
#include "../../RenderObjects/Interfaces/RenderObject.h"
#include "../../CAD/MeshModel.h"

class Object
{
public:
	Object()
	{
	}

	Object(std::unique_ptr<RenderObject> renderer)
		: renderer(std::move(renderer))
	{
	}

	void Dispose()
	{
		renderer->Dispose();
	}

	std::string name;

	Vector3f position;
	Vector4f rotation;
	Vector3f velocity;
	Vector4f angularVelocity;

	Vector3f scale;

	std::unique_ptr<RenderObject> renderer;
};
