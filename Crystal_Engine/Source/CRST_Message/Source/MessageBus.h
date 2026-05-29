//==============================================================================
#pragma once
#include <memory>
#include <mutex>
#include <utility>
#include <CRST_Core/CRST_Core.h>
#include <CRST_Framework/CRST_Framework.h>
#include "Command.h"
#include "Event.h"

//==============================================================================
namespace Crystal::Message
{
	//==============================================================================
	inline void submitCommand(std::unique_ptr<CommandBase> cmd) noexcept
	{
		Framework::ApplicationBase::queueCommand(std::move(cmd));
	}
	inline void raiseEvent(std::unique_ptr<EventBase> e) noexcept
	{
		Framework::ApplicationBase::queueEvent(std::move(e));
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
