////////////////////////////////////////////////////////////////////////////////
// Filename: main.cpp
////////////////////////////////////////////////////////////////////////////////
#include "systemclass.h"
#include <thread>


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
	SystemClass* System;
	bool result;


	// Create the system object.
	System = new SystemClass;
	if (!System)
	{
		return 0;
	}

	// Fire up the autosave thread.
	std::thread thread_to_save_file([=]()
	{
		System->Autosave();
	});

	// Initialize and run the system object.
	result = System->Initialize();
	if (result)
		System->Run();

	// Something failed to initialize. Die. Horribly.
	else
		return -1;

	// Shutdown the system object.   
	System->Shutdown();
	
	// Wait until the autosave thread finishes.	
	thread_to_save_file.join();

	// Release the main class.
	delete System;
	System = 0;

	// Success! We did it!
	return 0;
}