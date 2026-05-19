//==============================================================================
#pragma once
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
	class EventBase;
	using EventCallbackFn = std::function<void(Core::EventBase&)>;
}
namespace Crystal::Core
{
	struct ApplicationContext
	{
		std::string_view name = "Crystal App";
		std::string_view version = "0.0.1";
		bool with_window = true;
	};

	struct WindowContext
	{
		std::string_view title = "Crystal Engine";
		CRSTu32 width = 1280;
		CRSTu32 height = 720;
	};

	enum class EventType : CRSTu8
	{
		None = 0,
		WindowClose, WindowResize,
		KeyPressed, KeyReleased,
		MouseMoved, MouseButtonPressed,
		TimeAdvanced
	};

	enum class MouseButtonCode : CRSTu8
	{
		None = 0,
		Left, Right, Middle, Side1, Side2
	};

}
namespace Crystal::Core
{
	class EventBase;

	template <typename T>
	concept isCRSTEvent = requires
	{
		typename T::is_event;
		requires std::derived_from<T, EventBase>;
	};

	template <typename T, isCRSTEvent EventT>
	concept isCRSTEventCallback = requires
	{
		requires std::invocable<T, EventT&>;
		requires std::convertible_to<std::invoke_result_t<T, EventT&>, CRSTbool>;
	};
}
//==============================================================================
