//==============================================================================
#pragma once
#include <functional>
#include <memory>
#include <string_view>
#include <CRST_Core/CRST_Core.h>
namespace Crystal::Message
{
	struct EventBase;
}
//==============================================================================
namespace Crystal::Framework
{
	struct WindowContext
	{
		std::string_view title = "Crystal Engine";
		CRSTu32 width = 1280;
		CRSTu32 height = 720;
	};
}
namespace Crystal::Framework
{
	class MinimumWindowBase
    {
    public:
        using is_crst_window = void;
		//==============================================================================
		MinimumWindowBase(const MinimumWindowBase&) = delete;
		MinimumWindowBase& operator=(const MinimumWindowBase&) = delete;
		virtual ~MinimumWindowBase() = default;
		//==============================================================================
        virtual void routeEvent(const std::function<void(Message::EventBase&)>& callback) = 0;
	protected:
		MinimumWindowBase() = default;
    };
	class ApplicationWindowBase : public MinimumWindowBase
	{
	public:
		ApplicationWindowBase() = default;
		~ApplicationWindowBase() override = default;
	protected:
		void dispatchToEngine(Message::EventBase& e) const
		{
			if (dispatch_event_callback) dispatch_event_callback(e);
		}
	protected:
		std::function<void(Message::EventBase&)> dispatch_event_callback = nullptr;
	};
}
namespace Crystal::Framework
{
	std::unique_ptr<MinimumWindowBase> createWindow();
}
//==============================================================================
