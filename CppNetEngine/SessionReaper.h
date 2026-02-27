#pragma once
#include "Session.h"

class SessionReaper : public Actor
{
public:

	SessionReaper() = default;
	virtual ~SessionReaper() override = default;

	static void ReapSession(const WeakSessionRef& pWeakSession);
};

