module;
#include <fstream>
#include <filesystem>
#include <string>

export module CrystalServices.FileControl;
import CrystalCore.CoreUtils;

export namespace CrystalServices
{
	std::string readShaderFile(const std::string& path)
	{
		std::filesystem::path shader_path(path);
		if (!std::filesystem::exists(shader_path) or !std::filesystem::is_regular_file(shader_path))
		{
			DEBUG_LOG("[ERROR] Unable to find shader file");
			return "";
		}

		std::string ext = shader_path.extension().string();
		std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
		if ((ext != ".hlsl") and (ext != ".glsl"))
		{
			DEBUG_LOG("[ERROR] Invalid shader file");
			return "";
		}

		std::ifstream shader_file(shader_path);
		if (!shader_file.is_open())
		{
			DEBUG_LOG("[ERROR] Unable to open shader file");
			return "";
		}

		std::stringstream buffer;
		buffer << shader_file.rdbuf();

		return buffer.str();
	}
}