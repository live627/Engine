////////////////////////////////////////////////////////////////////////////////
// Filename: main.cpp
////////////////////////////////////////////////////////////////////////////////
#include "systemclass.h"
#include <iomanip>
#include <ctime>
#pragma warning(disable:4996)


class MyStream : public std::ostream
{
	// Write a stream buffer that prefixes each line with Plop
	class MyStreamBuf : public std::stringbuf
	{
		std::ostream& output;
	public:
		MyStreamBuf(std::ostream& str)
			:
			output(str)
		{}
		~MyStreamBuf() {
			if (pbase() != pptr()) 
			{
				putOutput();
			}
		}

		// Decorate the input stream when we sync the stream with the output.
		virtual int sync() {
			putOutput();
			return 0;
		}
		void putOutput() {
			// Called by destructor.
			// destructor can not call virtual methods.
			std::time_t t = std::time(nullptr);
			std::array<char, 64> buffer;
			strftime(buffer.data(), sizeof(buffer), "[%F %r]", std::localtime(&t));
			output << std::setw(80) << std::setfill('=') << "=" << "\n"
				<< std::setw(80) << std::setfill(' ') << std::right
				<< buffer.data()
				// BUG I cannot Get this to align properly.
				//<< std::put_time(std::localtime(&t), "[%F %r]")
				<< "\n" << str() << "\n\n";

			// Clear the input stream.
			str("");

			// Flush the actual output stream we are using.
			output.flush();
		}
	};

	// My Stream just uses a version of my special buffer
	MyStreamBuf buffer;
public:
	MyStream(std::ostream& str)
		:
		std::ostream(&buffer),
		buffer(str)
	{}
};


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
	// Log stderr to a file.
	// cerr always flushes on every command, so use clog since it only
	// flushes as expected (when explicitly told to or by calling endl).
	std::ofstream ofs("logfile", std::ios::app);
	MyStream myStream(ofs);
	std::clog.rdbuf(myStream.rdbuf());

	// Create the system object.
	SystemClass System;

	// Success! We did it!
	return EXIT_SUCCESS;
}