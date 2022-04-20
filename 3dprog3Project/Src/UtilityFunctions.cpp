#include "pch.h"

#include "UtilityFunctions.h"

namespace utl
{
	void PrintDebug(const std::string& text)
	{
#ifdef _DEBUG
		std::cout << text << std::endl;
#endif // _DEBUG
	}
	void PrintDebug(const std::wstring& text)
	{
#ifdef _DEBUG
		std::wcout << text << std::endl;
#endif // _DEBUG
	}

	//lol this is funny
	std::wstring StringToWString(const std::string& str)
	{
		return std::filesystem::path(str).wstring();
	}
	std::string WStringToString(const std::wstring& wstr)
	{
		return std::filesystem::path(wstr).string();
	}
}
