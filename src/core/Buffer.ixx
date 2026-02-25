module;
#include <cmath>
#include <cstring>
#include <cstddef>
#include <algorithm>
#include <vector>

export module CrystalCore.Signal;

export namespace CrystalCore
{
	template <std::size_t N>
		class Block
	{
	public:
		static constexpr std::size_t BlockSize = 1ULL << N;
		static constexpr std::size_t Alignment = 64;

		Block() = default;

		void*			extractOut() { return data; }
		const void*		extractOut() const { return data; }
		float*			extractOutFloat() { return reinterpret_cast<float*>(data); }
		const float*	extractOutFloat() const { return reinterpret_cast<const float*>(data); }
		void			pumpIn(const void* src) { if (src) std::memcpy(data, src, BlockSize); }

		float& operator[](std::size_t i) { return extractOutFloat()[i]; }
		const float& operator[](std::size_t i) const { return extractOutFloat()[i]; }

		constexpr size_t getSizeinByte() const	{ return BlockSize; }
		constexpr size_t getSizeinFloat() const { return BlockSize / sizeof(float); }
	private:
		alignas(Alignment) std::byte data[BlockSize] {};
	};

	template <std::size_t N>
		Block<N>& operator<<(Block<N>& dst, const Block<N>& src) {
		dst.pumpIn(src.extractOut());
		return dst;
	}

	template <std::size_t N>
		Block<N>& operator<<(Block<N>& dst, const void* src) {
		dst.pumpIn(src);
		return dst;
	}

	template <std::size_t N>
		class TripleBuf
	{
	public:
		void Update(Block<N>& in, Block<N>& out)
		{
			out << bk_in;
			bk_in << bk_mid;
			bk_mid << bk_out;
			bk_out << in;
		}

	private:
		Block<N> bk_in;
		Block<N> bk_mid;
		Block<N> bk_out;
	};

	class DataBlocks
	{
	public:
		explicit DataBlocks(size_t bytes) : buffer(bytes, std::byte{ 0 }) {}

		DataBlocks(const DataBlocks&) = delete;
		DataBlocks& operator=(const DataBlocks&) = delete;
		DataBlocks(DataBlocks&&) noexcept = default;

		void pumpIn(const void* src, size_t bytes)
		{
			size_t to_copy = std::min(bytes, buffer.size());
			if (src) std::memcpy(buffer.data(), src, to_copy);
			if (to_copy < buffer.size()) std::memset(buffer.data() + to_copy, 0, buffer.size() - to_copy);
		}

		void* extractOut() { return buffer.data(); }
		const void* extractOut() const { return buffer.data(); }
		size_t getByteSize() const { return buffer.size(); }

	private:
		std::vector<std::byte> buffer;
	};
}