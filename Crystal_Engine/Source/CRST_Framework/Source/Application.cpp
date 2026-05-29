//==============================================================================
#include "Application.h"
#include <CRST_Message/CRST_Message.h>
#include <iostream>

#include "CRST_Time/Source/Clock.h"
using namespace Crystal::Time;
using namespace Crystal::Framework;
using namespace Crystal::Message;
using namespace Crystal::Input;

ApplicationBase* ApplicationBase::instance = nullptr;
ApplicationBase::ApplicationBase()
{
	CRST_ASSERT(instance == nullptr, "Application instance duplicated");
	instance = this;
}
ApplicationBase::~ApplicationBase()
{
	if (instance == this) instance = nullptr;
}
void ApplicationBase::initialise(const ApplicationContext& context) noexcept
{
	CRST_ASSERT(context.time_step > 0.0, "Application initialised with non-positive timestep intervals");
	initialiseCommandMap();
	initialiseImpl(context);
}
void ApplicationBase::engineLoop()
{
	CRST_ASSERT(engine_clock != nullptr, "Initialise error");

	TimePoint frame_time_point;
	Duration frame_time;
	InputState frame_input;
	
	while (is_running.load(std::memory_order_relaxed))
	{
		engine_clock->advanceTime();

		while (engine_clock->canUpdateMajorFrame())
		{
			frame_time_point = engine_clock->getCurrentTimePoint();
			frame_input = engine_input->fetchInputState(frame_time_point);

			for (auto& layer_it : layers) (*layer_it).onTimeAdvance(frame_time, frame_input);	

			engine_clock->consumeMajorFrame();
		}

		engine_clock->consumeSubFrame();

		//std::this_thread::yield();
	}
}
void ApplicationBase::quit()
{
	is_running = false;
}
void ApplicationBase::onTimeAdvance(const Time::Duration duration,
	const Input::InputState& input) noexcept
{
	flushCommands();
	pollEvents();
	onTimeAdvanceImpl(duration,input);
}
void ApplicationBase::onCommand(CommandBase& cmd) noexcept
{
	onCommandImpl(cmd);
}
void ApplicationBase::onEvent(EventBase& e) noexcept
{
	onEventImpl(e);
}
void ApplicationBase::flushCommands() noexcept
{
	std::queue<std::unique_ptr<CommandBase>> commands_temp;
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
void ApplicationBase::pollEvents() noexcept
{
	if (!is_running) [[unlikely]] return;
	std::queue<std::unique_ptr<EventBase>> events_temp;
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

	engine_input = createInputAdapter(context.input_buffer_capacity);
	std::cout << "[Engine] engine_input initialised" << std::endl;

	engine_clock = createClock(context.time_step);
	std::cout << "[Engine] engine_clock initialised" << std::endl;

	window = createWindow();
	window->linkInputAdapter(engine_input.get());
	window->routeEvent(CRST_BIND_CALLBACK(onEvent));
	std::cout << "[Engine] engine_window initialised" << std::endl;

	userInitialise();
	std::cout << "[Engine] userInitialisation finished" << std::endl;

	is_running.store(true);
	engine_thread = std::thread(&WindowedApplicationBase::engineLoop, this);

	std::cout << "[Engine] Engine loop started" << std::endl;
}
void WindowedApplicationBase::onTimeAdvanceImpl(const Time::Duration duration, const Input::InputState& input) noexcept
{
	for (auto layer_it = layers.begin(); layer_it != layers.end(); ++layer_it)
	{
		CRST_ASSERT((*layer_it) != nullptr, "Null layer pointer detected inside active layer stack");
		(*layer_it)->onTimeAdvance(duration, input);
	}
	userTimeAdvance(duration, input);
}
void WindowedApplicationBase::onCommandImpl(CommandBase& cmd) noexcept
{
	CRSTu8 index = static_cast<CRSTu8>(cmd.getType());
	CRST_ASSERT(index < static_cast<CRSTu8>(CommandType::Count), "command_table index out of bounds");
	CRST_EXPECT(command_table[index] != nullptr, "Attempted to process an unbound command slot");
	command_table[index](cmd, this);

	userCommandProcess(cmd);
}
void WindowedApplicationBase::onEventImpl(EventBase& e) noexcept
{
	EventDispatcher dispatcher{ e };
	for (auto layer_it = layers.rbegin(); layer_it != layers.rend(); ++layer_it)
	{
		CRST_ASSERT((*layer_it) != nullptr, "Null layer pointer detected inside active layer stack");
		if (e.handled) break;
		(*layer_it)->onEvent(e);
	}

	userEventProcess(e);

	if (!e.handled and e.getType() == EventType::WindowClose) 
	{
		std::cout << "[Engine] WindowClose handled, system shutdown" << std::endl;
		submitCommand(std::make_unique<Command<CommandType::ApplicationShutDown>>());
		e.handled = true;
	}
}
void ApplicationBase::queueCommand(std::unique_ptr<CommandBase> cmd) noexcept
{
	CRST_ASSERT(instance != nullptr, "Initialise application first");
	CRST_EXPECT(cmd != nullptr, "Do not submit a null command", void());
	{
		std::scoped_lock lock(instance->commands_lock);
		instance->commands.push(std::move(cmd));
	}
}
void ApplicationBase::queueEvent(std::unique_ptr<EventBase> e) noexcept
{
	CRST_ASSERT(instance != nullptr, "Initialise application first");
	CRST_EXPECT(e != nullptr, "Do not raise a null event", void());
	{
		std::scoped_lock lock(instance->events_lock);
		instance->events.push(std::move(e));
	}
}
//==============================================================================
