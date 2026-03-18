module;
#include <cstdint>
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

		Path_Not_Exists = 6,
	};
}