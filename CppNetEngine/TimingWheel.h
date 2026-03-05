#pragma once
#include <utility>

#include "SharedPtrUtils.h"

class TimingWheel final
{
private:

	class TimingNode final
	{
	public:
		TimingNode(const TimingNode& rhs) noexcept;
		TimingNode& operator=(const TimingNode& rhs) noexcept;
		TimingNode(TimingNode&& rhs) noexcept;
		TimingNode& operator=(TimingNode&& rhs) noexcept;

		explicit TimingNode(const uint64 expireTick, std::function<void()> pCallback);
		~TimingNode() = default;

		[[nodiscard]] uint64 GetExpiredTick() const;
		void Execute() const;

	private:

		uint64 mExpireTick;
		std::function<void()> mpCallback;
	};

	static constexpr uint32 LEVEL0_BITS = 8; // 256칸 (0xFF)
	static constexpr uint32 LEVEL1_BITS = 8; // 256칸 (0xFF)
	static constexpr uint32 LEVEL2_BITS = 6; // 64칸 (0x3F)

	static constexpr uint32 LEVEL0_SIZE = 1 << LEVEL0_BITS;
	static constexpr uint32 LEVEL1_SIZE = 1 << LEVEL1_BITS;
	static constexpr uint32 LEVEL2_SIZE = 1 << LEVEL2_BITS;

	static constexpr uint32 LEVEL0_MASK = LEVEL0_SIZE - 1;
	static constexpr uint32 LEVEL1_MASK = LEVEL1_SIZE - 1;
	static constexpr uint32 LEVEL2_MASK = LEVEL2_SIZE - 1;

public:

	static constexpr uint64 DEFAULT_TICK_INTERVAL = 16;

	TimingWheel(const TimingWheel&) = delete;
	TimingWheel& operator=(const TimingWheel&) = delete;
	TimingWheel(TimingWheel&&) = delete;
	TimingWheel& operator=(TimingWheel&&) = delete;

	explicit TimingWheel(const uint64 tickInterval = DEFAULT_TICK_INTERVAL);
	~TimingWheel() = default;

	void AddTiming(std::function<void()> pCallback, const uint64 delayMs);
	void Tick();

private:

	void processTick(List<TimingNode>& outExecuteList);
	void addNode(TimingNode&& node);
	void cascade(List<TimingNode>& bucket);

	std::atomic<bool> mbTicking;
	const uint64 mTickIntervalMs;

	Mutex mLock;
	uint64 mCurrentTick;
	std::chrono::steady_clock::time_point mLastTickTime;
	Vector<List<TimingNode>> mWheel0;
	Vector<List<TimingNode>> mWheel1;
	Vector<List<TimingNode>> mWheel2;
};