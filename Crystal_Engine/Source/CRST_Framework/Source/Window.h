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
	class WindowBase
    {
    public:
        using is_crst_window = void;
		//==============================================================================
		virtual ~WindowBase() = default;
		CRST_NON_COPYABLE(WindowBase)
		//==============================================================================
        virtual void routeEvent(const std::function<void(Message::EventBase&)>& callback) = 0;
	protected:
		WindowBase() = default;
    };

	class ApplicationWindowBase : public WindowBase
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
	std::unique_ptr<WindowBase> createWindow();
}
//==============================================================================
