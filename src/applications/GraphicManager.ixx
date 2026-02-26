module;
#include <iostream>
#include <format>
#include "glad/glad.h"
#include <SDL.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl2.h"
#include "imgui/imgui_impl_opengl3.h"
export module CrystalApplications.GraphicManager;
import CrystalCore.CoreUtils;
import CrystalCore.Contexts;

export namespace CrystalApplications
{
	class GraphicManager
	{
	public:
		SDL_Window*		main_window = nullptr;
		SDL_GLContext	gl_context	= nullptr;
		~GraphicManager() { cleanup(); }

		bool init(CrystalCore::WindowContext& context)
		{
			// SDL init
			if (SDL_Init(SDL_INIT_VIDEO) < 0) return false;
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

			main_window = SDL_CreateWindow(context.title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
				context.width, context.getHeight(), context.window_flags);
			if (!main_window) return false;

			gl_context = SDL_GL_CreateContext(main_window);
			if (!gl_context) return false;

			if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) return false;

			// ImGui init
			IMGUI_CHECKVERSION();
			ImGui::CreateContext();
			ImGui::StyleColorsDark();
			ImGui_ImplSDL2_InitForOpenGL(main_window, gl_context);
			ImGui_ImplOpenGL3_Init("#version 460");

			SDL_GL_SetSwapInterval(1);
			
			SDL_SetWindowResizable(main_window, (context.window_flags & SDL_WINDOW_RESIZABLE) ? SDL_TRUE : SDL_FALSE);
			SDL_SetWindowHitTest(main_window, HitTestCallback, this);
			return true;
		}
		void cleanup()
		{
			if (gl_context)		SDL_GL_DeleteContext(gl_context);
			if (main_window)	SDL_DestroyWindow(main_window);
			SDL_Quit();
		}
	public:
		void printGLAttributes() 
		{
			const GLubyte* renderer = glGetString(GL_RENDERER);
			const GLubyte* version	= glGetString(GL_VERSION);

			DEBUG_LOG("[SYSTEM INFO] GPU[{}] OpenGL Version[{}]",\
				reinterpret_cast<const char*>(renderer), reinterpret_cast<const char*>(version));
		}

	public:
		static SDL_HitTestResult HitTestCallback(SDL_Window* window, const SDL_Point* area, void* data)
		{
			auto* self = static_cast<GraphicManager*>(data);
			int w, h;
			SDL_GetWindowSize(window, &w, &h);

			const int edge_margin = 5;
			const int title_bar_height = 30;

			if (area->y < edge_margin) {
				if (area->x < edge_margin) return SDL_HITTEST_RESIZE_TOPLEFT;
				if (area->x > w - edge_margin) return SDL_HITTEST_RESIZE_TOPRIGHT;
				return SDL_HITTEST_RESIZE_TOP;
			}
			if (area->y > h - edge_margin) {
				if (area->x < edge_margin) return SDL_HITTEST_RESIZE_BOTTOMLEFT;
				if (area->x > w - edge_margin) return SDL_HITTEST_RESIZE_BOTTOMRIGHT;
				return SDL_HITTEST_RESIZE_BOTTOM;
			}
			if (area->x < edge_margin) return SDL_HITTEST_RESIZE_LEFT;
			if (area->x > w - edge_margin) return SDL_HITTEST_RESIZE_RIGHT;

			if (area->y < title_bar_height) return SDL_HITTEST_DRAGGABLE;

			return SDL_HITTEST_NORMAL;
		}
	};
}
