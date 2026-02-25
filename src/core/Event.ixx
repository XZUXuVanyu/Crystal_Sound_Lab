module;
#include <mutex>
#include <queue>
#include <variant>
export module CrystalCore.Event;
import CrystalCore.CoreUtils;
import CrystalCore.Contexts;
import CrystalCore.Signal;
import CrystalApplications.GraphicManager;

// todo: add multithread support

export namespace CrystalCore
{
	// To define new event type, add struct EventTypeName { params; };
	struct EventType
	{
		struct AudioSpecChange { CrystalCore::AudioContext context; };
		struct WindowSpecChange { CrystalCore::WindowContext context; };
		struct SetPlayBackSpeed { float new_speed; };
		struct test {};
	};

	using EventData = std::variant<EventType::AudioSpecChange,\
		EventType::WindowSpecChange, EventType::SetPlayBackSpeed,\
		EventType::test>;

	struct Event 
	{
		EventData data;
	};

	class EventManager
	{
	public:
		void pushEvent(EventData data) 
		{
			std::lock_guard<std::mutex> lock(mtx);
			event_queue.push({ data });
		}

		bool pollEvent(Event& out_ev) 
		{
			std::lock_guard<std::mutex> lock(mtx);
			if (event_queue.empty()) return false;

			out_ev = event_queue.front();
			event_queue.pop();
			return true;
		}

	private:
		std::queue<Event> event_queue;
		std::mutex mtx;
	};
}
