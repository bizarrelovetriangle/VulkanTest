#pragma once
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include <shaderc/shaderc.hpp>
#include <unordered_map>
#include <filesystem>

class ShaderCompiler
{
public:
	static std::vector<uint32_t> CompileShader(
		const std::filesystem::path& path, vk::ShaderStageFlagBits shaderStage, bool optimize = false)
	{
		const std::filesystem::path rootSpirevPath = std::filesystem::current_path() / "spirv";
		std::string optimizeFlag = optimize ? ".spirv_opt" : ".spirv";
		auto spirvPath = rootSpirevPath / (path.relative_path().string() + optimizeFlag);
		bool outdated = true;

		if (std::filesystem::exists(spirvPath))
		{
			auto codeLastModified = std::filesystem::last_write_time(path).time_since_epoch();
			auto spirvLastModified = std::filesystem::last_write_time(spirvPath).time_since_epoch();
			if (codeLastModified < spirvLastModified) outdated = false;
		}

		if (outdated) UpdateSpirv(path, spirvPath, shaderStage, optimize);

		std::string spirvCode = ReadFile(spirvPath);
		auto spirvData = (uint32_t*)spirvCode.c_str();
		return std::vector<uint32_t>(spirvData, spirvData + spirvCode.length() / sizeof(uint32_t));
	}

private:
	static void UpdateSpirv(const std::filesystem::path& path, const std::filesystem::path& spirvPath,
		vk::ShaderStageFlagBits shaderStage, bool optimize = false)
	{
		std::filesystem::create_directories(spirvPath.parent_path());

		std::string code = ReadFile(path);
		auto fileName = path.filename();

		shaderc::Compiler compiler;
		shaderc::CompileOptions options;

		// Like -DMY_DEFINE=1
		options.AddMacroDefinition("MY_DEFINE", "1");
		if (optimize) options.SetOptimizationLevel(shaderc_optimization_level_performance);

		shaderc_shader_kind shaderKind = shaderKindMap[shaderStage];
		auto module = compiler.CompileGlslToSpv(code, shaderKind, fileName.string().c_str(), options);

		if (module.GetCompilationStatus() != shaderc_compilation_status_success)
		{
			std::cerr << module.GetErrorMessage().c_str() << std::endl;
			throw std::exception();
		}

		std::cout << fileName << " - successfully recompilled" << std::endl;

		WriteFile(spirvPath, std::vector<uint32_t>(module.cbegin(), module.cend()));
	}

	static std::string ReadFile(const std::filesystem::path& path)
	{
		std::ifstream file(path, std::ios::binary);
		if (!file) throw std::runtime_error("file is not found");
		std::stringstream ss;
		ss << file.rdbuf();
		return ss.str();
	}

	static void WriteFile(const std::filesystem::path& path, const std::vector<uint32_t> data)
	{
		std::ofstream file(path, std::ios::binary);
		file.write((char*) data.data(), data.size() * sizeof(uint32_t));
	}

	inline static std::unordered_map<vk::ShaderStageFlagBits, shaderc_shader_kind> shaderKindMap
	{
		{vk::ShaderStageFlagBits::eVertex, shaderc_vertex_shader},
		{vk::ShaderStageFlagBits::eFragment, shaderc_fragment_shader},
		{vk::ShaderStageFlagBits::eCompute, shaderc_compute_shader},
		{vk::ShaderStageFlagBits::eGeometry, shaderc_geometry_shader}
	};
};
