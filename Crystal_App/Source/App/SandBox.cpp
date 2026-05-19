//==============================================================================
// 文件名: SandboxApp.cpp (归属于 SandboxApp 项目)
//==============================================================================
#include "Core/Core_Mininum.h"
class MyMathSandbox : public Crystal::Core::ApplicationBase
{
public:
    void initialise() override 
	{
        ApplicationBase::initialise();
        std::cout << "[Sandbox] Sandbox initialised" << std::endl;
    }
    void onEvent(Crystal::Core::EventBase& e) override 
	{
        ApplicationBase::onEvent(e);
    }

    Crystal::CRSTbool onWindowClose(Crystal::Core::WindowClose& e) override
    {
        e.handled = true;
        std::cout << "[Sandbox] onWindowClose event handled" << std::endl;
        return true;
    }
    Crystal::CRSTbool onMouseMoved(Crystal::Core::MouseMoved& e) override
    {
        e.handled = true;
        return true;
    }
    Crystal::CRSTbool onMouseButtonPressed(Crystal::Core::MouseButtonPressed& e) override
    {
        e.handled = true;
        std::cout << "[Sandbox] onMouseButtonPressed: Button = " << 
            static_cast<int>(e.button) << std::endl;
        return true;
    }
};

Crystal::Core::ApplicationBase* Crystal::Core::createApplication()
{
    return new MyMathSandbox();
}
