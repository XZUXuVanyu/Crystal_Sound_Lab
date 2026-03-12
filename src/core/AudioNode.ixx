module;
#include <memory>
#include <vector>
export module Crystal_Core.AudioNode;
import Crystal_Core.Foundation;
export namespace Crystal
{
	class Audio_Node
	{
	public:
		virtual ~Audio_Node() = default;
		virtual void Process(Audio_Buffer& output) = 0;
	};

	class Audio_Processor : public Audio_Node 
	{
	public:
		virtual void Process(Audio_Buffer& output) override = 0;
	};

	class Audio_Pipe : public Audio_Node 
	{
	private:
		std::shared_ptr<Audio_Node> source;
		std::vector<std::shared_ptr<Audio_Node>> fx_chain;

	public:
		void Set_Source(std::shared_ptr<Audio_Node> src) { source = src; }
		void Add_Effect(std::shared_ptr<Audio_Node> fx) { fx_chain.push_back(fx); }

		void Process(Audio_Buffer& output) override 
		{
			if (!source) return;
			source->Process(output);

			for (auto& fx : fx_chain) {
				fx->Process(output);
			}
		}
	};

	class Sine_Source : public Audio_Node
	{
	public:
		~Sine_Source() {};
		void Process(Audio_Buffer& output) override
		{

		}
	};
}