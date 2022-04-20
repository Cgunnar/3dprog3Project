#pragma once

namespace utl
{
	void PrintDebug(const std::string& text);
	void PrintDebug(const std::wstring& text);

	inline uint64_t GenerateRandomID()
	{
		std::random_device rdev;
		std::mt19937 gen(rdev());
		std::uniform_int_distribution<int64_t> udis(1, INT64_MAX - 1);
		return udis(gen);
	}

	inline size_t AlignSize(size_t offset, size_t alignment)
	{
		assert(!(0 == alignment) || (alignment & (alignment - 1)));
		return ((offset + (alignment - 1)) & ~(alignment - 1));
	}

	inline void* AlignAdress(void* adress, size_t alignment)
	{
		return (void*)AlignSize((size_t)adress, alignment);
	}

	std::wstring StringToWString(const std::string& str);
	std::string WStringToString(const std::wstring& wstr);
}