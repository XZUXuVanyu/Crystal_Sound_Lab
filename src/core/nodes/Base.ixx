module;
#include <cmath>
#include <memory>
#include <vector>
#include <numbers>
export module Crystal_Core.AudioNode:Base;
import Crystal_Core.Foundation;
export namespace Crystal
{
	export class Audio_Node
	{
	public:
		virtual ~Audio_Node() = default;
		virtual void Process(Audio_Buffer& output) = 0;
	};

	export class Audio_Effects : public Audio_Node
	{
	public:
		virtual void Process(Audio_Buffer& output) override = 0;
	};
	export class Audio_Pipe : public Audio_Node
	{
	public:
		void Process(Audio_Buffer& output) override
		{
			if (!source) return;
			source->Process(output);

			for (auto& fx : fx_chain) {
				fx->Process(output);
			}
		}
		void Set_Source(std::shared_ptr<Audio_Node> src) { source = src; }
		void Add_Effect(std::shared_ptr<Audio_Effects> fx) { fx_chain.push_back(fx); }

	private:
		std::shared_ptr<Audio_Node> source;
		std::vector<std::shared_ptr<Audio_Effects>> fx_chain;
	};
	export class Audio_Source : public Audio_Node
	{
	public:
		virtual void Process(Audio_Buffer& output) override = 0;
	};
}