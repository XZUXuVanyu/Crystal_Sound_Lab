//==============================================================================
#pragma once
#include <atomic>
#include <memory>
#include <mutex>
#include <queue>
#include <vector>
#include <fstream>
#include <CRST_Input/CRST_Input.h>
#include <CRST_Time/CRST_Time.h>
#include "Layer.h"
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
		CRSTu64 input_buffer_capacity = 256;
	};
}
namespace Crystal::Framework
{
	class ApplicationBase
	{
	public:
		static void queueCommand(std::unique_ptr<Message::CommandBase> cmd) noexcept;
		static void queueEvent(std::unique_ptr<Message::EventBase> e) noexcept;
		using is_crst_application = void;
		//==============================================================================
		ApplicationBase();
		virtual ~ApplicationBase();
		ApplicationBase(const ApplicationBase&) = delete;
		ApplicationBase& operator=(const ApplicationBase&) = delete;
		//==============================================================================
		void			initialise(const ApplicationContext& context) noexcept;
		void			engineLoop();
		void			quit();
		//==============================================================================
		void			onTimeAdvance(const Time::Duration duration, 
			const Input::InputState& input) noexcept;
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
		virtual void	onTimeAdvanceImpl(const Time::Duration duration, 
			const Input::InputState& input) noexcept = 0;
		virtual void	onCommandImpl(Message::CommandBase& cmd) noexcept = 0;
		virtual void	onEventImpl(Message::EventBase& e) noexcept = 0;
	protected:
		//==============================================================================
		std::queue<std::unique_ptr<Message::CommandBase>>	commands;
		std::queue<std::unique_ptr<Message::EventBase>>		events;
		std::vector<std::unique_ptr<LayerBase>>				layers;
		mutable std::mutex									commands_lock;
		mutable std::mutex									events_lock;

		std::unique_ptr<Input::InputBase>					engine_input;
		std::unique_ptr<Time::ClockBase>					engine_clock;
		std::thread											engine_thread;
		std::atomic<CRSTbool>								is_running { false };
	private:
		static ApplicationBase* instance;
	};
	class WindowedApplicationBase : public ApplicationBase
	{
	protected:
		friend struct Message::CommandMapper;
		//==============================================================================
		WindowedApplicationBase() = default;
		virtual void userInitialise() = 0;
		virtual void userTimeAdvance(const Time::Duration duration, const Input::InputState& input) = 0;
		virtual void userCommandProcess(Message::CommandBase& cmd) = 0;
		virtual void userEventProcess(Message::EventBase& e) = 0;
	protected:
		//==============================================================================
		std::unique_ptr<WindowBase>	window;
	private:
		void        initialiseImpl(const ApplicationContext& context) noexcept override;
		void        onTimeAdvanceImpl(const Time::Duration duration, 
			const Input::InputState& input) noexcept final;
		void		onCommandImpl(Message::CommandBase& cmd) noexcept final;
		void        onEventImpl(Message::EventBase& e) noexcept final;
	};

}
namespace Crystal::Framework
{
	std::unique_ptr<ApplicationBase> createApplication();
}
//==============================================================================
