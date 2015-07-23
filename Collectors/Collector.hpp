/*
 * Collector.hpp
 *
 *  Created on: 2013-09-04
 *      Author: GarCoSim
 */

#ifndef COLLECTOR_HPP_
#define COLLECTOR_HPP_

#include "../Allocators/Allocator.hpp"
#include "../Main/ObjectContainer.hpp"
#include <queue>
#include <stack>
#include <string>
#include <stdlib.h>
#include <ctime>
#include "../defines.hpp"
#include <stdio.h>
#include <vector>

using namespace std;

namespace traceFileSimulator {

class MemoryManager;

class Collector {
public:
	Collector();
	void setEnvironment(Allocator* allocator, ObjectContainer* container, MemoryManager* memManager, int watermark, int generation, int traversal);
	virtual ~Collector();
	virtual void collect(int reason);
	virtual void checkWatermark();
	void printStats();
	virtual int promotionPhase();
	void lastStats();
	bool hasThreadRegions() {
		return isRegionBased;
	};
	void addThread(int thread) {
		gcThreads.push_back(thread);
	}
	void clearThreadList() {
		gcThreads.clear();
	}
	void setMaxThreads(int threads) {
		maxThreads = threads;
	}

	void getThreadGroup(vector<vector<int> > tg) {
		threadGroup = tg;
	}
	int howManyThreadGroups() {
		unsigned int i;
		int j = 0;

		for (i = 0; i < threadGroup.size(); i++)
			if (!threadGroup.at(i).empty())
				j++;

		return j;
	}
	int howManyThreadsInGroups() {
		unsigned int i;
		int j = 0;

		for (i = 0; i < threadGroup.size(); i++)
			if (!threadGroup.at(i).empty())
				j += threadGroup.at(i).size();

		return j;
	}


protected:
	void postCollect();

	Allocator* myAllocator;
	ObjectContainer* myObjectContainer;
	queue<Object *> myQueue;
	stack<Object *> myStack;
	
	int statGcNumber;
	int statFreedObjects;
	int statLiveObjectCount;
	int statFreeSpaceOnHeap;
	int statFreeSpaceFragmentCount;
	int statCollectionReason;
	int myWatermark;
	int myGeneration;
	int statFreedDuringThisGC;
	int gcsSinceLastPromotionPhase;
	int myTraversal;
	int statHeapSide;
	double shortestGC;
	double longestGC;
	double allGCs;

	bool isRegionBased;
	
	MemoryManager* myMemManager;
	int maxThreads;
	vector<int> gcThreads;

	vector<vector<int> > threadGroup;

	traversalEnum order;
};

} 
#endif /* COLLECTOR_HPP_ */
