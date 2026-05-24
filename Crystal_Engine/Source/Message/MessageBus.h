//==============================================================================
#pragma once
#include "../Framework/Application.h"
//==============================================================================
namespace Crystal::Message
{
	//==============================================================================
	inline void submitCommand(std::unique_ptr<CommandBase> cmd) noexcept
	{
		CRST_ASSERT(Framework::MinimumApplicationBase::instance != nullptr, "Initialise application first");
		CRST_EXPECT((cmd != nullptr) and (cmd->getType() != CommandType::None),
			"Do not submit a null command", void());
		{
			std::scoped_lock lock(Framework::MinimumApplicationBase::instance->commands_lock);
			Framework::MinimumApplicationBase::instance->commands.push(std::move(cmd));
		}
	}
	inline void raiseEvent(std::unique_ptr<EventBase> e) noexcept
	{
		CRST_ASSERT(Framework::MinimumApplicationBase::instance != nullptr, "Initialise application first");
		CRST_EXPECT((e != nullptr) and (e->getType() != EventType::None),
			"Do not raise a null event", void());
		{
			std::scoped_lock lock(Framework::MinimumApplicationBase::instance->events_lock);
			Framework::MinimumApplicationBase::instance->events.push(std::move(e));
		}
	}
	//==============================================================================
	template <isCRSTCommand CommandType_T, typename... ArgsType_T_List>
	void submitCommand(ArgsType_T_List&&... args_list)
	{
		submitCommand(std::make_unique<CommandType_T>(std::forward<ArgsType_T_List>(args_list)...));
	}
	//==============================================================================
	template <typename EventType_T, typename... ArgsType_T_List>
	void raiseEvent(ArgsType_T_List&&... args_list)
	{
		raiseEvent(std::make_unique<EventType_T>(std::forward<ArgsType_T_List>(args_list)...));
	}
	//==============================================================================
}
//==============================================================================