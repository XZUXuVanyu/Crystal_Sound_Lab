//==============================================================================
// 文件名: SandboxApp.cpp (归属于 SandboxApp 项目)
//==============================================================================
#include "Core/Core_Mininum.h"
using namespace Crystal::Core;
class AppLayer : public LayerBase
{
public:
    AppLayer() : LayerBase("App") {}

    void onEvent(EventBase& e) override
    {
        EventDispatcher dispatcher{ e };
        dispatcher.dispatch<MouseButtonPressed>([this](MouseButtonPressed& event) 
            {
            std::cout << "[Layer: App] MouseButtonPressed: button = " << 
	            static_cast<int>(event.button) << std::endl;
            return true;
            });
    }
};
class UIOverlayLayer : public Crystal::Core::LayerBase
{
public:
    UIOverlayLayer() : LayerBase("UI_Overlay") {}

    void onEvent(EventBase& e) override
    {
        EventDispatcher dispatcher{ e };
        dispatcher.dispatch<MouseButtonPressed>([this](MouseButtonPressed& event)
            {
            std::cout << "[Layer: UI] MouseButtonPressed" << std::endl;
            return false;
            });
    }
};

class MyMathSandbox : public ApplicationBase
{
public:
    void initialise() override 
	{
        ApplicationBase::initialise();
        pushLayer<AppLayer>(std::make_unique<AppLayer>());
        pushLayer(std::make_unique<UIOverlayLayer>());
        std::cout << "[Sandbox] Sandbox initialised" << std::endl;
    }
    Crystal::CRSTbool onWindowClose(WindowClose& e) override
    {
        e.handled = true;
        std::cout << "[Sandbox] onWindowClose event handled" << std::endl;
        return true;
    }
};
namespace Crystal::Core
{
    std::unique_ptr<ApplicationBase> createApplication()
    {
        return std::make_unique<MyMathSandbox>();
    }
}