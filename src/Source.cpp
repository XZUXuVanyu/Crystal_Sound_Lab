#include <atomic>
#include <condition_variable>
#include <format>
#include <iostream>
#include <memory>
#include <mutex>
#include <regex>
#include <string>
#include <thread>

import Crystal_Log;
import Crystal_Core.Foundation;
import Crystal_Core.AudioNode;
import Crystal_Service.AudioDriver;
using namespace Crystal;
using namespace Crystal::Service;

Audio_Descriptor g_audio_spec{ .sample_rate = 44100, .channels = 2, .format = ma_format_f32 };
std::condition_variable g_cv;
std::mutex g_lock;
std::atomic<bool> exit_flag = false;

void Type_Control(std::shared_ptr<Oscillator> osc, std::atomic<bool>& exit_flag)
{
	std::string input;
	std::smatch match_results;

	std::vector<std::regex> patterns {
		std::regex(R"(\[Wave_Form\]\((\d+)\))"), std::regex(R"(\[Freq\]\(([-+]?[0-9]*\.?[0-9]+)\))"),
		std::regex(R"(\[Amp\]\(([-+]?[0-9]*\.?[0-9]+)\))") };

	std::cout << std::format("Enter [Wave_Form](id) to switch between different waveforms, [Amp](value) to set amp, [Freq](val) to set freq. Exit to quit.") 
		<< std::endl;
	while (!exit_flag.load())
	{
		if (!(std::cin >> input)) {
			break;
		}

		if (input == "exit") {
			exit_flag.store(true);
		}
		try
		{
			if (std::regex_match(input, match_results, patterns[0]))
			{
				int wv_form_id = std::stod(match_results[1]);
				switch (wv_form_id)
				{
				default:
					break;
				case 0:
					osc->Set_Wave_Form(Wave_Form::Sine);
					break;
				case 1:
					osc->Set_Wave_Form(Wave_Form::Square);
					break;
				case 2:
					osc->Set_Wave_Form(Wave_Form::Triangle);
					break;
				case 3:
					osc->Set_Wave_Form(Wave_Form::White_Noise);
					break;
				}
			}
			if (std::regex_match(input, match_results, patterns[1]))
			{
				f32 new_freq = std::stof(match_results[1]);
				osc->freq.store(new_freq);
			}
			if (std::regex_match(input, match_results, patterns[2]))
			{
				f32 new_amp = std::stof(match_results[1]);
				osc->amp.store(new_amp);
			}
		}
		catch (const std::exception& e)
		{
			Crystal_Error_Log(std::format("Invalid numeric input: {}", e.what()));
		}
	}
}

int main(int argc, char** argv)
{
	
	//Init Log
	Crystal_Log_Init();
	Crystal_Log_Set_Level(Log_Level::Info);

	//Search for devices
	Device_List d_list;
	Enum_Audio_Devices(d_list);
	Show_Audio_Devices(d_list);
	
	//Set audio_source and master_bus
	auto s_sine = std::make_shared<Oscillator>();
	auto b_master = std::make_shared<Audio_Bus>();
	s_sine->freq = 440;
	s_sine->amp = 0.5;
	s_sine->Set_Wave_Form(Wave_Form::Sine);
	b_master->Set_Source(s_sine);

	//Set audio_device
	u32 device_id = 0;
	std::cout << "Please enter the device to enable. " << std::endl;
	std::cin >> device_id;

	Audio_Device d_output(d_list[device_id]);
	d_output.Init(g_audio_spec);
	d_output.Set_Bus(b_master);

	//Run 
	std::thread td_controller(Type_Control, s_sine, std::ref(exit_flag));
	d_output.Run();
	td_controller.join();
	return 0;
}