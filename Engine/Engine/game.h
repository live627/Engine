#pragma once


//////////////
// INCLUDES //
////////////// 
#include <fstream>


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