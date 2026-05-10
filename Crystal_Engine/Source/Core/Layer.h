//==============================================================================
#pragma once
#include <memory>

//==============================================================================
namespace Crystal::Core
{
	class LayerBase
	{
	public:
		virtual void onUpdate() = 0;
		virtual void onRender() = 0;

		/* TODO: Add queue after event */
		void transitionTo(std::unique_ptr<LayerBase> another);
	};

	class LayerManager
	{
	public:

	};

}
//==============================================================================