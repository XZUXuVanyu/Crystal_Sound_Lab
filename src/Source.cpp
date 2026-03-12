#include <memory>
#include <thread>
import Crystal_Log;
import Crystal_Core.AudioNode;
import Crystal_Service.AudioDriver;
using namespace Crystal;
using namespace Crystal::Service;
int main(int argc, char** argv)
{
	Crystal_Log_Init();
	Crystal_Log_Set_Level(Log_Level::Info);
	Crystal_Info_Log("Hello");
	Device_List d_list;
	Enum_Audio_Devices(d_list);
	Show_Audio_Devices(d_list);

	Audio_Descriptor g_audio_spec{.sample_rate = 44100, .channels = 2, .format = ma_format_f32};

	auto s_sine = std::make_shared<Oscillator>();
	auto b_master = std::make_shared<Audio_Bus>();
	s_sine->freq = 100;
	s_sine->amp = 0.5;
	s_sine->Set_Wave_Form(Wave_Form::Square);
	b_master->Set_Source(s_sine);

	Audio_Device d_output(d_list[5]);
	d_output.Init(g_audio_spec);
	d_output.Set_Bus(b_master);

	d_output.Run();
	std::this_thread::sleep_for(std::chrono::seconds(50));
	return 0;
}