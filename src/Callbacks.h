#pragma once

class UtilsCallbacks
{
public:
	bool m_waiting;

public:
	void Prepare();
	void Wait();
	void Done();
};
