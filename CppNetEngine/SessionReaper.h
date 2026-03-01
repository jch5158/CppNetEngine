#pragma once

class SessionReaper : public Actor
{
public:

	SessionReaper() = default;
	virtual ~SessionReaper() override = default;

	static void ReapSession(const WeakSessionRef& pWeakSession);
};

