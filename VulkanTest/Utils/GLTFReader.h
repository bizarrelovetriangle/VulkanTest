#pragma once
#undef max
#define STBI_MSC_SECURE_CRT
#include "../Dependencies/tiny_gltf/tiny_gltf.h"
#include "../Math/Vector2.h"
#include "../Math/Vector3.h"
#include "../Math/Matrix4.h"
#include "../CAD/MeshModel.h"
#include <string>
#include <optional>

namespace
{
	const std::string Position = "POSITION";
	const std::string Normal = "NORMAL";
	const std::string TextureCoord = "TEXCOORD_0";
	const std::string Color = "COLOR_0";
}

struct SerializedObject
{
	std::string name;

	Vector3f scale;
	Vector3f translation;
	Vector4f rotation;

	std::vector<uint32_t> indexes;
	std::vector<Vector3f> positions;
	std::vector<Vector3f> normals;
	std::vector<Vector2f> textureCoords;
	std::vector<Vector4f> colors;
	std::optional<std::pair<Vector2u, std::vector<std::byte>>> textureData;
	Vector4f baseColor;
};

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

			SerializedObject serializedObject;
			serializedObject.name = node.name;

			serializedObject.translation = GetVector<Vector3f>(node.translation)
				.value_or(Vector3f());
			serializedObject.scale = GetVector<Vector3f>(node.scale)
				.value_or(Vector3f(1., 1., 1.));
			serializedObject.rotation = GetVector<Vector4f>(node.rotation)
				.value_or(Vector4f(0., 0., 0., 1.));;

			for (auto& primitive : mesh.primitives)
			{
				serializedObject.indexes = ReadBuffer<uint32_t>(primitive.indices);

				for (auto& [name, index] : primitive.attributes)
				{
					if (name == Position)		serializedObject.positions = ReadBuffer<Vector3f>(index);
					if (name == Normal)			serializedObject.normals = ReadBuffer<Vector3f>(index);
					if (name == TextureCoord)	serializedObject.textureCoords = ReadBuffer<Vector2f>(index);
					if (name == Color) {
						serializedObject.colors = ReadBuffer<Vector4f>(index);
						NormalizeIntegerColors(index, serializedObject.colors);
					}
				}

				std::transform(serializedObject.positions.begin(), serializedObject.positions.end(), serializedObject.positions.begin(),
					[](auto& p) { return Vector3f::FromGLTF(p); });
				std::transform(serializedObject.normals.begin(), serializedObject.normals.end(), serializedObject.normals.begin(),
					[](auto& p) { return Vector3f::FromGLTF(p); });

				if (primitive.material != -1)
				{
					auto& material = glTFModel.materials[primitive.material];
					auto& roughness = material.pbrMetallicRoughness;
					serializedObject.baseColor = *GetVector<Vector4f>(roughness.baseColorFactor);

					if (roughness.baseColorTexture.index != -1)
					{
						auto& texture = glTFModel.textures[roughness.baseColorTexture.index];
						auto& image = glTFModel.images[texture.source];
						std::vector<std::byte> data(image.image.size());
						std::memcpy(data.data(), image.image.data(), image.image.size());
						serializedObject.textureData = std::make_pair(Vector2u(image.width, image.height), data);
					}
				}

				serializedObjects.push_back(serializedObject);
			}
		}
	}

	std::vector<SerializedObject> serializedObjects;

private:
	void NormalizeIntegerColors(int index, std::vector<Vector4f>& colors)
	{
		auto& accessor = glTFModel.accessors[index];
		bool notFloat =
			accessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT &&
			accessor.componentType != TINYGLTF_COMPONENT_TYPE_DOUBLE;

		if (notFloat) {
			size_t componentSize = tinygltf::GetComponentSizeInBytes(accessor.componentType);
			float factor = 1. / std::pow(255, componentSize);
			for (auto& color : colors) color *= factor;
		}
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
		if (componentCount == TINYGLTF_TYPE_SCALAR)
			return InnerTypeMarshal<Scalar, O>(componentType, buffer);
		if (componentCount == TINYGLTF_TYPE_VEC2)
			return InnerTypeMarshal<Vector2, O>(componentType, buffer);
		if (componentCount == TINYGLTF_TYPE_VEC3)
			return InnerTypeMarshal<Vector3, O>(componentType, buffer);
		if (componentCount == TINYGLTF_TYPE_VEC4)
			return InnerTypeMarshal<Vector4, O>(componentType, buffer);
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
