///////////////////////////////////////////////////////////////////////////////
// Filename: cpuclass.h
///////////////////////////////////////////////////////////////////////////////
#ifndef _CPUCLASS_H_
#define _CPUCLASS_H_


/////////////
// LINKING //
/////////////
#pragma comment(lib, "winmm.lib")


//////////////
// INCLUDES //
//////////////
#include <timeapi.h>


///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "game.h"


///////////////////////////////////////////////////////////////////////////////
// Class name: CpuClass
///////////////////////////////////////////////////////////////////////////////
class CpuClass : public IGameObject
{
public:
	CpuClass(InputClass * p_Input, CameraClass * p_Camera, TextClass * p_Text)
		:
		m_Input(p_Input),
		m_Camera(p_Camera),
		m_Text(p_Text)
	{
		m_startTime = timeGetTime();

		// Check to see if this system supports high performance timers.
		QueryPerformanceFrequency((LARGE_INTEGER*)&m_frequency);
		if (m_frequency != 0)
		{
			// Find out how many times the frequency counter ticks every millisecond.
			m_ticksPerMs = (int)(m_frequency / 1000);

			QueryPerformanceCounter((LARGE_INTEGER*)&m_frameStartTime);
		}
	}

	void Frame()
	{
		int64_t currentTime;
		int timeDifference;


		QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);

		timeDifference = currentTime - m_frameStartTime;

		m_count++;

		if (m_startTime + 1000 < timeGetTime())
		{
			m_fps = m_count;
			m_count = 0;

			m_cpuUsage = static_cast<int>(GetCPULoad() * 100);
			m_startTime = timeGetTime();
			m_frameTime = timeDifference / m_ticksPerMs;
		}

		m_frameStartTime = currentTime;
		UpdateDebugInfo();
	}
	void UpdateDebugInfo()
	{
		int mouseX, mouseY;

		// Get the location of the mouse from the input object,
		m_Input->GetMousePositionForDebug(mouseX, mouseY);
		m_Text->SetMousePosition(mouseX, mouseY);
		m_Text->SetFps(GetFps(), GetFrameTimeDelta());
		m_Text->SetCpu(GetCpuPercentage());
		m_Text->SetCameraPosition(m_Camera->GetPosition());
	}


	unsigned int GetFps() const { return m_fps; }
	unsigned int GetCpuPercentage() const { return m_cpuUsage; }
	unsigned int GetFrameTimeDelta() { return m_frameTime; }

private:
	unsigned int
		m_fps = 0,
		m_count = 0,
		m_startTime = 0,
		m_cpuUsage = 0,
		m_frameTime = 0;
	InputClass * m_Input;
	CameraClass * m_Camera;
	TextClass * m_Text;
	int64_t
		m_frequency,
		m_frameStartTime;
	int m_ticksPerMs;

	float CalculateCPULoad(uint64_t idleTicks, uint64_t totalTicks) const
	{
		static uint64_t previousTotalTicks = 0;
		static uint64_t previousIdleTicks = 0;

		uint64_t totalTicksSinceLastTime = totalTicks - previousTotalTicks;
		uint64_t idleTicksSinceLastTime = idleTicks - previousIdleTicks;

		previousTotalTicks = totalTicks;
		previousIdleTicks = idleTicks;

		return
			1.0f - (totalTicksSinceLastTime > 0
				? static_cast<float>(idleTicksSinceLastTime) / totalTicksSinceLastTime
				: 0);
	}

	uint64_t FileTimeToInt64(const FILETIME & ft) const
	{
		return
			(static_cast<uint64_t>(ft.dwHighDateTime) << 32)
			| static_cast<uint64_t>(ft.dwLowDateTime);
	}

	// Returns 1.0f for "CPU fully pinned", 0.0f for "CPU idle", or somewhere in between
	// You'll need to call this at regular intervals, since it measures the load between
	// the previous call and the current one.  Returns -1.0 on error.
	float GetCPULoad() const
	{
		FILETIME idleTime, kernelTime, userTime;

		return
			GetSystemTimes(&idleTime, &kernelTime, &userTime)
				? CalculateCPULoad(
					FileTimeToInt64(idleTime),
					FileTimeToInt64(kernelTime) + FileTimeToInt64(userTime)
				)
				: -1.0f;
	}
};

#endif