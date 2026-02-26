module;
#include <utility>
#include <numbers>
#include <format>
#include <string>
#include "glad/glad.h"
#include <SDL.h>
#include "imgui/imgui.h"

export module CrystalCore.Contexts;
import CrystalCore.CoreUtils;
export namespace CrystalCore
{

	struct AudioContext
	{
		size_t sample_rate = 48000;
		size_t bit_depth = 16;
		size_t channel = 1;

		size_t bytes_per_point() const { return (bit_depth / 8) * channel; }
		size_t sample_points_to_bytes(size_t sample_points) const { return sample_points * bytes_per_point(); }
		size_t bytes_to_sample_points(size_t bytes) const { return bytes / bytes_per_point(); }

		void print() const
		{
			DEBUG_LOG("[Info] [AudioContext] SampleRate[{} Hz], BitDepth[{} bits], Channels[{}]", \
				sample_rate, bit_depth, channel);
		}

	};

	struct WindowContext
	{
		size_t width = 1920;
		double ratio = 16.0 / 9.0;
		size_t refresh_rate = 180;
		std::string title = "Default Name";
		Uint32 window_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_BORDERLESS;

		size_t getHeight()
		{
			return static_cast<size_t>(static_cast<double>(width) / ratio);
		}
		void operator=(const WindowContext& context)
		{
			width = context.width;
			ratio = context.ratio;
			refresh_rate = context.refresh_rate;
			title = context.title;
			window_flags = context.window_flags;
		}
		void print()
		{
			DEBUG_LOG("[Info] [WindowContext] Width[{}], Height[{}], Ratio[{:<.2f}], title[{}]", \
				width, getHeight(), ratio, title);
		}
	};

	struct GLAttribute
	{
		GLuint index; // layout( location = index)
		GLint dimension;
		GLenum type;
		GLboolean normalized;
		GLsizei byte_per_vertex;
		size_t offset;
		void print() const
		{
			DEBUG_LOG("[INFO] [GLAttr] Index[{}], Dimension[{}], Type[{:#x}], Normalized[{}], Stride[{}], Offset[{}]", \
				index, dimension, type, normalized, byte_per_vertex, offset);
		}
	};
	struct VAOContext
	{
		GLuint vao_id = 0;
		GLuint vbo_id = 0;
		size_t attrib_count = 0;
		size_t buffer_size = 0;
		GLuint buffer_usage = GL_DYNAMIC_DRAW;

		// Use it like init(size, { attr1, attr2... }) Note: .vao_id is unbinded after init()
		void init(size_t size, std::initializer_list<GLAttribute> attributes)
		{
			buffer_size = size;
			glGenVertexArrays(1, &vao_id);
			glGenBuffers(1, &vbo_id);

			glBindVertexArray(vao_id);
			glBindBuffer(GL_ARRAY_BUFFER, vbo_id);

			glBufferData(GL_ARRAY_BUFFER, buffer_size, nullptr, buffer_usage);

			for (const auto& attr : attributes)
			{
				glVertexAttribPointer(
					attr.index,
					attr.dimension,
					attr.type,
					attr.normalized,
					attr.byte_per_vertex,
					reinterpret_cast<const void*>(attr.offset)
				);
				glEnableVertexAttribArray(attr.index);

				attrib_count++;
			}

			// Unbind
			glBindVertexArray(0);
		}
		void set(GLAttribute attribute)
		{
			glBindVertexArray(vao_id);
			glBindBuffer(GL_ARRAY_BUFFER, vbo_id);

			glVertexAttribPointer(
				attribute.index,
				attribute.dimension,
				attribute.type,
				attribute.normalized,
				attribute.byte_per_vertex,
				reinterpret_cast<const void*>(attribute.offset)
			);

			glEnableVertexAttribArray(attribute.index);
			glBindVertexArray(0);
		}
		void bind()
		{
			glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
		}
		void unbind()
		{
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}

		void print() const
		{
			DEBUG_LOG("[INFO] [VAOContext] VAO_ID[{}], VBO_ID[{}], AttrCount[{}], Size[{} bytes], Usage[{:#x}]", \
				vao_id, vbo_id, attrib_count, buffer_size, buffer_usage);
		}

		~VAOContext()
		{
			if (vao_id) glDeleteVertexArrays(1, &vao_id);
			if (vbo_id) glDeleteBuffers(1, &vbo_id);
		}

	};
	struct FBOContext
	{
		GLuint fbo_id = 0;
		GLuint texture_id = 0;
		int width = 0;
		int height = 0;

		void set(int w, int h)
		{
			if (w <= 0 or h <= 0) { DEBUG_LOG("[WARNING] Wrong FBO context: w[{}], h[{}]", w, h); return; }
			if (w == width and h == height and fbo_id != 0) { return; }
			width = w; height = h;

			if (fbo_id == 0) glGenFramebuffers(1, &fbo_id);
			if (texture_id == 0) glGenTextures(1, &texture_id);

			glBindTexture(GL_TEXTURE_2D, texture_id);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_id, 0);

			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			{
				GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
				DEBUG_LOG("FBO incomplete: {}", status);
			}

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		void bind() const
		{
			glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);
			glViewport(0, 0, width, height);
		}

		void unbind() const
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		void print() const
		{
			DEBUG_LOG("[INFO] [FBOContext] FBO_ID[{}], Texture_ID[{}], Resolution[{}x{}]", \
				fbo_id, texture_id, width, height);
		}

		~FBOContext()
		{
			if (fbo_id) glDeleteFramebuffers(1, &fbo_id);
			if (texture_id) glDeleteTextures(1, &texture_id);
		}
	};
}