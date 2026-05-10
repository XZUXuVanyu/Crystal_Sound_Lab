#include "Window.h"

namespace Crystal::Core
{
	WindowBase::WindowBase(const WindowContext& window_context)
		: context(window_context)
	{
		
	}
	WindowBase::~WindowBase()
	{
		
	}
	WindowContext WindowBase::getContext() const
	{
		return context;
	}

}

