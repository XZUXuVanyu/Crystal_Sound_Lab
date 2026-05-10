//==============================================================================
#pragma once
#include <string>
#include "TypeAndConcepts.h"
//==============================================================================
namespace Crystal::Core
{
	struct WindowContext
	{
		std::string title;
		CRSTu32 width = 1920, height = 1080;

		/**
		 * @brief refresh rate of window in Hz, 0 = Vsync
		 */
		CRSTu16 refresh_rate = 0;
		CRSTf32 getAspectRatio() const
		{
			return static_cast<CRSTf32>(width / height);
		}
	};
	class WindowBase
	{
	public:
		//==============================================================================
		WindowBase() = delete;
		virtual ~WindowBase();
		WindowBase(const WindowBase& other) = delete;
		WindowBase& operator=(const WindowBase& other) = delete;
		WindowBase(WindowBase&& other) = delete;
		WindowBase& operator=(WindowBase&& other) = delete;
		//==============================================================================
		WindowContext getContext() const;
		virtual void getWidth() const = 0;
		virtual void getHeight() const = 0;
		virtual void* getNativePtr() const = 0;
		//==============================================================================
		virtual void setRefreshRate(const CRSTu16& refresh_rate) const = 0;
		//==============================================================================
		virtual void onUpdate() = 0;
	protected:
		//==============================================================================
		WindowBase(const WindowContext& window_context);
	private:
		//==============================================================================
		WindowContext context;
	};
}
