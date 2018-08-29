#pragma once


//////////////
// INCLUDES //
////////////// 
#include <fstream>
#include <algorithm> 
#include <memory> 
#include <vector> 
#include <Windows.h>


///////////////////////////////////////////////////////////////////////////////
// BinaryReader
///////////////////////////////////////////////////////////////////////////////
class BinaryReader
{
	std::istream &io;

public:
	BinaryReader(std::istream &io)
		: 
		io(io)
	{}

	std::string GetString()
	{
		std::vector<char> buf(Get<size_t>());
		io.read(&buf[0], buf.size());
		return std::string(buf.begin(), buf.end());
	}

	void Read(void *p, size_t size)
	{
		io.read(reinterpret_cast<char *>(p), size);
	}

	template<typename T>
	T Get()
	{
		T buf;
		io.read(reinterpret_cast<char *>(&buf), sizeof T);
		return buf;
	}
};


////////////////////////////////////////////////////////////////////////////////
// Class name: IGameObject
////////////////////////////////////////////////////////////////////////////////
class IGameObject
{
public:
	virtual ~IGameObject() {}
	virtual bool Initialize() = 0;
	virtual void Shutdown() {}
	virtual void Frame() = 0;
	virtual bool Render() { return true; }
	virtual void Save(BinaryWriter &) {}
	virtual void Load(BinaryReader &) {}
};


class ui
{
public:
	// how much to scale a design that assumes 96-DPI pixels
	static double scaleX;
	static double scaleY;

	template<typename T>
	inline static constexpr T ScaleX(const T argX)
	{
		return static_cast<T>(argX * scaleX);
	}
};


// Helper class for COM exceptions
class com_exception : public std::runtime_error
{
public:
	com_exception(const HRESULT hr, const char * msg)
		:
		std::runtime_error(msg),
		result(hr)
	{}

	virtual const char* what() const override
	{
		LPSTR errorText = NULL;
		FormatMessageA(
			// use system message tables to retrieve error text
			FORMAT_MESSAGE_FROM_SYSTEM
			// allocate buffer on local heap for error text
			| FORMAT_MESSAGE_ALLOCATE_BUFFER
			// Important! will fail otherwise, since we're not 
			// (and CANNOT) pass insertion parameters
			| FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,    // unused with FORMAT_MESSAGE_FROM_SYSTEM
			result,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPSTR)&errorText,  // output 
			0, // minimum size for output buffer
			NULL);   // arguments - see note 

		static char s_str[1064] = {};
		if (NULL != errorText)
		{
			sprintf_s(s_str, "Failure with HRESULT of 0X%08X:\n\n%s\n\n%s",
				static_cast<unsigned int>(result),
				errorText,
				std::runtime_error::what()
			);

			// release memory allocated by FormatMessage()
			LocalFree(errorText);
			errorText = NULL;
		}
		else
		{
			sprintf_s(s_str, "Failure with HRESULT of 0X%08X:\n\n%s",
				static_cast<unsigned int>(result),
				std::runtime_error::what()
			);
		}

		return s_str;
	}

private:
	HRESULT result;
};

// Helper utility converts D3D API failures into exceptions.
inline void ThrowIfFailed(const HRESULT hr, const char * msg)
{
	if (FAILED(hr))
		throw com_exception(hr, msg);
}

// snprintf with automatic string size measurement.
template<typename... Args>
auto FormatString(const char * fmt, Args... args)
{
	size_t sz = std::snprintf(nullptr, 0, fmt, args...);
	std::vector<char> buf(sz + 1); // note +1 for null terminator
	std::snprintf(&buf[0], buf.size(), fmt, args...);

	return buf;
}