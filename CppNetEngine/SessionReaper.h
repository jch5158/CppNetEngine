#pragma once

class SessionReaper : public Actor
{
public:

	SessionReaper();
	virtual ~SessionReaper() override = default;

private:

	PriorityQueue<WeakSessionRef> mExpiredQueue;
};

