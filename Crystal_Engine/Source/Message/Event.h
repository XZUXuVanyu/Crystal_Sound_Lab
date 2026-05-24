//==============================================================================
#pragma once
#include <concepts>
#include "../Core/CoreTypeDef.h"
//==============================================================================
namespace Crystal::Message
{
    enum class EventType : CRSTu8
    {
        None = 0,
        WindowClose, WindowResize,
        KeyPressed, KeyReleased,
        MouseMoved, MouseButtonPressed,
        TimeAdvanced,
        Count
    };
}
namespace Crystal::Message
{
    struct EventBase
    {
        //==============================================================================
        using is_crst_event = void;
        //==============================================================================
        EventBase() = default;
        virtual ~EventBase() = default;
        EventBase(const EventBase&) = delete;
        EventBase& operator=(const EventBase&) = delete;
        EventBase(EventBase&&) noexcept = default;
        EventBase& operator=(EventBase&&) noexcept = default;
        //==============================================================================
        virtual EventType getType() const { return EventType::None; }
        //==============================================================================
        CRSTbool handled = false;
        CRSTbool is_loop_event = false;
        CRSTu32 frame_live_count = 0;
    };

    template <typename T>
    concept isCRSTEvent = requires
    {
        typename T::is_crst_event;
        requires std::derived_from<T, EventBase>;
    };
    template <typename T, typename EventT>
    concept isCRSTEventCallback = requires
    {
        requires isCRSTEvent<EventT>;
        requires std::invocable<T, EventT&>;
        requires std::convertible_to<std::invoke_result_t<T, EventT&>, CRSTbool>;
    };

    template<EventType EventType_T>
    struct Event : EventBase
    {
        static EventType getStaticType() { return EventType_T; }
        EventType getType() const override { return getStaticType(); }
    };
}
namespace Crystal::Message
{
    struct MouseMoved : Event<EventType::MouseMoved>
    {
        MouseMoved(CRSTf32 x, CRSTf32 y) : x(x), y(y) {}
        CRSTf32 x, y;
    };
    enum class MouseButtonCode : CRSTu8
    {
        None = 0,
        Left, Right, Middle, Side1, Side2
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
namespace Crystal::Message
{
    class EventDispatcher
    {
    public:
        EventDispatcher(EventBase& e) : event(e) {}

        template<isCRSTEvent EventT, isCRSTEventCallback<EventT> Callback_T>
        [[nodiscard]] CRSTbool dispatch(const Callback_T& func) noexcept
        {
            if (event.getType() == EventT::getStaticType())
            {
                CRST_ASSERT(event.getType() != EventType::None, 
                    "Cannot dispatch event with EventType::None");
                if (!event.handled) bool callback_checked = func(static_cast<EventT&>(event));
                return true;
            }
            return false;
        }
    private:
        EventBase& event;
    };
}
//==============================================================================