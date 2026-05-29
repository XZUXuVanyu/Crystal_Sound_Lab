//==============================================================================
#pragma once
#include <array>
#include <memory>
#include <CRST_Core/CRST_Core.h>
#include <CRST_Time/CRST_Time.h>
#include "InputBuffer.h"
//==============================================================================
namespace Crystal::Input
{
	enum class InputChannel : CRSTu8
	{
		None = 0,
		MouseX = 1,
		MouseY = 2,
		MouseMiddleUp = 3,
		MouseMiddleDown = 4
	};
	enum class InputBit : CRSTu8
	{
		MouseLeft = 0,
		MouseRight = 1,
		MouseMiddle = 2,
		KeyShift = 3,
		KeyCtrl = 4,
		KeyAlt = 5,
		KeyW = 6,
		KeyA = 7,
		KeyS = 8,
		KeyD = 9,
		KeySpace = 10,
		KeyEscape = 11,
		KeyEnter = 12,
		Action0 = 15,
		Action1 = 16
	};
	/**
	 * @brief 纯值语义的输入状态快照
	 * @note 满足内存对齐，便于后续向 AI 训练框架提取张量
	 */
	struct alignas(16) InputState
	{
		Time::TimePoint time_point;
		std::array<CRSTf32, 16> continuous_inputs;
		CRSTu64 discrete_inputs_h;
		CRSTu64 discrete_inputs_l;

		[[nodiscard]] CRSTbool isInputActive(InputBit bit) const noexcept
		{
			const CRSTu8 bit_pos = static_cast<CRSTu8>(bit);
			if (bit_pos < 64) { return (discrete_inputs_l & (1ULL << bit_pos)) != 0; }
			else { return (discrete_inputs_h & (1ULL << (bit_pos - 64))) != 0; }
		}
		void setInputBit(InputBit bit, CRSTbool active) noexcept
		{
			const CRSTu8 bit_pos = static_cast<CRSTu8>(bit);
			const CRSTu64 mask = 1ULL << (bit_pos % 64);

			if (bit_pos < 64)
			{
				if (active) discrete_inputs_l |= mask;
				else discrete_inputs_l &= ~mask;
			}
			else
			{
				if (active) discrete_inputs_h |= mask;
				else discrete_inputs_h &= ~mask;
			}
		}
	};
}
namespace Crystal::Input
{
	class InputBase
	{
	public:
		//==============================================================================
		explicit InputBase(CRSTu64 capacity)
			: shared_discrete(std::make_unique<LockFreeInputBuffer>(capacity)),
			persistent_discrete_l(0), persistent_discrete_h(0)
		{

		}
		virtual ~InputBase() = default;
		CRST_NON_COPYABLE(InputBase)
		//==============================================================================
		void recordContinuousInput(InputChannel channel, CRSTf32 value) noexcept
		{
			const CRSTu8 idx = static_cast<CRSTu8>(channel);
			CRST_ASSERT(idx < 16, "Not a valid input channel");
			shared_continuous[idx].store(value, std::memory_order_relaxed);
		}
		void recordDiscreteInput(InputBit bit, CRSTbool is_down) noexcept
		{
			const CRSTu8 bit_pos = static_cast<CRSTu8>(bit);
			CRST_ASSERT(bit_pos < 128, "Not a valid input bit");

			shared_discrete->push(InputEvent{ .bit = bit, .is_down = is_down });
		}
		[[nodiscard]] InputState fetchInputState() noexcept
		{
			InputState local_state{};

			for (CRSTu16 i = 0; i < 16; ++i)
			{
				local_state.continuous_inputs[i] = shared_continuous[i].load(std::memory_order_relaxed);
			}

			local_state.discrete_inputs_l = persistent_discrete_l;
			local_state.discrete_inputs_h = persistent_discrete_h;

			InputEvent event;
			while (shared_discrete->pop(event))
			{
				const CRSTu8 bit_pos = static_cast<CRSTu8>(event.bit);
				const CRSTu64 mask = 1ULL << (bit_pos % 64);

				if (bit_pos < 64)
				{
					if (event.is_down)
					{
						local_state.discrete_inputs_l |= mask;
					}
					else
					{
						local_state.discrete_inputs_l &= ~mask;
					}
				}
				else
				{
					if (event.is_down)
					{
						local_state.discrete_inputs_h |= mask;
					}
					else
					{
						local_state.discrete_inputs_h &= ~mask;
					}
				}
			}

			persistent_discrete_l = local_state.discrete_inputs_l;
			persistent_discrete_h = local_state.discrete_inputs_h;

			return local_state;
		}

	protected:
		//==============================================================================
		alignas(64) std::array<std::atomic<CRSTf32>, 16> shared_continuous{};
		std::unique_ptr<InputBufferBase> shared_discrete = nullptr;
		//==============================================================================
		CRSTu64 persistent_discrete_l;
		CRSTu64 persistent_discrete_h;
	};
}
namespace Crystal::Input
{
	std::unique_ptr<InputBase> createInputAdapter(CRSTu64 capacity);
}
