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
        if (auto* mouse_event = dynamic_cast<Crystal::Core::MouseClickEvent*>(&e))
        {
            std::cout << "[Sandbox] Mouse click" << std::endl;
            e.handled = true;
        }
    }
};
//==============================================================================
// 履行引擎的入口点契约 (Fulfilling the Entry Point Contract)
// 链接器 (Linker) 会在最后把这个函数和 JUCE 平台层缝合在一起。
//==============================================================================
Crystal::Core::ApplicationBase* Crystal::Core::createApplication()
{
    return new MyMathSandbox();
}
