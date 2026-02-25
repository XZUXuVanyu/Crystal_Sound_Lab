module;
#include <cmath>
#include <numbers>
#include "glad/glad.h"
#include <SDL.h>
#include "imgui.h"

export module CrystalModules.Oscillator;
import CrystalCore.Signal;

export namespace CrystalModules 
{
	enum WaveForm
	{
		Sine,
		Square,
		WhiteNoise
	};

	template <std::size_t N>
	class Oscillator
	{
	public:
		explicit Oscillator(WaveForm waveform, float freq, float amp)
			: waveform(waveform), freq(freq), amp(amp) 
		{
			fillWaveForm(waveform);
		};
		void fillWaveForm(WaveForm waveform)
		{
			switch (waveform)
			{
			case CrystalModules::Sine:
				for (size_t i = 0; i < wave_table.getSizeinFloat(); i++)
					wave_table[i] = amp * std::sin(2.0 * std::numbers::pi_v<double>*i / (std::pow(2, N)/sizeof(float)));
				break;
			case CrystalModules::Square:
				for (size_t i = 0; i < wave_table.getSizeinFloat(); i++)
					wave_table[i] = amp * ((i <= (wave_table.getSizeinFloat() + 1) / 2) ? 1.0f : -1.0f);
				break;
			case CrystalModules::WhiteNoise:
				for (size_t i = 0; i < wave_table.getSizeinFloat(); i++)
				break;
			default:
				break;
			}
		}
		void extractOut(CrystalCore::Block<11>& out)
		{
			out << wave_table;
		}

		void drawUI() 
		{
			ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar;

			ImGui::Begin("Oscillator Control", nullptr, flags);

			if (ImGui::SliderFloat("Frequency", &freq, 20.0f, 2000.0f, "%.2f Hz"))
			ImGui::SliderFloat("Amplitude", &amp, 0.0f, 1.0f);

			const char* items[] = { "Sine", "Square", "WhiteNoise" };
			int current_item = (int)waveform;
			if (ImGui::Combo("Waveform", &current_item, items, IM_ARRAYSIZE(items))) {
				waveform = (WaveForm)current_item;
				fillWaveForm(waveform);
			}
			ImGui::PlotLines("Table Preview", wave_table.extractOutFloat(), (int)wave_table.getSizeinFloat());
			ImGui::End();
		}

		CrystalCore::Block<11> wave_table;
		WaveForm waveform;
		float freq;
		float amp;

	};
}