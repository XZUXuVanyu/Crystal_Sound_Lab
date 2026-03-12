module;
#include <atomic>
export module Crystal_Core.AudioNode:Bus;
import :Base;
import Crystal_Core.Foundation;

export namespace Crystal
{
	export class Audio_Bus : public Audio_Pipe
	{
	public:
		void Process(Audio_Buffer& output) override
		{
			Audio_Pipe::Process(output);

			Process_Spec_Conversion();
			//global gain, pan
		}
		void Process_Spec_Conversion()
		{
		}
		std::atomic<f32> global_gain;
		std::atomic<f32> global_pan;
	};
}