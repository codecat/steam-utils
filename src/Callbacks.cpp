#include "Common.h"
#include "Callbacks.h"

void UtilsCallbacks::Prepare()
{
	m_waiting = true;
}

void UtilsCallbacks::Wait()
{
	while (m_waiting) {
		usleep(1000);
		SteamAPI_RunCallbacks();
	}
}

void UtilsCallbacks::Done()
{
	m_waiting = false;
}
