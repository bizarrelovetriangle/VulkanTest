#undef max
#define STBI_MSC_SECURE_CRT
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "tiny_gltf/tiny_gltf.h"
#include "../Math/Matrix4.hpp"

namespace
{
	const std::string Position = "POSITION";
	const std::string Normal = "NORMAL";
	const std::string TextureCoord = "TEXCOORD_0";
}

class GLTFReader
{
public:
	GLTFReader(const std::string& path)
	{
		tinygltf::TinyGLTF gltfContext;
		std::string error, warning;

		bool fileLoaded = gltfContext.LoadASCIIFromFile(&glTFModel, &error, &warning, path);

		auto& scene = glTFModel.scenes.front();

		for (auto nodeId : scene.nodes)
		{
			auto& node = glTFModel.nodes[nodeId];
			auto& mesh = glTFModel.meshes[node.mesh];

			auto& renderObject = *renderObjects.emplace_back(std::make_unique<RenderObject>());
			renderObject.name = node.name;
			renderObject.model = ComposeMatrix(node);

			for (auto& primitive : mesh.primitives)
			{
				auto positionsId = primitive.attributes.at(Position);
				auto nomalsId = primitive.attributes.at(Normal);
				auto textureCoordsId = primitive.attributes.at(TextureCoord);

				auto indexes = ReadBuffer<uint16_t>(primitive.indices);
				auto positions = ReadBuffer<Vector3f>(positionsId);
				auto normals = ReadBuffer<Vector3f>(nomalsId);
				auto textureCoords = ReadBuffer<Vector2f>(textureCoordsId);

				for (auto index : indexes)
				{
					RenderObjectVertexData vertexData
					{
						.position = positions[index],
						.normal = normals[index],
						.textureCoord = textureCoords[index]
					};

					renderObject.vertexData.push_back(vertexData);
				}
			}
		}
	}

	std::vector<std::unique_ptr<RenderObject>> renderObjects;

private:
	Matrix4 ComposeMatrix(const tinygltf::Node& node)
	{
		auto translation = GetVector<Vector3f>(node.translation);
		auto scale = GetVector<Vector3f>(node.scale);
		auto rotation = GetVector<Vector4f>(node.rotation);

		Matrix4 matrix;
		if (scale) matrix = Matrix4::Scale(*scale) * matrix;
		if (rotation) matrix = Matrix4::Rotate(*rotation) * matrix;
		if (translation) matrix = Matrix4::Translation(*translation) * matrix;
		return matrix;
	}

	template <class T>
	std::vector<T> ReadBuffer(int accessorId)
	{
		auto& accessor = glTFModel.accessors[accessorId];
		auto& bufferView = glTFModel.bufferViews[accessor.bufferView];
		int elementSize =
			tinygltf::GetComponentSizeInBytes(accessor.componentType) *
			tinygltf::GetNumComponentsInType(accessor.type);
		int offset = bufferView.byteOffset + accessor.byteOffset;

		if (elementSize != sizeof(T)) throw std::exception();
		auto& buffer = glTFModel.buffers[bufferView.buffer];
		std::vector<T> data(accessor.count);

		if (bufferView.byteStride == 0)
		{
			std::memcpy(data.data(), buffer.data.data() + offset, elementSize * accessor.count);
		}
		else
		{
			for (int i = 0; i < accessor.count; ++i)
			{
				int dataOffset = elementSize * i;
				int bufferOffset = offset + bufferView.byteStride * i;
				std::memcpy(data.data() + dataOffset, buffer.data.data() + bufferOffset, elementSize);
			}
		}

		return data;
	}

	template <class T>
	std::optional<T> GetVector(const std::vector<double> list)
	{
		if (list.empty()) return std::nullopt;
		if constexpr (std::is_same_v<T, Vector4f>) return T(list[0], list[1], list[2], list[3]);
		else if constexpr (std::is_same_v<T, Vector3f>) return T(list[0], list[1], list[2]);
		else throw std::exception();
	}

	tinygltf::Model glTFModel;
};
