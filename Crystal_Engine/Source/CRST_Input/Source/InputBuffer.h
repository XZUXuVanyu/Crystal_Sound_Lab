//==============================================================================
#pragma once
#include <atomic>
#include <memory>
#include <CRST_Core/CRST_Core.h>
//==============================================================================
namespace Crystal::Input
{
	enum class InputBit : CRSTu8;
	struct InputEvent
    {
        InputBit bit;
        CRSTbool is_down;
    };
}

namespace Crystal::Input
{
    class InputBufferBase
    {
    public:
        //==============================================================================
        explicit InputBufferBase(CRSTu64 capacity) : capacity(capacity) {}
        virtual ~InputBufferBase() = default;
        CRST_NON_COPYABLE(InputBufferBase)
    	//==============================================================================
    	virtual CRSTbool push(const InputEvent& event) noexcept = 0;
        virtual CRSTbool pop(InputEvent& event) noexcept = 0;
        //==============================================================================
        const CRSTu64 capacity;
    };
    class LockFreeInputBuffer final : public InputBufferBase
    {
    public:
        //==============================================================================
        LockFreeInputBuffer(CRSTu64 capacity_)
    	: InputBufferBase(capacity_), mask(capacity_ - 1), buffer(std::make_unique<InputEvent[]>(capacity_))
        {
            CRST_ASSERT(capacity_ > 0, "Capacity must be greater than 0");
            CRST_ASSERT((capacity_ & (capacity_ - 1)) == 0, "Capacity must be a power of two");
        }
        ~LockFreeInputBuffer() override = default;
        //==============================================================================
        CRSTbool push(const InputEvent& event) noexcept override
        {
            const CRSTu64 current_push_count = push_count.load(std::memory_order_relaxed);
            /* forbid write operation go upside */
            const CRSTu64 current_pop_count = pop_count.load(std::memory_order_acquire);

            if ((current_push_count - current_pop_count) >= capacity) return false;

            /* write */
            buffer[current_push_count & mask] = event;

            /* forbid read operation go downside */
            push_count.store(current_push_count + 1, std::memory_order_release);
            return true;
        }
        CRSTbool pop(InputEvent& event) noexcept override
        {
            const CRSTu64 current_pop_count = pop_count.load(std::memory_order_relaxed);
            /* forbid read operation go upside */
            const CRSTu64 current_push_count = push_count.load(std::memory_order_acquire);
            if (current_push_count == current_pop_count) return false;

            /* read */
            event = buffer[current_pop_count & mask];

            /* forbid read operation go downside */
            pop_count.store(current_pop_count + 1, std::memory_order_release);
            return true;
        }

    private:
        //==============================================================================
        const CRSTu64 mask;
        std::unique_ptr<InputEvent[]> buffer;
        //==============================================================================
        alignas(64) std::atomic<CRSTu64> push_count{ 0 };
        alignas(64) std::atomic<CRSTu64> pop_count{ 0 };
    };
}