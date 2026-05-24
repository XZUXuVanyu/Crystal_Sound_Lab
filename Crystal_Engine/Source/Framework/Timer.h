//==============================================================================
#pragma once
#include <functional>
#include <memory>
#include "../Core/CoreTypeDef.h"
#include "../Core/CoreUtilities.h"
//==============================================================================
namespace Crystal::Framework
{
	class MinimumTimerBase
	{
	public:
		using is_crst_timer = void;
		//==============================================================================
		MinimumTimerBase(const MinimumTimerBase&) = delete;
		MinimumTimerBase& operator=(const MinimumTimerBase&) = delete;
		virtual ~MinimumTimerBase() = default;
		//==============================================================================
		virtual void start(const std::function<void(CRSTf64)>& callback) = 0;
		virtual void pause() = 0;
		virtual void stop() = 0;
		//==============================================================================
		virtual void onTimeAdvance(CRSTf64 delta_time) = 0;
		virtual void setTimeStep(CRSTf64 step) = 0;
		//==============================================================================
		virtual CRSTf64 getCurrentTime() const = 0;
		virtual CRSTf64 getDeltaTime() const = 0;
	protected:
		MinimumTimerBase() = default;
	};
}
namespace Crystal::Framework
{
	class FixedTimeStepBase : public MinimumTimerBase
	{
	public:
		//==============================================================================
		// You must implement this !!!
		// virtual void start(const std::function<void(CRSTf64)>& callback) = 0;
		//==============================================================================
		FixedTimeStepBase(CRSTf64 step) : time_step(step)
		{
			CRST_ASSERT(step > 0.0, "Time step must be positive");
		}
		~FixedTimeStepBase() override = default;
		//==============================================================================
		void onTimeAdvance(CRSTf64 raw_dt) final
		{
			CRST_EXPECT(raw_dt >= 0.0,
				"Hardware clock inversion");
			CRST_ASSERT(time_step > 0.0,
				"Simulation step collapsed to zero");

			delta_time = raw_dt;
			accumulator += raw_dt;
			
			while (accumulator >= time_step)
			{
				total_time += time_step;
				accumulator -= time_step;

				if (notify_time_advance_callback) notify_time_advance_callback(time_step);
			}
		}
		//==============================================================================
		void setTimeStep(CRSTf64 step) override { time_step = step; }
		CRSTf64 getCurrentTime() const final { return total_time; }
		CRSTf64 getDeltaTime() const final { return delta_time; }
	protected:
		std::function<void(CRSTf64)> notify_time_advance_callback = nullptr;
	protected:
		CRSTf64 time_step;
		CRSTf64 accumulator = 0.0;
		CRSTf64 delta_time = 0.0;
		CRSTf64 total_time = 0.0;
	};
}
namespace Crystal::Framework
{
	std::unique_ptr<MinimumTimerBase> createTimer(CRSTf64 time_step);
}
//==============================================================================