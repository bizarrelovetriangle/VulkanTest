#pragma once;
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>
#include <functional>
#include "../Math/Vector3.hpp"
#include "../Math/Vector2.hpp"
#include "../Primitives/RenderObject.h"

using namespace std::placeholders;

struct FaceData
{
	struct FaceVertexData
	{
		int vertexId = 0;
		int textureId = 0;
		int normalId = 0;
	};

	FaceVertexData a;
	FaceVertexData b;
	FaceVertexData c;
};

struct Obj3DObjectSerialized
{
	std::string name;
	std::vector<FaceData> faces;
};

struct Obj3DObjectSerializedSet
{
	std::vector<Vector3f> points;
	std::vector<Vector3f> normals;
	std::vector<Vector2f> texturePositions;

	std::vector<Obj3DObjectSerialized> obj3DObjectSerializeds;
};

class ObjSerializer
{
public:
	static RenderObject Deserialize(
		const Obj3DObjectSerializedSet obj3DObjectSerializedSet,
		const Obj3DObjectSerialized& obj3DObjectSerialized)
	{
		RenderObject obj3DObject;
		obj3DObject.name = obj3DObjectSerialized.name;

		for (auto& face : obj3DObjectSerialized.faces)
		{
			for (auto faceVertexData : {face.a, face.b, face.c})
			{
				RenderObjectVertexData vertexData
				{
					.position = obj3DObjectSerializedSet.points[faceVertexData.vertexId],
					.normal = obj3DObjectSerializedSet.normals[faceVertexData.normalId],
					.textureCoord = obj3DObjectSerializedSet.texturePositions[faceVertexData.textureId]
				};

				obj3DObject.vertexData.push_back(vertexData);
			}
		}

		return obj3DObject;
	}
};

class ObjReader
{
public:
	ObjReader(const std::string& path)
	{
		auto lines = ReadFile(path);

		for (auto& line : lines) {
			std::string commandStr;
			auto commandType = ExtractCommand(line, commandStr);
			if (commandType == CommandType::None) continue;
			auto& command = commands.at(commandType);
			command(commandStr);
		}

		for (auto& obj3DObjectSerialized : obj3DObjectSerializedSet.obj3DObjectSerializeds) {
			auto obj3DObject = ObjSerializer::Deserialize(obj3DObjectSerializedSet, obj3DObjectSerialized);
			obj3DObjects.emplace(obj3DObjectSerialized.name, std::move(obj3DObject));
		}
	}

	std::map<std::string, RenderObject> obj3DObjects;

private:
	Obj3DObjectSerializedSet obj3DObjectSerializedSet;

	enum class CommandType { Object, Point, Normal, TexturePosition, Face, Texture, None };

	std::vector<std::pair<std::string, CommandType>> commandNames
	{
		{ "mtllib", CommandType::Texture },
		{ "o", CommandType::Object },
		{ "vn", CommandType::Normal },
		{ "vt", CommandType::TexturePosition },
		{ "v", CommandType::Point },
		{ "f", CommandType::Face }
	};

	std::unordered_map<CommandType, std::function<void(const std::string&)>> commands
	{
		{ CommandType::Object, std::bind(&ObjReader::NewObject, this, _1) },
		{ CommandType::Point, std::bind(&ObjReader::AddPoint, this, _1) },
		{ CommandType::Normal, std::bind(&ObjReader::AddNormal, this, _1) },
		{ CommandType::TexturePosition, std::bind(&ObjReader::AddTexturePosition, this, _1) },
		{ CommandType::Face, std::bind(&ObjReader::AddFace, this, _1) },
		{ CommandType::Texture, std::bind(&ObjReader::AddTexture, this, _1) },
	};

	Obj3DObjectSerialized* current3DObjectSerialized = nullptr;

	void NewObject(const std::string& str)
	{
		current3DObjectSerialized =
			&obj3DObjectSerializedSet.obj3DObjectSerializeds.emplace_back(Obj3DObjectSerialized());
		current3DObjectSerialized->name = str;
	}

	void AddPoint(const std::string& str)
	{
		auto segments = Split<float>(str, " ");
		Vector3 position(segments[0], segments[1], segments[2]);
		obj3DObjectSerializedSet.points.push_back(position);
	}

	void AddNormal(const std::string& str)
	{
		auto segments = Split<float>(str, " ");
		Vector3 normal(segments[0], segments[1], segments[2]);
		obj3DObjectSerializedSet.normals.push_back(normal);
	}

	void AddTexturePosition(const std::string& str)
	{
		auto segments = Split<float>(str, " ");
		Vector2 texturePos(segments[0], segments[1]);
		obj3DObjectSerializedSet.texturePositions.push_back(texturePos);
	}

	void AddTexture(const std::string& str)
	{

	}

	void AddFace(const std::string& str)
	{
		auto segments = Split<std::string>(str, " ");
		std::vector<FaceData::FaceVertexData> vertexDatas;
		std::transform(std::begin(segments), std::end(segments), std::back_inserter(vertexDatas),
			[this](auto& str)
			{
				auto indexes = Split<int>(str, "/");
				return FaceData::FaceVertexData
				{
					.vertexId = indexes[0] - 1,
					.textureId = indexes[1] - 1,
					.normalId = indexes[2] - 1
				};
			});
		
		FaceData faceData
		{
			.a = vertexDatas[0],
			.b = vertexDatas[1],
			.c = vertexDatas[2]
		};
		current3DObjectSerialized->faces.push_back(faceData);
	}

private:
	CommandType ExtractCommand(const std::string& str, std::string& commandStr)
	{
		for (auto& [name, command] : commandNames) {
			if (str.starts_with(name)) {
				commandStr = str.substr(name.length() + 1);
				return command;
			}
		}

		return CommandType::None;
	}

	std::vector<std::string> ReadFile(const std::string& path)
	{
		std::ifstream file(path);
		file.exceptions(std::istream::badbit);
		std::vector<std::string> lines;
		for (std::string line; std::getline(file, line);) lines.push_back(line);
		return lines;
	}

	template <class T>
	std::vector<T> Split(const std::string& str, const std::string delimeter)
	{
		std::vector<T> segments;
		size_t last = 0;

		while (true)
		{
			size_t index = str.find(delimeter, last);
			if (index == std::string::npos) break;
			auto segment = str.substr(last, index - last);
			segments.push_back(Parse<T>(segment));
			last = index + delimeter.length();
		}

		if (last != str.length()) segments.push_back(Parse<T>(str.substr(last)));
		return segments;
	}

	template <class T>
	T Parse(const std::string& str)
	{
		if constexpr (std::is_same<T, float>::value) {
			return std::stod(str);
		}
		else if constexpr (std::is_same<T, int>::value) {
			return std::stoi(str);
		}
		else if constexpr (std::is_same<T, std::string>::value) {
			return str;
		}
		else {
			throw std::exception();
		}
	}
};
