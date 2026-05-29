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

inline volatile uint64_t g_telemetry_workload_sink = 0;
inline void execute_linear_workload(uint64_t iterations) noexcept
{
	uint64_t x = 0x123456789ULL;
	for (uint64_t i = 0; i < iterations; ++i)
	{
		x = (x * 6364136223846793005ULL) + 1442695040888963407ULL;
	}
	g_telemetry_workload_sink = x;
}

struct TelemetryRecord {
	uint64_t raw_dt;
	uint64_t delta_accumulator;
	uint64_t duration_ticks;
};
class TelemetryController 
{
public:
	explicit TelemetryController(size_t max_samples = 10000000)
		: max_samples_count(max_samples)
	{
		records.reserve(max_samples_count);
	}

	inline void pushRecord(uint64_t raw_dt, uint64_t delta_acc, uint64_t ticks) noexcept {
		if (!is_full) [[likely]] {
			records.push_back({ raw_dt, delta_acc, ticks });
			if (records.size() >= max_samples_count) {
				is_full = true;
				dumpToCSV("engine_time_telemetry.csv");
			}
		}
	}

	[[nodiscard]] bool isFull() const noexcept { return is_full; }

private:
	void dumpToCSV(const std::string& filename) {
		std::cout << "[Telemetry]"  << max_samples_count << "data points collected. Writing to " << filename << "..." << std::endl;
		std::ofstream file(filename, std::ios::out | std::ios::trunc);
		if (!file.is_open()) {
			std::cerr << "[Telemetry] Failed to open file for writing!" << std::endl;
			return;
		}

		file << "Raw_DT_Nano,Delta_Accumulator,Duration_Ticks\n";

		// 批量写入
		for (const auto& record : records) {
			file << record.raw_dt << ","
				<< record.delta_accumulator << ","
				<< record.duration_ticks << "\n";
		}

		file.close();
		std::cout << "[Telemetry] Data dumped successfully. Collection stopped." << std::endl;
	}

	std::vector<TelemetryRecord> records;
	const size_t                max_samples_count;
	bool                        is_full = false;
};

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
		virtual void userTimeAdvance(CRSTf64 dt) = 0;
		virtual void userCommandProcess(Message::CommandBase& cmd) = 0;
		virtual void userEventProcess(Message::EventBase& e) = 0;
	protected:
		//==============================================================================
		std::unique_ptr<WindowBase>	window;
	private:
		void        initialiseImpl(const ApplicationContext& context) noexcept override;
		void        onTimeAdvanceImpl(CRSTf64 dt) noexcept final;
		void		onCommandImpl(Message::CommandBase& cmd) noexcept final;
		void        onEventImpl(Message::EventBase& e) noexcept final;
	};

}
namespace Crystal::Framework
{
	std::unique_ptr<ApplicationBase> createApplication();
}


//==============================================================================
