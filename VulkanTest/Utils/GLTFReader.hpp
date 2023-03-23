#undef max
#define STBI_MSC_SECURE_CRT
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "tiny_gltf/tiny_gltf.h"

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
			auto& renderObject = renderObjects.emplace(node.name, RenderObject{ .name = node.name }).first->second;

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
						.normal = positions[index],
						.textureCoord = textureCoords[index]
					};

					renderObject.vertexData.push_back(vertexData);
				}
			}
		}
	}

	std::map<std::string, RenderObject> renderObjects;

private:
	template <class T>
	std::vector<T> ReadBuffer(int accessorId)
	{
		auto& accessor = glTFModel.accessors[accessorId];
		auto& bufferView = glTFModel.bufferViews[accessor.bufferView];
		int elementSize = tinygltf::GetComponentSizeInBytes(accessor.componentType) *
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

	tinygltf::Model glTFModel;
};
