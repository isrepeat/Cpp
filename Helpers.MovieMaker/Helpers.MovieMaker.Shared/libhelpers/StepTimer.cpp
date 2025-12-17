#include "pch.h"
#include "StepTimer.h"

StepTimer::StepTimer() :
	m_elapsedTicks(0),
	m_totalTicks(0),
	m_leftOverTicks(0),
	m_frameCount(0),
	m_framesPerSecond(0),
	m_framesThisSecond(0),
	m_qpcSecondCounter(0),
	m_isFixedTimeStep(false),
	m_targetElapsedTicks(TicksPerSecond / 60)
{
	if (!QueryPerformanceFrequency(&m_qpcFrequency)) {
		throw std::exception();
	}

	if (!QueryPerformanceCounter(&m_qpcLastTime)) {
		throw std::exception();
	}

	// Initialize max delta to 1/10 of a second.
	m_qpcMaxDelta = m_qpcFrequency.QuadPart / 10;
}

// Get elapsed time since the previous Update call.
uint64_t StepTimer::GetElapsedTicks() const { 
	return m_elapsedTicks; 
}

double StepTimer::GetElapsedSeconds() const { 
	return TicksToSeconds(m_elapsedTicks); 
}

// Get total time since the start of the program.
uint64_t StepTimer::GetTotalTicks() const { 
	return m_totalTicks; 
}

double StepTimer::GetTotalSeconds() const { 
	return TicksToSeconds(m_totalTicks); 
}

// Get total number of updates since start of the program.
uint32_t StepTimer::GetFrameCount() const { 
	return m_frameCount; 
}

// Get the current framerate.
uint32_t StepTimer::GetFramesPerSecond() const { 
	return m_framesPerSecond; 
}

// Set whether to use fixed or variable timestep mode.
void StepTimer::SetFixedTimeStep(bool isFixedTimestep) { 
	m_isFixedTimeStep = isFixedTimestep; 
}

// Set how often to call Update when in fixed timestep mode.
void StepTimer::SetTargetElapsedTicks(uint64_t targetElapsed) { 
	m_targetElapsedTicks = targetElapsed; 
}

void StepTimer::SetTargetElapsedSeconds(double targetElapsed) { 
	m_targetElapsedTicks = SecondsToTicks(targetElapsed); 
}

double StepTimer::TicksToSeconds(uint64_t ticks) { 
	return static_cast<double>(ticks) / TicksPerSecond; 
}

uint64_t StepTimer::SecondsToTicks(double seconds) { 
	return static_cast<uint64_t>(seconds * TicksPerSecond); 
}

// After an intentional timing discontinuity (for instance a blocking IO operation)
// call this to avoid having the fixed timestep logic attempt a set of catch-up 
// Update calls.

void StepTimer::ResetElapsedTime() {
	if (!QueryPerformanceCounter(&m_qpcLastTime)) {
		throw std::exception();
	}

	m_leftOverTicks = 0;
	m_framesPerSecond = 0;
	m_framesThisSecond = 0;
	m_qpcSecondCounter = 0;
}