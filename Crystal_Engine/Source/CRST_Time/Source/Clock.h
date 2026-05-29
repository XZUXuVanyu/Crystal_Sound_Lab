//==============================================================================
// EngineClock.h
//==============================================================================
#pragma once
#include <algorithm>
#include <atomic>
#include <memory>
#include <ostream>
#include "Timer.h"
//==============================================================================
namespace Crystal::Time
{
	/**
	 * @brief 时间增量单位
	 * @param bits 数据, 高32位为主帧计数, 低32位为副帧定点表示
	 */
	class alignas(16) Duration
	{
	public:
		CRSTu64 bits = 0;

		Duration operator+(const Duration& other) const noexcept
		{
			const CRSTu64 result = bits + other.bits;
			CRST_ASSERT(result >= other.bits, "Duration total 64-bit integer overflow");
			return Duration{ .bits = result };
		}
		Duration& operator+=(const Duration& other) noexcept
		{
			bits += other.bits;
			CRST_ASSERT(bits >= other.bits, "Duration total 64-bit integer overflow");
			return *this;
		}
		friend std::ostream& operator<<(std::ostream& os, const Duration& duration)
		{
			os << "[" << (duration.bits >> 32) << "] frames + [" << (duration.bits & 0xFFFF) << "] subframe";
			return os;
		}

	};

	class alignas(16) TimePoint
	{
	public:
		CRSTu64 maj_count = 0;
		CRSTu32 sub_frac = 0;
		CRSTu32 reserved = 0;

		Duration operator-(const TimePoint& other) const
		{
			CRST_ASSERT((maj_count > other.maj_count) || (maj_count == other.maj_count && sub_frac >= other.sub_frac),
				"Time point reversed");
			return Duration{ .bits = ((maj_count - other.maj_count) << 32) + sub_frac - other.sub_frac };
		}
		friend std::ostream& operator<<(std::ostream& os, const TimePoint& time_point)
		{
			os << "[" << time_point.maj_count << "] frames + [" << (time_point.sub_frac) << "] subframe";
			return os;
		}

	};
}

namespace Crystal::Time
{
	class ClockBase
	{
	public:
		//==============================================================================
		explicit ClockBase(CRSTf64 time_step_seconds, std::unique_ptr<TimerBase> timer_impl)
			: timer(std::move(timer_impl)),
			base_time_step_nano(static_cast<CRSTu64>(time_step_seconds * 1e9)),
			start_absolute_nano(timer ? timer->getAbsoluteTimeNano() : 0ULL),
			last_absolute_time_nano(start_absolute_nano),
			accumulator_nano(0ULL)
		{
			CRST_ASSERT(timer != nullptr, "Platform timer dependency missing");
			CRST_ASSERT(base_time_step_nano > 0, "Time step is too small to be represented in nanoseconds");
		}
		virtual ~ClockBase() = default;
		CRST_NON_COPYABLE(ClockBase)
		//==============================================================================
			void advanceTime() noexcept
		{
			const CRSTu64 absolute_time_nano = timer->getAbsoluteTimeNano();
			CRSTu64 raw_dt_nano = (absolute_time_nano > last_absolute_time_nano) ?
				absolute_time_nano - last_absolute_time_nano : 0ULL;
			last_absolute_time_nano = absolute_time_nano;

			raw_dt_nano = std::min(raw_dt_nano, 3 * base_time_step_nano);
			accumulator_nano += raw_dt_nano;
		}

		[[nodiscard]] CRSTbool canUpdateMajorFrame() const noexcept
		{
			return accumulator_nano >= base_time_step_nano;
		}

		/**
		 * @brief 消耗一个固定步长，确立逻辑主帧格点（此时子帧强制对齐清零）
		 */
		void consumeMajorFrame() noexcept
		{
			if (accumulator_nano < base_time_step_nano) return;
			accumulator_nano -= base_time_step_nano;

			TimePoint local_time = system_time.load(std::memory_order_relaxed);
			local_time.maj_count += 1;
			local_time.sub_frac = 0;

			system_time.store(local_time, std::memory_order_release);
		}

		/**
		 * @brief 刷新并注入包含高精度最近舍入物理残余的子帧
		 */
		void consumeSubFrame() noexcept
		{
			TimePoint local_time = system_time.load(std::memory_order_relaxed);

			const CRSTu64 scaled = (accumulator_nano << 32) + (base_time_step_nano / 2);
			const CRSTu64 sub_ticks = scaled / base_time_step_nano;

			local_time.maj_count += (sub_ticks >> 32);
			local_time.sub_frac = static_cast<CRSTu32>(sub_ticks & 0xFFFFFFFF);

			system_time.store(local_time, std::memory_order_release);
		}

		//==============================================================================
		TimePoint getCurrentTimePoint() const noexcept
		{
			return system_time.load(std::memory_order_acquire);
		}
		CRSTu64 getBaseTimeStepNano() const noexcept
		{
			return base_time_step_nano;
		}
		CRSTu64 getAccumulatorNano() const noexcept
		{
			return accumulator_nano;
		}
		CRSTu64 getRawTimeNano() const noexcept
		{
			return timer->getAbsoluteTimeNano();
		}

		//==============================================================================
		[[nodiscard]] TimePoint secondToTimePoint(CRSTf64 absolute_seconds) const noexcept
		{
			if (absolute_seconds <= 0.0) return TimePoint{ .maj_count = 0, .sub_frac = 0 };
			const CRSTu64 target_nanoseconds = static_cast<CRSTu64>(absolute_seconds * 1e9);
			return nanosecondToTimePoint(target_nanoseconds);
		}
		[[nodiscard]] TimePoint nanosecondToTimePoint(CRSTu64 absolute_nanoseconds) const noexcept
		{
			if (absolute_nanoseconds <= start_absolute_nano)
			{
				return TimePoint{ .maj_count = 0, .sub_frac = 0 };
			}

			const CRSTu64 elapsed_nano = absolute_nanoseconds - start_absolute_nano;
			const CRSTu64 maj_count = elapsed_nano / base_time_step_nano;
			const CRSTu64 ns_remain = elapsed_nano % base_time_step_nano;
			const CRSTu64 scaled = (ns_remain << 32) + (base_time_step_nano / 2);
			const CRSTu64 sub_ticks = scaled / base_time_step_nano;

			TimePoint result;
			if (sub_ticks >= (1ULL << 32))
			{
				result.maj_count = maj_count + 1;
				result.sub_frac = 0;
			}
			else
			{
				result.maj_count = maj_count;
				result.sub_frac = static_cast<CRSTu32>(sub_ticks);
			}
			return result;
		}
		[[nodiscard]] CRSTu64	durationToNanosecond(const Duration& duration) const
		{
			return ((duration.bits * base_time_step_nano) >> 32);
		}
	protected:
		//==============================================================================
		std::unique_ptr<TimerBase>          timer = nullptr;
		CRSTu64                             last_absolute_time_nano;
		CRSTu64                             accumulator_nano;

	private:
		//==============================================================================
		const CRSTu64                       base_time_step_nano;
		const CRSTu64                       start_absolute_nano;
		alignas(16) std::atomic<TimePoint>  system_time{};
	};
}
namespace Crystal::Time
{
	std::unique_ptr<ClockBase> createClock(CRSTf64 time_step);
}
//==============================================================================