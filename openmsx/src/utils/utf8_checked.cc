// $Id: utf8_checked.cc 11726 2010-10-24 08:09:51Z m9710797 $

#ifdef _WIN32

#include "utf8_checked.hh"
#include "vla.hh"
#include "StringOp.hh"
#include "MSXException.hh"
#include <windows.h>

namespace utf8 {

bool multibytetoutf16(const std::string& multibyte, UINT cp, DWORD dwFlags, std::wstring& utf16)
{
	const char* multibyteA = multibyte.c_str();
	int len = MultiByteToWideChar(cp, dwFlags, multibyteA, -1, NULL, 0);
	if (len) {
		VLA(wchar_t, utf16W, len);
		len = MultiByteToWideChar(cp, dwFlags, multibyteA, -1, utf16W, len);
		if (len) {
			utf16 = utf16W;
			return true;
		}
	}
	return false;
}

bool utf16tomultibyte(const std::wstring& utf16, UINT cp, std::string& multibyte)
{
	const wchar_t* utf16W = utf16.c_str();
	int len = WideCharToMultiByte(cp, 0, utf16W, -1, NULL, 0, NULL, NULL);
	if (len) {
		VLA(char, multibyteA, len);
		len = WideCharToMultiByte(cp, 0, utf16W, -1, multibyteA, len, NULL, NULL);
		if (len) {
			multibyte = multibyteA;
			return true;
		}
	}
	return false;
}

std::string utf8toansi(const std::string& utf8)
{
	std::wstring utf16;
	if (!multibytetoutf16(utf8, CP_UTF8, MB_ERR_INVALID_CHARS, utf16))
	{
		throw openmsx::FatalError(StringOp::Builder() <<
			"MultiByteToWideChar failed: " << GetLastError());
	}

	std::string ansi;
	if (!utf16tomultibyte(utf16, CP_ACP, ansi))
	{
		throw openmsx::FatalError(StringOp::Builder() <<
			"MultiByteToWideChar failed: " << GetLastError());
	}
	return ansi;
}

std::wstring utf8to16(const std::string& utf8)
{
	std::wstring utf16;
	if (!multibytetoutf16(utf8, CP_UTF8, MB_ERR_INVALID_CHARS, utf16))
	{
		throw openmsx::FatalError(StringOp::Builder() <<
			"MultiByteToWideChar failed: " << GetLastError());
	}
	return utf16;
}

std::string utf16to8(const std::wstring& utf16)
{
	std::string utf8;
	if (!utf16tomultibyte(utf16, CP_UTF8, utf8))
	{
		throw openmsx::FatalError(StringOp::Builder() <<
			"MultiByteToWideChar failed: " << GetLastError());
	}
	return utf8;
}

} // namespace utf8

#endif // _WIN32
