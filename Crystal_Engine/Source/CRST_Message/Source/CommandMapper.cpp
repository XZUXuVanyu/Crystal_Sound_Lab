//==============================================================================
#include <algorithm>
#include <iostream>
#include "Command.h"
//==============================================================================
namespace Crystal::Message
{
	struct CommandMapper
	{
		CommandMapper() noexcept
		{
			command_table[static_cast<CRSTu8>(CommandType::LayerPush)] = &CommandMapper::handleLayerPush;
			command_table[static_cast<CRSTu8>(CommandType::LayerDelete)] = &CommandMapper::handleLayerDelete;
			command_table[static_cast<CRSTu8>(CommandType::LayerSwap)] = &CommandMapper::handleLayerSwap;
			command_table[static_cast<CRSTu8>(CommandType::ApplicationShutDown)] = &CommandMapper::handleApplicationShutDown;
			command_table[static_cast<CRSTu8>(CommandType::UserDefined)] = &CommandMapper::handleUserDefinedCommands;
		}
		static void handleLayerPush(CommandBase& cmd, Framework::ApplicationBase* instance)
		{
			CRST_ASSERT(instance != nullptr, "Application instance runtime pointer is collapsed");
			auto* app = static_cast<Framework::WindowedApplicationBase*>(instance);

			CRST_ASSERT(cmd.getType() == CommandType::LayerPush,
				"Command routing mismatch in handleLayerPush");
			auto& command = static_cast<LayerPush&>(cmd);
			CRST_ASSERT(!command.layer->getName().empty(),
				"Do not create a layer with null name");

			bool is_duplicate = std::ranges::any_of(app->layers,
				[&](std::string_view name) { return name == command.layer->getName(); },
				&Framework::LayerBase::getName);
			CRST_EXPECT(!is_duplicate, 
				"Layer name repetition");

			app->layers.emplace_back(std::move(command.layer));
			app->layers.back()->onAttach();

			cmd.handled = true;
		}
		static void handleLayerDelete(CommandBase& cmd, Framework::ApplicationBase* instance)
		{
			CRST_ASSERT(instance != nullptr, "Application instance runtime pointer is collapsed");
			auto* app = static_cast<Framework::WindowedApplicationBase*>(instance);

			CRST_ASSERT(cmd.getType() == CommandType::LayerDelete,
				"Command routing mismatch in handleLayerDelete");
			auto& command = static_cast<LayerDelete&>(cmd);

			CRST_EXPECT(!command.name.empty(), 
				"Intended to delete a layer with null name");

			auto layer_it = std::ranges::find(app->layers,
				command.name, &Framework::LayerBase::getName);
			CRST_EXPECT(layer_it != app->layers.end(), 
				"Cannot found corresponded layer");

			(*layer_it)->onDetach();
			app->layers.erase(layer_it);

			cmd.handled = true;
		}
		static void handleLayerSwap(CommandBase& cmd, Framework::ApplicationBase* instance)
		{
			CRST_ASSERT(instance != nullptr, "Application instance runtime pointer is collapsed");
			auto* app = static_cast<Framework::WindowedApplicationBase*>(instance);

			CRST_ASSERT(cmd.getType() == CommandType::LayerSwap,
				"Command routing mismatch in handleLayerSwap");
			auto& command = static_cast<LayerSwap&>(cmd);

			CRST_EXPECT(!command.name_1.empty() and !command.name_2.empty(),
				"Intended to swap layer with null name");

			auto layer_it1 = std::ranges::find(app->layers,
				command.name_1, &Framework::LayerBase::getName);
			auto layer_it2 = std::ranges::find(app->layers,
				command.name_2, &Framework::LayerBase::getName);

			CRST_EXPECT((layer_it1 != app->layers.end() and layer_it2 != app->layers.end()),
				"At least one of the layer's name not found");

			std::iter_swap(layer_it1, layer_it2);

			cmd.handled = true;
		}
		static void handleApplicationShutDown(CommandBase& cmd, Framework::ApplicationBase* instance)
		{
			CRST_ASSERT(instance != nullptr, "Application instance runtime pointer is collapsed");
			auto* app = static_cast<Framework::WindowedApplicationBase*>(instance);
			app->quit();

			cmd.handled = true;
		}
		static void handleUserDefinedCommands(CommandBase& cmd, Framework::ApplicationBase* instance)
		{
			CRST_ASSERT(cmd.getType() == CommandType::UserDefined, "Command routing mismatch");
			auto& user_cmds = static_cast<UserDefinedCommands&>(cmd);

			CRST_EXPECT(!user_cmds.actions.empty(), 
				"Attempted to execute an empty user - defined command sequence", void(user_cmds.handled = true));

			while (!user_cmds.actions.empty())
			{
				auto& current_action = user_cmds.actions.front();
				CRSTbool is_subcommand_handled = current_action(user_cmds.frame_count_used++);
				if (is_subcommand_handled)
				{
					std::cout << "[UserDefinedCommands] Active sub-action resolved inside current frame\n";
					user_cmds.actions.pop();
					user_cmds.frame_count_used = 0;
				}
				else break;
			}
			if (user_cmds.actions.empty()) user_cmds.handled = true;
		}
	};
}
namespace Crystal::Message
{
	void initialiseCommandMap() noexcept
	{
		static CommandMapper command_mapper;
	}
}
//==============================================================================
