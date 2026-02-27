#pragma once

class SessionReaper : public Actor
{
public:

	SessionReaper();
	virtual ~SessionReaper() override = default;

	void ReapSession(const WeakSessionRef& pWeakSession);
};

