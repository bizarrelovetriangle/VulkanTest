#undef max
#define STBI_MSC_SECURE_CRT
#include "../Dependencies/tiny_gltf/tiny_gltf.h"
#include "../Primitives/RenderObject.h"
#include "../Math/Vector2.hpp"
#include "../Math/Vector3.hpp"
#include "../Math/Matrix4.hpp"
#include <string>
#include <optional>

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

		for (auto& node : glTFModel.nodes)
		{
			if (node.mesh < 0) continue;
			auto& mesh = glTFModel.meshes[node.mesh];

			auto& renderObject = *renderObjects.emplace_back(std::make_unique<RenderObject>());
			renderObject.name = node.name;
			renderObject.model = ComposeMatrix(node);

			for (auto& primitive : mesh.primitives)
			{
				Vector3<uint16_t> vec;

				auto positionsId = primitive.attributes.at(Position);
				auto nomalsId = primitive.attributes.at(Normal);
				//auto textureCoordsId = primitive.attributes.at(TextureCoord);

				auto indexes = ReadBuffer<uint32_t>(primitive.indices);
				auto positions = ReadBuffer<Vector3f>(positionsId);
				auto normals = ReadBuffer<Vector3f>(nomalsId);
				//auto textureCoords = ReadBuffer<Vector2f>(textureCoordsId);

				for (auto index : indexes)
				{
					RenderObjectVertexData vertexData
					{
						.position = positions[index],
						.normal = normals[index],
						//.textureCoord = textureCoords[index]
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
		size_t componentSize = tinygltf::GetComponentSizeInBytes(accessor.componentType);
		size_t componentCount = tinygltf::GetNumComponentsInType(accessor.type);
		size_t elementSize = componentSize * componentCount;
		size_t offset = bufferView.byteOffset + accessor.byteOffset;

		auto& buffer = glTFModel.buffers[bufferView.buffer];
		std::vector<std::byte> data(accessor.count * elementSize);

		if (bufferView.byteStride == 0) bufferView.byteStride = elementSize;

		for (size_t i = 0; i < accessor.count; ++i)
		{
			size_t dataOffset = elementSize * i;
			size_t bufferOffset = offset + bufferView.byteStride * i;
			std::memcpy(data.data() + dataOffset, buffer.data.data() + bufferOffset, elementSize);
		}

		return TypeMarshal<T>(accessor.componentType, accessor.type, data);
	}

	template <class O>
	std::vector<O> TypeMarshal(size_t componentType, size_t componentCount, const std::vector<std::byte>& buffer)
	{
		if (componentCount == TINYGLTF_TYPE_VEC2)
			return InnerTypeMarshal<Vector2, O>(componentType, buffer);
		if (componentCount == TINYGLTF_TYPE_VEC3)
			return InnerTypeMarshal<Vector3, O>(componentType, buffer);
		if(componentCount == TINYGLTF_TYPE_SCALAR)
			return InnerTypeMarshal<Scalar, O>(componentType, buffer);
		throw std::exception("gosh");
	}

	template <template<class> class I, class O>
	std::vector<O> InnerTypeMarshal(size_t componentType, const std::vector<std::byte>& buffer)
	{
		if (componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
			return Helper<I<uint16_t>, O>(buffer);
		if (componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
			return Helper<I<uint32_t>, O>(buffer);
		if (componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
			return Helper<I<float>, O>(buffer);
		if (componentType == TINYGLTF_COMPONENT_TYPE_DOUBLE)
			return Helper<I<double>, O>(buffer);
		throw std::exception("gosh");
	}

	template <class I, class O>
	std::vector<O> Helper(const std::vector<std::byte>& buffer)
	{
		std::vector<I> bufferData(buffer.size() / sizeof(I));
		std::memcpy(bufferData.data(), buffer.data(), buffer.size());
		if constexpr (std::is_convertible_v<I, O>)
			return std::vector<O>(std::begin(bufferData), std::end(bufferData));
		throw std::exception("gosh");
	}

	template<class T>
	struct Scalar
	{
		T value;
		operator T() const { return value; }
	};

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
