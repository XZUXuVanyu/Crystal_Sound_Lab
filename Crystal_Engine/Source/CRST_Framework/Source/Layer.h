//==============================================================================
#pragma once
#include <CRST_Core/CRST_Core.h>
#include <string>
#include <string_view>
#include <concepts>

namespace Crystal::Input
{
	struct InputState;
}

namespace Crystal::Time
{
	class Duration;
}

namespace Crystal::Message
{
	struct EventBase;
}
//==============================================================================
namespace Crystal::Framework
{
	class LayerBase
    {
    public:
        //==============================================================================
        using is_crst_layer = void;
        //==============================================================================
        LayerBase(std::string name = "Layer") : name(std::move(name))
        {
            CRST_ASSERT(!this->name.empty(), "Do not create layer with null name");
        }
        virtual ~LayerBase() = default;
        CRST_NON_COPYABLE(LayerBase)
        //==============================================================================
        virtual void onAttach() = 0;
        virtual void onDetach() = 0;
        virtual void onTimeAdvance(const Time::Duration& duration, const Input::InputState& input) = 0;
        virtual void onEvent(Message::EventBase& e) = 0;
        //==============================================================================
        const std::string_view getName() const { return name; }
    protected:
        //==============================================================================
        std::string name;
    };
}
namespace Crystal::Framework
{
    template <typename T>
    concept isCRSTLayer = requires
    {
        typename T::is_crst_layer;
        requires std::derived_from<T, LayerBase>;
    };
}
//==============================================================================
