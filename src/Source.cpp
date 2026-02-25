#include <iostream>
#include <variant>

#include "glad/glad.h"
#include "SDL.h"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"

import CrystalCore.CoreUtils;
import CrystalServices.FileControl;
import CrystalCore.Contexts;
import CrystalCore.Signal;
import CrystalCore.Shader;
import CrystalCore.Event;
import CrystalApplications.GraphicManager;
import CrystalModules.Oscillator;
import CrystalModules.Oscilloscope;

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

CrystalCore::AudioContext global_actx{};
CrystalCore::WindowContext global_wctx{.width = 720};

int main(int argc, char* args[])
{
	CrystalApplications::GraphicManager graphic_manager;
	graphic_manager.init(global_wctx);
	graphic_manager.printGLAttributes();
	CrystalModules::Oscillator<11>		oscillator(CrystalModules::Sine, 440.0f, 0.5f);
	CrystalModules::Oscilloscope<11>	oscilloscope(global_actx, global_wctx);

	CrystalCore::Shader oscshader(\
		R"(F:\Project Files\Visual Studio Projects\Crystal Sound Lab\src\waveform_vs.glsl)", \
		R"(F:\Project Files\Visual Studio Projects\Crystal Sound Lab\src\waveform_fs.glsl)");
	oscshader.useShaderProgram();


	bool running = true;
	SDL_Event sdl_event;
	CrystalCore::EventManager event_manager;
	CrystalCore::Event crystal_event;
	float my_value = 0.0f;
	while (running)
	{
		// Event Cycle
		{
			while (SDL_PollEvent(&sdl_event))
			{
				ImGui_ImplSDL2_ProcessEvent(&sdl_event);
				if (sdl_event.type == SDL_QUIT) 
				{ 
					running = false; 
				}

				// Key events
				if (sdl_event.type == SDL_KEYDOWN)
				{

				}

				if (sdl_event.type == SDL_WINDOWEVENT)
				{
					if (sdl_event.window.event == SDL_WINDOWEVENT_RESIZED)
					{
						double new_ratio = static_cast<double>(sdl_event.window.data1) / sdl_event.window.data2;
						global_wctx.width = sdl_event.window.data1;
						global_wctx.ratio = new_ratio;
						event_manager.pushEvent(CrystalCore::EventType::WindowSpecChange{ global_wctx });
					}
				}
			}
			while (event_manager.pollEvent(crystal_event))
			{
				auto EventHandler = overloaded
				{
					[&](CrystalCore::EventType::WindowSpecChange& ev)
					{ glViewport(0,0,ev.context.width, ev.context.getHeight()); ev.context.print(); },
					[&](auto&& other) {},
				};

				std::visit(EventHandler, crystal_event.data);
			}
		}
		CrystalCore::Block<11> temp;
		oscillator.extractOut(temp);
		oscilloscope.pumpIn(temp);
		oscilloscope.RenderToFBO(oscshader);

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

		oscillator.drawUI();
		oscilloscope.DrawUI(); 

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		SDL_GL_SwapWindow(graphic_manager.main_window);
	}

	graphic_manager.cleanup();
	return 0;
}