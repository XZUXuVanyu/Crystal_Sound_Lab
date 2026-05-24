//==============================================================================
#include "Application.h"
#include <CRST_Message/CRST_Message.h>
#include <iostream>
namespace Crystal::Framework
{
	MinimumApplicationBase* MinimumApplicationBase::instance = nullptr;
	MinimumApplicationBase::MinimumApplicationBase()
	{
		CRST_ASSERT(instance == nullptr, "Application instance duplicated");
		instance = this;
	}
	MinimumApplicationBase::~MinimumApplicationBase()
	{
		if (instance == this) instance = nullptr;
	}
	void MinimumApplicationBase::initialise(const ApplicationContext& context) noexcept
	{
		CRST_ASSERT(context.time_step > 0.0, "Application initialised with non-positive timestep intervals");
		Message::initialiseCommandMap();
		initialiseImpl(context);
	}
	void MinimumApplicationBase::quit()
	{
		is_running = false;
	}
	void MinimumApplicationBase::onTimeAdvance(CRSTf64 dt) noexcept
	{
		flushCommands();
		pollEvents();
		onTimeAdvanceImpl(dt);
	}
	void MinimumApplicationBase::onCommand(Message::CommandBase& cmd) noexcept
	{
		onCommandImpl(cmd);
	}
	void MinimumApplicationBase::onEvent(Message::EventBase& e) noexcept
	{
		onEventImpl(e);
	}
	void MinimumApplicationBase::flushCommands() noexcept
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
	void MinimumApplicationBase::pollEvents() noexcept
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
	// WindowedApplicationBase implementation
	//==============================================================================
	void WindowedApplicationBase::initialiseImpl(const ApplicationContext& context) noexcept
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
	void WindowedApplicationBase::onTimeAdvanceImpl(CRSTf64 dt) noexcept
	{
		current_time.store(timer->getCurrentTime());
		++current_frame;

		for (auto layer_it = layers.begin(); layer_it != layers.end(); ++layer_it)
		{
			CRST_ASSERT((*layer_it) != nullptr, "Null layer pointer detected inside active layer stack");
			(*layer_it)->onTimeAdvance(dt);
		}
		userTimeAdvance(dt);
	}
	void WindowedApplicationBase::onCommandImpl(Message::CommandBase& cmd) noexcept
	{
		CRSTu8 index = static_cast<CRSTu8>(cmd.getType());
		CRST_ASSERT(index < static_cast<CRSTu8>(Message::CommandType::Count), "command_table index out of bounds");
		CRST_EXPECT(Message::command_table[index] != nullptr, "Attempted to process an unbound command slot");
		Message::command_table[index](cmd, this);

		userCommandProcess(cmd);
	}
	void WindowedApplicationBase::onEventImpl(Message::EventBase& e) noexcept
	{
		Message::EventDispatcher dispatcher{ e };
		for (auto layer_it = layers.rbegin(); layer_it != layers.rend(); ++layer_it)
		{
			CRST_ASSERT((*layer_it) != nullptr, "Null layer pointer detected inside active layer stack");
			if (e.handled) break;
			(*layer_it)->onEvent(e);
		}

		userEventProcess(e);

		if (!e.handled and e.getType() == Message::EventType::WindowClose) 
		{
			std::cout << "[Engine] WindowClose handled, system shutdown" << std::endl;
			Message::submitCommand(std::make_unique<Message::Command<Message::CommandType::ApplicationShutDown>>());
			e.handled = true;
		}
	}
	void MinimumApplicationBase::queueCommand(std::unique_ptr<Message::CommandBase> cmd) noexcept
	{
		CRST_ASSERT(instance != nullptr, "Initialise application first");
		CRST_EXPECT(cmd != nullptr, "Do not submit a null command", void());
		{
			std::scoped_lock lock(instance->commands_lock);
			instance->commands.push(std::move(cmd));
		}
	}
	void MinimumApplicationBase::queueEvent(std::unique_ptr<Message::EventBase> e) noexcept
	{
		CRST_ASSERT(instance != nullptr, "Initialise application first");
		CRST_EXPECT(e != nullptr, "Do not raise a null event", void());
		{
			std::scoped_lock lock(instance->events_lock);
			instance->events.push(std::move(e));
		}
	}
}
//==============================================================================