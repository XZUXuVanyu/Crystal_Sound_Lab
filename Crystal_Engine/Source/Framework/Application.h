//==============================================================================
#pragma once
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include "Layer.h"
#include "Timer.h"
#include "Window.h"
#include "../Core/CoreUtilities.h"
#include "../Message/Command.h"
#include "../Message/Event.h"

namespace Crystal::Message
{
	struct CommandMapper;
	inline void submitCommand(std::unique_ptr<CommandBase> cmd) noexcept;
	inline void raiseEvent(std::unique_ptr<EventBase> e) noexcept;
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
		friend Message::CommandBase;
		friend Message::CommandMapper;
		friend void Message::submitCommand(std::unique_ptr<Message::CommandBase> cmd) noexcept;
		friend void Message::raiseEvent(std::unique_ptr<Message::EventBase> e) noexcept;
		//==============================================================================
		using is_crst_application = void;
		//==============================================================================
		MinimumApplicationBase()
		{
			CRST_ASSERT(instance == nullptr, "Application instance duplicated");
			instance = this;
		}
		virtual ~MinimumApplicationBase()
		{
			if (instance == this) instance = nullptr;
		}
		MinimumApplicationBase(const MinimumApplicationBase&) = delete;
		MinimumApplicationBase& operator=(const MinimumApplicationBase&) = delete;
		//==============================================================================
		void			initialise(const ApplicationContext& context) noexcept
		{
			CRST_ASSERT(context.time_step > 0.0,
				"Application initialised with non-positive timestep intervals");
			Message::initialiseCommandMap();
			initialiseImpl(context);
		}
		void			quit()
		{
			is_running = false;
		}
		//==============================================================================
		void			onTimeAdvance(CRSTf64 dt) noexcept
		{
			flushCommands();
			pollEvents();
			onTimeAdvanceImpl(dt);
		}
		void			onCommand(Message::CommandBase& cmd) noexcept
		{
			onCommandImpl(cmd);
		}
		void			onEvent(Message::EventBase& e) noexcept
		{
			onEventImpl(e);
		}
		//==============================================================================
		void			flushCommands() noexcept
		{
			std::queue<std::unique_ptr<Message::CommandBase>> commands_temp;
			{
				std::scoped_lock lock(commands_lock);
				std::swap(commands, commands_temp);
			}

			while (!commands_temp.empty())
			{
				if (!is_running) [[unlikely]] break;
				auto cmd = std::move(commands_temp.front());
				commands_temp.pop();
				if (!cmd) continue;

				onCommand(*cmd);

				if (!cmd->handled)
				{
					if (cmd->frame_live_count > 0)
					{
						cmd->frame_live_count--;
						commands.push(std::move(cmd));
					}
					else
					{
						CRST_ASSERT(!cmd->is_loop_command,
							"Loop command frame_live_count expired without being handled."
							"Ensure loop commands are correctly handled before their frame_live_count hits zero");
					}
				}
			}
		}
		void			pollEvents() noexcept
		{
			if (!is_running) [[unlikely]] return;
			std::queue<std::unique_ptr<Message::EventBase>> events_temp;
			{
				std::scoped_lock lock(events_lock);
				std::swap(events, events_temp);
			}
			while (!events_temp.empty())
			{
				auto e = std::move(events_temp.front());
				events_temp.pop();
				if (!e) continue;

				onEvent(*e);

				if (!e->handled)
				{
					if (e->frame_live_count > 0)
					{
						e->frame_live_count--;
						events.push(std::move(e));
					}
					else
					{
						CRST_ASSERT(!e->is_loop_event,
							"Loop event frame_live_count expired without being handled."
							"Ensure loop events are correctly handled before their frame_live_count hits zero");
					}
				}
			}
		}
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
		//==============================================================================
		// You must reserve memory for this member !!!
		//==============================================================================
		static MinimumApplicationBase* instance;
	};
}
namespace Crystal::Framework
{
	class WindowedApplicationBase : public MinimumApplicationBase
	{
	protected:
		friend Message::CommandMapper;
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
		void        initialiseImpl(const ApplicationContext& context) noexcept override
		{
			CRST_ASSERT(context.type == ApplicationType::Windowed,
				"Initialise windowed application with incorrect context.type");

			timer = createTimer(context.time_step);
			CRST_ASSERT(timer != nullptr, "Failed to allocate hardware timer subsystem");
			timer->start(CRST_BIND_CALLBACK(onTimeAdvance));

			window = createWindow();
			CRST_ASSERT(window != nullptr, "Failed to allocate platform window subsystem");
			window->routeEvent(CRST_BIND_CALLBACK(onEvent));

			std::cout << "[Engine] Engine initialised" << std::endl;

			userInitialise();
		}
		void        onTimeAdvanceImpl(CRSTf64 dt) noexcept final
		{
			current_time.store(timer->getCurrentTime());
			++current_frame;

			for (auto layer_it = layers.begin(); layer_it != layers.end(); ++layer_it)
			{
				CRST_ASSERT((*layer_it) != nullptr,
					"Null layer pointer detected inside active layer stack");
				(*layer_it)->onTimeAdvance(dt);
			}
			userTimeAdvance(dt);
		}
		void		onCommandImpl(Message::CommandBase& cmd) noexcept final
		{
			CRSTu8 index = static_cast<CRSTu8>(cmd.getType());
			CRST_ASSERT(index < static_cast<CRSTu8>(Message::CommandType::Count),
				"command_table index out of bounds");
			CRST_EXPECT(Message::command_table[index] != nullptr,
				"Attempted to process an unbound command slot");
			Message::command_table[index](cmd, this);

			userCommandProcess(cmd);
		}
		void        onEventImpl(Message::EventBase& e) noexcept final
		{
			Message::EventDispatcher dispatcher{ e };
			for (auto layer_it = layers.rbegin(); layer_it != layers.rend(); ++layer_it)
			{
				CRST_ASSERT((*layer_it) != nullptr,
					"Null layer pointer detected inside active layer stack");
				if (e.handled) break;
				(*layer_it)->onEvent(e);
			}

			userEventProcess(e);

			if (!e.handled and e.getType() == Message::EventType::WindowClose) 
			{
				std::cout << "[Engine] WindowClose handled, system shutdown" << std::endl;

				Message::submitCommand(
					std::make_unique<
					Message::Command<Message::CommandType::ApplicationShutDown>>());
				e.handled = true;
			}
		}
	};
	/// TODO: class TaskApplicationBase : public MinimumApplicationBase
}
namespace Crystal::Framework
{
	std::unique_ptr<MinimumApplicationBase> createApplication();
}
//==============================================================================