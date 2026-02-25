module;
#include <format>
#include <string>
#include <unordered_map>
#include "glad/glad.h"
#include "glm/glm.hpp"
#include <glm/gtc/type_ptr.hpp>

export module CrystalCore.Shader;
import CrystalCore.CoreUtils;
import CrystalServices.FileControl;

// 辅助模板，用于 static_assert
template<class T> inline constexpr bool always_false = false;

namespace CrystalCore
{
	export class Shader
	{
	public:
		Shader(const std::string& vs_path, const std::string& fs_path)
		{
			std::string vCode = CrystalServices::readShaderFile(vs_path);
			const char* vShaderCode = vCode.c_str();
			GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
			glShaderSource(vertex, 1, &vShaderCode, NULL);
			glCompileShader(vertex);
			checkCompileErrors(vertex, "VERTEX");

				std::string fCode = CrystalServices::readShaderFile(fs_path);
			const char* fShaderCode = fCode.c_str();
			GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(fragment, 1, &fShaderCode, NULL);
			glCompileShader(fragment);
			checkCompileErrors(fragment, "FRAGMENT");

				ID = glCreateProgram();
			glAttachShader(ID, vertex);
			glAttachShader(ID, fragment);
			glLinkProgram(ID);
			checkCompileErrors(ID, "PROGRAM");

				glDeleteShader(vertex);
			glDeleteShader(fragment);
		}

		~Shader() { cleanUp(); }

		Shader(const Shader&) = delete;
		Shader& operator=(const Shader&) = delete;

		Shader(Shader&& other) noexcept : ID(other.ID), m_LocationCache(std::move(other.m_LocationCache))
		{
			other.ID = 0;
		}

		Shader& operator=(Shader&& other) noexcept
		{
			if (this != &other)
			{
				cleanUp();
				ID = other.ID;
				m_LocationCache = std::move(other.m_LocationCache);
				other.ID = 0;
			}
			return *this;
		}

		void useShaderProgram() const { glUseProgram(ID); }

			void cleanUp()
		{
			if (ID != 0)
			{
				glDeleteProgram(ID);
				ID = 0;
			}
		}

		template <typename T>
		bool setUniform(const std::string& name, const T& value)
		{
			GLint location = getUniformLocation(name);
			if (location == -1) return false;

				if constexpr (std::is_same_v<T, float>)          glUniform1f(location, value);
				else if constexpr (std::is_same_v<T, int>)       glUniform1i(location, value);
				else if constexpr (std::is_same_v<T, bool>)      glUniform1i(location, static_cast<int>(value));
				else if constexpr (std::is_same_v<T, uint32_t>)  glUniform1ui(location, value);

					// GLM Vectors
				else if constexpr (std::is_same_v<T, glm::vec2>) glUniform2fv(location, 1, glm::value_ptr(value));
				else if constexpr (std::is_same_v<T, glm::vec3>) glUniform3fv(location, 1, glm::value_ptr(value));
				else if constexpr (std::is_same_v<T, glm::vec4>) glUniform4fv(location, 1, glm::value_ptr(value));

					// Matrices
				else if constexpr (std::is_same_v<T, glm::mat4>) glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));

				else
				{
					static_assert(always_false<T>, "[ERROR] Unsupported uniform type for Shader class.");
					return false;
				}
			return true;
		}

	private:
		GLuint ID = 0;
		std::unordered_map<std::string, GLint> m_LocationCache;

		GLint getUniformLocation(const std::string& name)
		{
			if (m_LocationCache.contains(name))
				return m_LocationCache[name];

			GLint location = glGetUniformLocation(ID, name.c_str());
			m_LocationCache[name] = location;
			return location;
		}

		void checkCompileErrors(GLuint shader, std::string type)
		{
			GLint success;
			GLchar infoLog[1024];
			if (type != "PROGRAM")
			{
				glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
				if (!success)
				{
					glGetShaderInfoLog(shader, 1024, NULL, infoLog);
					DEBUG_LOG("[ERROR] [GL::SHADER_ERROR] Type: {}, Info: {}", type, (const char*)infoLog);
				}
			}
			else
			{
				glGetProgramiv(shader, GL_LINK_STATUS, &success);
				if (!success)
				{
					glGetProgramInfoLog(shader, 1024, NULL, infoLog);
					DEBUG_LOG("[ERROR] [GL::PROGRAM_ERROR] Type: {}, Info: {}", type, (const char*)infoLog);
				}
			}
		}
	};
}