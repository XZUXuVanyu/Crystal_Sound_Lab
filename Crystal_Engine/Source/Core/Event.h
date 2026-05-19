//==============================================================================
#pragma once
#include "TypeAndConcepts.h"
//==============================================================================
namespace Crystal::Core
{
    struct EventBase
    {
        CRSTbool handled = false;
        virtual EventType getType() const { return EventType::None; }
        EventBase() = default;
        virtual ~EventBase() = default;
    };
    template<EventType EventType_T>
    struct Event : EventBase
    {
        using is_event = void;
        static EventType getStaticType() { return EventType_T; }
        EventType getType() const override { return getStaticType(); }
    };
    struct MouseMoved : Event<EventType::MouseMoved>
    {
	    MouseMoved(CRSTf32 x, CRSTf32 y) : x(x), y(y) {}
        CRSTf32 x, y;
    };
    struct MouseButtonPressed : Event<EventType::MouseButtonPressed>
    {
        MouseButtonPressed(MouseButtonCode button) : button(button) {}
        MouseButtonCode button;
    };
    struct WindowClose : Event<EventType::WindowClose>
    {
        WindowClose() = default;
    };
}
namespace Crystal::Core
{
    class EventDispatcher
    {
    public:
        EventDispatcher(EventBase& e) : event(e) {}

        template<isCRSTEvent EventT, isCRSTEventCallback<EventT> CallbackT>
        CRSTbool dispatch(const CallbackT& func)
        {
            if (event.getType() == EventT::getStaticType())
            {
                if (!event.handled)
                {
                    event.handled |= func(static_cast<EventT&>(event));
                }
                return true;
            }
            return false;
        }

    private:
        EventBase& event;
    };

#define CRST_BIND_EVENT_CALLBACK(fn) [this](auto&&... args) -> decltype(auto) \
    { return this->fn(std::forward<decltype(args)>(args)...); }
}
