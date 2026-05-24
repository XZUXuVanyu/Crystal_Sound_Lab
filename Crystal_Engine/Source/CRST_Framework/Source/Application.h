//==============================================================================
#pragma once
#include <memory>
#include <mutex>
#include <queue>
#include <vector>
#include <atomic>
#include <CRST_Core/CRST_Core.h>

#include "Layer.h"
#include "Timer.h"
#include "Window.h"
namespace Crystal::Message
{
	struct CommandBase;
	struct EventBase;
	struct CommandMapper;
}
//==============================================================================
namespace Crystal::Framework
{
	enum class ApplicationType : CRSTu8
	{
		None = 0, 
		Windowed, Task
	};
	struct ApplicationContext
	{
		std::string_view name = "Crystal App";
		std::string_view version = "0.0.1";
		ApplicationType type = ApplicationType::Windowed;
		CRSTf64 time_step = 0.01;
	};
}
namespace Crystal::Framework
{
	class MinimumApplicationBase
	{
	public:
		static void queueCommand(std::unique_ptr<Message::CommandBase> cmd) noexcept;
		static void queueEvent(std::unique_ptr<Message::EventBase> e) noexcept;
		using is_crst_application = void;
		//==============================================================================
		MinimumApplicationBase();
		virtual ~MinimumApplicationBase();
		MinimumApplicationBase(const MinimumApplicationBase&) = delete;
		MinimumApplicationBase& operator=(const MinimumApplicationBase&) = delete;
		//==============================================================================
		void			initialise(const ApplicationContext& context) noexcept;
		void			quit();
		//==============================================================================
		void			onTimeAdvance(CRSTf64 dt) noexcept;
		void			onCommand(Message::CommandBase& cmd) noexcept;
		void			onEvent(Message::EventBase& e) noexcept;
		//==============================================================================
		void			flushCommands() noexcept;
		void			pollEvents() noexcept;
		//==============================================================================
		static CRSTbool	isRunning() noexcept
		{
			CRST_EXPECT(instance != nullptr, "Initialise application first", false);
			return instance->is_running.load();
		}
	protected:
		//==============================================================================
		virtual void	initialiseImpl(const ApplicationContext& context) noexcept = 0;
		virtual void	onTimeAdvanceImpl(CRSTf64 dt) noexcept = 0;
		virtual void	onCommandImpl(Message::CommandBase& cmd) noexcept = 0;
		virtual void	onEventImpl(Message::EventBase& e) noexcept = 0;
	protected:
		//==============================================================================
		std::queue<std::unique_ptr<Message::CommandBase>>	commands;
		std::queue<std::unique_ptr<Message::EventBase>>		events;
		std::vector<std::unique_ptr<LayerBase>>				layers;
		mutable std::mutex									commands_lock;
		mutable std::mutex									events_lock;
		std::atomic<CRSTbool>								is_running { true };
	private:
		static MinimumApplicationBase* instance;
	};
	class WindowedApplicationBase : public MinimumApplicationBase
	{
	protected:
		friend struct Message::CommandMapper;
		//==============================================================================
		WindowedApplicationBase() = default;
		virtual void userInitialise() = 0;
		virtual void userTimeAdvance(CRSTf64 dt) = 0;
		virtual void userCommandProcess(Message::CommandBase& cmd) = 0;
		virtual void userEventProcess(Message::EventBase& e) = 0;
	protected:
		//==============================================================================
		std::unique_ptr<MinimumWindowBase>	window;
		std::unique_ptr<MinimumTimerBase>	timer;
		std::atomic<CRSTu64>				current_frame{ 0 };
		std::atomic<CRSTf64>				current_time{ 0 };
	private:
		void        initialiseImpl(const ApplicationContext& context) noexcept override;
		void        onTimeAdvanceImpl(CRSTf64 dt) noexcept final;
		void		onCommandImpl(Message::CommandBase& cmd) noexcept final;
		void        onEventImpl(Message::EventBase& e) noexcept final;
	};

}
namespace Crystal::Framework
{
	std::unique_ptr<MinimumApplicationBase> createApplication();
}
//==============================================================================
