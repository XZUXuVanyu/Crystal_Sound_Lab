//==============================================================================
#pragma once
#include <array>
#include <bitset>
#include <memory>
#include <CRST_Core/CRST_Core.h>
#include <CRST_Time/CRST_Time.h>

#include "Input.h"

//==============================================================================
namespace Crystal::Input
{
	class LockFreeInputBuffer;
	class InputBufferBase;

	enum class InputChannel : CRSTu8
	{
		None = 0,
		MouseX,
		MouseY,
		MouseMiddleUp,
		MouseMiddleDown
	};
	enum class InputBit : CRSTu8
	{
		None = 0,
		MouseLeft,
		MouseRight,
		MouseMiddle,
		KeyShift,
		KeyCtrl,
		KeyAlt,
		KeyW,
		KeyA,
		KeyS,
		KeyD,
		KeySpace,
		KeyEscape,
		KeyEnter,
		Action0,
		Action1
	};
	struct alignas(16) InputState
	{
		Time::TimePoint time_point = {};
		std::array<CRSTf32, 16> continuous_inputs = {};
		std::bitset<256> discrete_inputs = 0;
	};

	struct ContinuousInputEvent
	{
		CRSTu64 absolute_time_nano;

		InputChannel channel;
		CRSTf32 value;
	};

	struct DiscreteInputEvent
	{
		CRSTu64 absolute_time_nano;

		InputBit bit;
		CRSTbool is_down;
	};

}
namespace Crystal::Input
{
	class InputBufferBase
	{
	public:
		explicit InputBufferBase(CRSTu64 capacity) : capacity(capacity) {}
		virtual ~InputBufferBase() = default;
		CRST_NON_COPYABLE(InputBufferBase)
		//==============================================================================
		virtual CRSTbool push(const DiscreteInputEvent& event) noexcept = 0;
		virtual CRSTbool pop(DiscreteInputEvent& event) noexcept = 0;

		const CRSTu64 capacity;
	};
	class LockFreeInputBuffer final : public InputBufferBase
	{
	public:
		//==============================================================================
		LockFreeInputBuffer(CRSTu64 capacity_)
			: InputBufferBase(capacity_), mask(capacity_ - 1), discrete_buffer(std::make_unique<DiscreteInputEvent[]>(capacity_))
		{
			CRST_ASSERT(capacity_ > 0, "Capacity must be greater than 0");
			CRST_ASSERT((capacity_ & (capacity_ - 1)) == 0, "Capacity must be a power of two");
		}
		~LockFreeInputBuffer() override = default;
		//==============================================================================
		CRSTbool push(const DiscreteInputEvent& event) noexcept override
		{
			const CRSTu64 current_push_count = push_count.load(std::memory_order_relaxed);
			/* forbid write operation go upside */
			const CRSTu64 current_pop_count = pop_count.load(std::memory_order_acquire);

			if ((current_push_count - current_pop_count) >= capacity) return false;

			/* write */
			discrete_buffer[current_push_count & mask] = event;

			/* forbid read operation go downside */
			push_count.store(current_push_count + 1, std::memory_order_release);
			return true;
		}
		CRSTbool pop(DiscreteInputEvent& event) noexcept override
		{
			const CRSTu64 current_pop_count = pop_count.load(std::memory_order_relaxed);
			/* forbid read operation go upside */
			const CRSTu64 current_push_count = push_count.load(std::memory_order_acquire);
			if (current_push_count == current_pop_count) return false;

			/* read */
			event = discrete_buffer[current_pop_count & mask];

			/* forbid read operation go downside */
			pop_count.store(current_pop_count + 1, std::memory_order_release);
			return true;
		}

	private:
		//==============================================================================
		const CRSTu64 mask;

		std::unique_ptr<DiscreteInputEvent[]> discrete_buffer;

		//==============================================================================
		alignas(64) std::atomic<CRSTu64> push_count{ 0 };
		alignas(64) std::atomic<CRSTu64> pop_count{ 0 };
	};
}
namespace Crystal::Input
{
	class InputBase
	{
	public:
		//==============================================================================
		explicit InputBase(CRSTu64 capacity)
			: discrete_buffer(std::make_unique<LockFreeInputBuffer>(capacity))
		{

		}
		virtual ~InputBase() = default;
		CRST_NON_COPYABLE(InputBase)
		//==============================================================================
		void recordContinuousInput(const ContinuousInputEvent& event) noexcept
		{
			continuous_channels[static_cast<CRSTu8>(event.channel)].value.store(
				event.value,std::memory_order_relaxed);
			continuous_channels[static_cast<CRSTu8>(event.channel)].absolute_time_nano.store(
				event.absolute_time_nano, std::memory_order_relaxed);
		}
		void recordDiscreteInput(const DiscreteInputEvent& event) noexcept
		{
			discrete_buffer->push(event);
		}
		[[nodiscard]] InputState fetchInputState(const Time::TimePoint& current_frame_time) noexcept
		{
			InputState snapshot;
			snapshot.time_point = current_frame_time;

			CRSTu64 max_hardware_nano = 0;
			DiscreteInputEvent discrete_event;
			while (discrete_buffer->pop(discrete_event))
			{
				const CRSTu8 bit_pos = static_cast<CRSTu8>(discrete_event.bit);

				snapshot.discrete_inputs.set(bit_pos, discrete_event.is_down);
				if (discrete_event.absolute_time_nano > max_hardware_nano)
				{
					max_hardware_nano = discrete_event.absolute_time_nano;
				}
			}

			for (size_t i = 1; i < 16; ++i) [[likely]]
			{
				auto val = continuous_channels[i].value.load(std::memory_order_relaxed);
				auto nano = continuous_channels[i].absolute_time_nano.load(std::memory_order_relaxed);

				snapshot.continuous_inputs[i] = val;

				if (nano > max_hardware_nano) max_hardware_nano = nano;
			}
			return snapshot;
		}


	protected:
		//==============================================================================
		struct alignas(64) ContinuousChannel
		{
			std::atomic<CRSTf32> value{ 0.0f };
			std::atomic<CRSTu64> absolute_time_nano{ 0 };
		};

		std::array<ContinuousChannel, 16> continuous_channels;
		std::unique_ptr<InputBufferBase>  discrete_buffer;
	};
}
namespace Crystal::Input
{
	std::unique_ptr<InputBase> createInputAdapter(CRSTu64 capacity);
}

