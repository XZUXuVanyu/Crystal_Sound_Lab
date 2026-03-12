module;
#include <cmath>
#include <numbers>
#include <array>
#include <random>
export module Crystal_Core.AudioNode:Source;
import :Base;
import Crystal_Log;
import Crystal_Core.Foundation;

export namespace Crystal
{
	enum class Wave_Form : u8
	{
		Sine = 0, Square, Impulse, White_Noise,

		//This should not be used, reserved for lookup table
		Count
	};
	class Oscillator : public Audio_Source
	{
	public:
		//Function ptr for lambdas, params are reserved for now
		using Wave_Generator = f32(*)(f64 phase, const std::array<f32, 8>& params);

		State_Code Set_Wave_Form(Wave_Form wv_form)
		{
			wave_form = wv_form;
			return State_Code::Success;
		}
		void Process(Audio_Buffer& output) override
		{
			if (!output.data) {
				Crystal_Debug_Log("check here");
				return;
			}

			f64 sample_rate = static_cast<f64>(output.spec.sample_rate);
			u8 channels = output.spec.channels;
			f64 dphase = (static_cast<f64>(freq) * two_pi) / sample_rate;
			f32* device_buf = reinterpret_cast<f32*>(output.data);

			for (size_t sp = 0; sp < output.frame_count; sp++)
			{
				f32 sample_value = wave_generator_table[static_cast<u8>(wave_form)](current_phase, params) * amp;
				for (u8 ch = 0; ch < channels; ch++) {
					device_buf[sp * channels + ch] = sample_value;
				}

				current_phase += dphase;
				if (current_phase >= 2.0 * std::numbers::pi_v<f64>)
					current_phase -= 2.0 * std::numbers::pi_v<f64>;
			}
		}
		f32 freq = 440.0f;
		f32 amp = 0.5f;

	private:
		const f64 two_pi = std::numbers::pi_v<f64> *2;
		f64 current_phase = 0.0;
		Wave_Form wave_form = Wave_Form::Sine;
		std::array<f32, 8> params = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

		static constexpr std::array<Wave_Generator, static_cast<size_t>(Wave_Form::Count)> \
			wave_generator_table = {
			//Sine generator
			[](f64 phase, const std::array<f32, 8>& params) -> f32 { return static_cast<f32>(std::sin(phase)); },
			//Square generator
			[](f64 phase, const std::array<f32, 8>& params) -> f32 { return (phase < std::numbers::pi_v<f64>) ? 1.0f : -1.0f; },
			//Impulse generator
			[](f64 phase, const std::array<f32, 8>& params) -> f32 { return 0.0f; },
			//White_Noise generator
			[](f64 phase, const std::array<f32, 8>& params) -> f32 {
				static std::mt19937 gen(77);
				static std::uniform_real_distribution<f32> dis(-1.0f, 1.0f);
				return dis(gen); },
		};
	};
}