#include "pch.h"
#include "Job.h"

void Job::Execute() const
{
	mJobFunc();
}
