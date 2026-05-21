//==============================================================================
#pragma once
#include <string>
#include "TypeAndConcepts.h"
//==============================================================================
namespace Crystal::Core
{
    class LayerBase
    {
    public:
        //==============================================================================
        using is_layer = void;
        //==============================================================================
        LayerBase(std::string name = "Layer") : name(std::move(name)) {}
        virtual ~LayerBase() = default;
        //==============================================================================
        virtual void onAttach() {}
        virtual void onDetach() {}
        virtual void onTimeAdvance(CRSTf64 dt) {}
        virtual void onEvent(EventBase& e) {}
        //==============================================================================
        const std::string& getName() const { return name; }
    protected:
        //==============================================================================
        std::string name;
    };
}
//==============================================================================