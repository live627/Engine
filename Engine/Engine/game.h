#pragma once


//////////////
// INCLUDES //
////////////// 
#include <fstream>
#include <Windows.h>


// Let's not rely on minwin.
using uint = unsigned int;
using byte = unsigned char;


////////////////////////////////////////////////////////////////////////////////
// Class name: GameObject
////////////////////////////////////////////////////////////////////////////////
class GameObject
{
public:
	virtual ~GameObject() {}
	virtual bool Initialize() = 0;
	virtual void Shutdown() {}
	virtual void Frame() = 0;
	virtual bool Render() { return true; }
	virtual void Save(std::ofstream&) {}
	virtual void Load(std::ifstream&) {}
};


class ui
{
public:
	// how much to scale a design that assumes 96-DPI pixels
	static double scaleX;
	static double scaleY;

	static const double ScaleX(const double argX)
	{
		return argX * scaleX;
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


#include <vector>    
#include <algorithm> 
#include <functional>
#include <memory> 


template<typename FuncTemplate>
class Event
{
public:
	class Delegate
	{
	private:
		typedef std::function<FuncTemplate> Func;

	public:
		Delegate(const Func& func) : functionPtr(std::make_shared<Func>(func)) {}

		inline bool operator== (const Delegate& other) const
		{
			return functionPtr.get() == other.functionPtr.get();
		}

		template<typename... Args>
		void operator()(Args&&... args)
		{
			(*functionPtr)(std::forward<Args>(args)...);
		}

	private:
		std::shared_ptr<Func> functionPtr;
	};

private:
	std::vector<Delegate> delegates;

public:
	Event() = default;
	~Event() = default;

	Event& operator+=(const Delegate& func)
	{
		delegates.push_back(func);

		return *this;
	}

	/**
	* Removes the first occurence of the given delegate from the call queue.
	*/
	Event& operator-=(const Delegate& func)
	{
		auto index = std::find(delegates.begin(), delegates.end(), func);
		if (index != delegates.end())
			delegates.erase(index);

		return *this;
	}

	/**
	* Fires this event.
	*
	* @param args Arguments to be passed to the called functions. Must have the exact same
	* number of arguments as the given event template.
	*/
	template<typename... Args>
	void operator()(Args&&... args)
	{
		for (auto delegate : delegates)
			delegate(std::forward<Args>(args)...);
	}
};