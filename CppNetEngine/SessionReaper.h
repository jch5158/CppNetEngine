#pragma once
#include "Session.h"

class SessionReaper : public Actor
{
public:

	SessionReaper() = default;
	virtual ~SessionReaper() override = default;

	void ReapSession(const WeakSessionRef& pWeakSession);
};

