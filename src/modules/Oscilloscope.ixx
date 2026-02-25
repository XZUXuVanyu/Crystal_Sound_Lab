module;
#include <cmath>
#include <numbers>
#include "glad/glad.h"
#include <SDL.h>
#include "imgui.h"

export module CrystalModules.Oscilloscope;
import CrystalCore.CoreUtils;
import CrystalCore.Contexts;
import CrystalCore.Signal;
import CrystalCore.Shader;

namespace CrystalModules
{
	const CrystalCore::GLAttribute osc_point_color
	{
		.index = 0,
		.dimension = 1,
		.type = GL_FLOAT,
		.normalized = GL_FALSE,
		.byte_per_vertex = sizeof(GL_FLOAT),
		.offset = 0
	};

	export template <std::size_t N>
		class Oscilloscope
	{
	public:
		struct OscSettings {
			float thickness = 1.0f;
			ImVec4 plot_color = { 0.1f, 1.0f, 0.2f, 1.0f };
			bool auto_res = true;
			int manual_w = 512;
			int manual_h = 256;
			float time_scale = 1.0f;
			float volt_scale = 1.0f;
		};

		explicit Oscilloscope(CrystalCore::AudioContext& audio_ctx,
			CrystalCore::WindowContext& window_ctx,
			const OscSettings& preset = OscSettings{})
			: audio_context(audio_ctx), window_context(window_ctx), osc_settings(preset)
		{
			osc_point.init(2048 * sizeof(float), { osc_point_color });
		}

		void pumpIn(CrystalCore::Block<N>& in)
		{
			glBindBuffer(GL_ARRAY_BUFFER, osc_point.vbo_id);
			glBufferSubData(GL_ARRAY_BUFFER, 0, in.getSizeinFloat() * sizeof(float), in.extractOut());
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}

		void RenderToFBO(CrystalCore::Shader& shader)
		{
			if (osc_wvform.width <= 0 || osc_wvform.height <= 0) return;

			osc_wvform.bind();
			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			shader.useShaderProgram();
			shader.setUniform<int>("ui_point_count_pf", osc_settings.manual_w);
			shader.setUniform<float>("uf_gain", osc_settings.volt_scale);
				shader.setUniform<float>("uf_x_aspect", osc_settings.time_scale);

				float aspect = static_cast<float>(osc_wvform.width) / static_cast<float>(osc_wvform.height);
				shader.setUniform<float>("uf_y_aspect", 1.0f / aspect);

				glBindVertexArray(osc_point.vao_id);
			glEnable(GL_LINE_SMOOTH);
			glLineWidth(osc_settings.thickness);

				glDrawArrays(GL_LINE_STRIP, 0, osc_settings.manual_w);

			osc_wvform.unbind();
		}

		void DrawUI()
		{
			static float aspect_ratio = 4.0f / 3.0f;
			ImGui::SetNextWindowSizeConstraints(
				ImVec2(300, 300 / aspect_ratio), ImVec2(FLT_MAX, FLT_MAX),
				KeepAspectRatio, (void*)&aspect_ratio
			);

			ImGuiWindowFlags parent_flags = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_AlwaysAutoResize;

				if (ImGui::Begin("Oscilloscope", nullptr, parent_flags))
				{
					float target_height = is_expanded ? 320.0f : 160.0f;

						ImGui::BeginChild("Control", ImVec2(0, target_height), false,
							ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar);

					ImDrawList* draw_list = ImGui::GetWindowDrawList();
					ImVec2 p_min = ImGui::GetWindowPos();
						ImVec2 p_max = ImVec2(p_min.x + ImGui::GetWindowWidth(), p_min.y + ImGui::GetWindowHeight());

						draw_list->AddRect(p_min, p_max, IM_COL32(255, 255, 255, 150), 2.0f);
					draw_list->AddText(ImVec2(p_min.x + 8, p_min.y + 5), IM_COL32(255, 255, 255, 255), "WAVEFORM");

						ImVec2 avail_size = ImGui::GetContentRegionAvail();
					if (osc_settings.auto_res) 
					{
						osc_wvform.set(static_cast<int>(avail_size.x), 160);
					}
					else {
						osc_wvform.set(osc_settings.manual_w, osc_settings.manual_h);
					}

					ImGui::SetCursorPos(ImVec2(5, 25));
						if (osc_wvform.texture_id != 0) {
							ImGui::Image((void*)(intptr_t)osc_wvform.texture_id, ImVec2(avail_size.x - 10, 120), { 0, 1 }, { 1, 0 });

								if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
									is_expanded = !is_expanded;
									DEBUG_LOG("Oscilloscope Panel Toggled: {}", is_expanded);
								}
						}

					if (is_expanded) {
						ImGui::SetCursorPosX(10);
						ImGui::BeginGroup();
							ImGui::Separator();

						ImGui::TextDisabled("PARAMS CONTROL");
						ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.6);
							ImGui::SliderFloat("Volt Gain", &osc_settings.volt_scale, 0.01f, 5.0f, "%.2f");
							ImGui::SliderFloat("Time Base", &osc_settings.time_scale, 0.01f, 5.0f, "%.2f");
							bool is_collapsed = ImGui::CollapsingHeader("Settings");
							if (is_collapsed) 
							{
								
								ImGui::SliderFloat("Line Thickness", &osc_settings.thickness, 1.0f, 5.0f);
								ImGui::Checkbox("Auto Resolution", &osc_settings.auto_res);
								if (!osc_settings.auto_res) 
								{
									ImGui::InputInt("Width", &osc_settings.manual_w);
									ImGui::InputInt("Height", &osc_settings.manual_h);
								}
							}
						ImGui::PopItemWidth();
							ImGui::EndGroup();
					}
					ImGui::EndChild();
				}
			ImGui::End();
		}

	private:
		static void KeepAspectRatio(ImGuiSizeCallbackData* data) 
		{
			float ratio = *(float*)data->UserData;
			data->DesiredSize.y = data->DesiredSize.x / ratio;
		}

		CrystalCore::AudioContext& audio_context;
		CrystalCore::WindowContext& window_context;
		CrystalCore::VAOContext osc_point;
		CrystalCore::FBOContext osc_wvform;


		bool is_expanded = false;
		OscSettings osc_settings;
	};
}