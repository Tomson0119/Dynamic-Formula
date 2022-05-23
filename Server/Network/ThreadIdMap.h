#pragma once
#include <thread>
#include <map>
#include <cassert>

class ThreadIdMap
{
public:
	static ThreadIdMap& GetInstance()
	{
		static ThreadIdMap instance;
		return instance;
	}

private:
	ThreadIdMap() = default;
	ThreadIdMap(const ThreadIdMap&) = delete;
	ThreadIdMap& operator=(const ThreadIdMap&) = delete;
	ThreadIdMap(ThreadIdMap&&) = delete;
	ThreadIdMap& operator=(ThreadIdMap&&) = delete;

public:
	void AssignId(const std::thread::id& threadId, int idx)
	{
		mIdMap.insert({ threadId, idx });
	}

	int GetId(const std::thread::id& threadId)
	{
		assert(mIdMap.find(threadId) != mIdMap.end());
		return mIdMap[threadId];
	}
	
private:
	std::map<std::thread::id, int> mIdMap;
};