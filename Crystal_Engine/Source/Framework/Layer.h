//==============================================================================
#pragma once
#include "../Core/CoreTypeDef.h"
#include "../Core/CoreUtilities.h"
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
        LayerBase(const LayerBase&) = delete;
        LayerBase& operator=(const LayerBase&) = delete;
        LayerBase(LayerBase&&) noexcept = default;
        LayerBase& operator=(LayerBase&&) noexcept = default;
        //==============================================================================
        virtual void onAttach() = 0;
        virtual void onDetach() = 0;
        virtual void onTimeAdvance(CRSTf64 dt) = 0;
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
