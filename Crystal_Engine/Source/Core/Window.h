//==============================================================================
#pragma once
#include <functional>
#include <memory>
//==============================================================================
namespace Crystal::Core
{
	struct EventBase;
	class MinimumWindowBase
    {
    public:
        using is_window = void;
		//==============================================================================
		MinimumWindowBase(const MinimumWindowBase&) = delete;
		MinimumWindowBase& operator=(const MinimumWindowBase&) = delete;
		virtual ~MinimumWindowBase() = default;
		//==============================================================================
        virtual void routeEvent(const std::function<void(EventBase&)>& callback) = 0;
	protected:
		MinimumWindowBase() = default;
    };
}
namespace Crystal::Core
{
	class ApplicationWindowBase : public MinimumWindowBase
	{
	public:
		//==============================================================================
		// You must implement this !!!
		// virtual void routeEvent(const std::function<void(EventBase&)>& callback) = 0;
		//==============================================================================
		ApplicationWindowBase() = default;
		~ApplicationWindowBase() override = default;
	protected:
		void dispatchToEngine(EventBase& e) const
		{
			if (dispatch_event_callback) dispatch_event_callback(e);
		}
	protected:
		std::function<void(EventBase&)> dispatch_event_callback = nullptr;
	};
}
namespace Crystal::Core
{
	std::unique_ptr<MinimumWindowBase> createWindow();
}
