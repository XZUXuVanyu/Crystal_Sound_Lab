//==============================================================================
#pragma once
#include <glm/glm.hpp>
//==============================================================================
namespace Crystal
{
	using CRSTbool = bool;

	using CRSTi8  = int8_t;
	using CRSTi16 = int16_t;
	using CRSTi32 = int32_t;
	using CRSTi64 = int64_t;

	using CRSTu8  = uint8_t;
	using CRSTu16 = uint16_t;
	using CRSTu32 = uint32_t;
	using CRSTu64 = uint64_t;

	using CRSTf32 = float;
	using CRSTf64 = double;
}

namespace Crystal::Core
{
	template <typename To_be_Checked, typename Base>
	concept CRSTisDerivedFrom = std::is_base_of_v<Base, To_be_Checked>;

	template <typename To_be_Checked, typename Derived>
	concept CRSTisBaseOf = std::is_base_of_v<Derived, To_be_Checked>;

	
}

