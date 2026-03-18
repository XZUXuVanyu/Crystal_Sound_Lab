export module Crystal_Core.AudioNodes;
export namespace Crystal
{
	class Audio_Node
	{
	public:
		virtual ~Audio_Node() = 0;
		virtual void Process() = 0;
	private:

	};
}