module;
#include <cstdint>
#include "miniaudio.h"
export module Crystal_Core.Foundation;
export namespace Crystal
{
	using u8 = uint8_t;
	using u32 = uint32_t;
	using bl = bool;
	using f32 = float;
	using f64 = double;

	enum class State_Code : u32 
	{
		Success = 0,
		Fail = 1,

		Not_Ready = 2,
		Invalid_Args = 3,

		Device_Error = 4,
		Device_Busy = 5,
	};

	struct Audio_Descriptor 
	{
		u32			sample_rate = 0;
		u8			channels;
		ma_format	format;
	};

	struct Audio_Buffer
	{
		void* data;
		Audio_Descriptor spec;
		//How many sample points to pump/pull from Audio_Device upon next process
		size_t frame_count;
	};

	size_t Frame_Count_To_Bytes(const Audio_Buffer& buf)
	{
		size_t format_to_bytes = 0;
		switch (buf.spec.format)
		{
		case ma_format::ma_format_f32:
			format_to_bytes = sizeof(f32);
			break;

		case ma_format::ma_format_s16:
			format_to_bytes = sizeof(int16_t);
			break;
		default:
			//Clog
			break;
		}
		return buf.frame_count * buf.spec.channels * format_to_bytes;
	}
}