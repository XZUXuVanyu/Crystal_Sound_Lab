//==============================================================================
#pragma once
#include <array>
#include <complex>
#include <functional>
#include <queue>
#include <concepts>

#include <CRST_Core/CRST_Core.h>
#include <CRST_Framework/CRST_Framework.h>

//==============================================================================
namespace Crystal::Message
{
	enum class CommandType : CRSTu8
	{
		None = 0,
		TimerStart,
		TimerPause,
		TimerStop,
		LayerPush,
		LayerDelete,
		LayerReplace,
		LayerSwap,
		ApplicationShutDown,

		UserDefined,
		Count
	};
}
namespace Crystal::Message
{
	struct CommandBase
	{
		//==============================================================================
		using is_crst_command = void;
		//==============================================================================
		CommandBase() = default;
		virtual ~CommandBase() = default;
		CommandBase(const CommandBase&) = delete;
		CommandBase& operator=(const CommandBase&) = delete;
		CommandBase(CommandBase&&) noexcept = default;
		CommandBase& operator=(CommandBase&&) noexcept = default;
		//==============================================================================
		virtual CommandType getType() const { return CommandType::None; }
		//==============================================================================
		CRSTbool handled = false;
		CRSTbool is_loop_command = false;
		CRSTu32 frame_live_count = 0;
	};
	template<typename T>
	concept isCRSTCommand = requires
	{
		typename T::is_crst_command;
		requires std::derived_from<T, CommandBase>;
	};

	template<CommandType CommandType_T>
	struct Command : CommandBase
	{
		static CommandType getStaticType() { return CommandType_T; }
		CommandType getType() const override { return getStaticType(); }
	};
}
namespace Crystal::Message
{
	struct TimerStart : Command<CommandType::TimerStart>
	{
		TimerStart(CRSTf64 ts) : time_step(ts) {}
		CRSTf64 time_step;
	};
	struct TimerPause : Command<CommandType::TimerPause>
	{
	};
	struct LayerPush : Command<CommandType::LayerPush>
	{
		LayerPush(std::unique_ptr<Framework::LayerBase> l) : layer(std::move(l)) {}
		std::unique_ptr<Framework::LayerBase> layer;
	};
	struct LayerDelete : Command<CommandType::LayerDelete>
	{
		LayerDelete(std::string n) : name(std::move(n)) {}
		std::string name;
	};
	struct LayerSwap : Command<CommandType::LayerSwap>
	{
		LayerSwap(std::string name_1, std::string name_2) :
			name_1(std::move(name_1)), name_2(std::move(name_2)) {}
		std::string name_1, name_2;
	};

	struct ApplicationShutDown : Command<CommandType::ApplicationShutDown>
	{
		ApplicationShutDown() {}
	};
}
namespace Crystal::Message
{
	using UserDefinedCommandAction = std::function<CRSTbool(CRSTu32 action_frame_count)>;
	struct UserDefinedCommands : Command<CommandType::UserDefined>
	{
		UserDefinedCommands(CRSTu32 total_frame_count)
		{
			this->frame_live_count = total_frame_count;
			if (total_frame_count > 0) this->is_loop_command = true;
		}
		UserDefinedCommands& nextAction(UserDefinedCommandAction action)
		{
			actions.push(std::move(action));
			return *this;
		}

		std::queue<UserDefinedCommandAction> actions;
		CRSTu32 frame_count_used = 0;
	};
}
namespace Crystal::Message
{
	using CommandHandler = void(*)(CommandBase&, Framework::MinimumApplicationBase*);
	inline std::array<CommandHandler, static_cast<CRSTu8>(CommandType::Count)> command_table = { nullptr };
	void initialiseCommandMap() noexcept;
}
//==============================================================================
