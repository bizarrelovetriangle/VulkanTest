#pragma once
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include <shaderc/shaderc.hpp>

class ShaderCompiler
{
public:
	static std::vector<uint32_t> CompileShader(
		const std::string& path, shaderc_shader_kind kind, bool optimize = false)
	{
		std::ifstream file(path, std::ios::binary);
		if (!file) throw std::runtime_error("file is not found");
		std::stringstream ss;
		ss << file.rdbuf();
		std::string code = ss.str();

		shaderc::Compiler compiler;
		shaderc::CompileOptions options;

		// Like -DMY_DEFINE=1
		options.AddMacroDefinition("MY_DEFINE", "1");
		if (optimize) options.SetOptimizationLevel(shaderc_optimization_level_size);

		auto module = compiler.CompileGlslToSpv(code, kind, code.c_str(), options);

		if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
			std::cerr << module.GetErrorMessage();
			return std::vector<uint32_t>();
		}

		return { module.cbegin(), module.cend() };
	}
};
