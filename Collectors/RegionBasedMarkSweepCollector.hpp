/*
 * RegionBasedMarkSweepCollector.hpp
 *
 *  Created on: 2013-09-04
 *      Author: GarCoSim
 */

#ifndef REGIONBASEDMARKSWEEPCOLLECTOR_HPP_
#define REGIONBASEDMARKSWEEPCOLLECTOR_HPP_

#include "Collector.hpp"
#include "../Allocators/Allocator.hpp"
#include "../Main/ObjectContainer.hpp"
#include <queue>
#include <stack>
#include "../defines.hpp"
#include "../Main/MemoryManager.hpp"
#include <stdio.h>
#include <ctime>

using namespace std;

namespace traceFileSimulator {

class MemoryManager;

// This collector immplements a split-heap copying collection policy
class RegionBasedMarkSweepCollector : public Collector {
public:
	RegionBasedMarkSweepCollector();
	virtual ~RegionBasedMarkSweepCollector();
	void collect(int reason);
	void collect(int reason,int thread);
	void checkWatermark();
	int promotionPhase();
	void initializeHeap();

private:
	void buildCollectionSet(int thread);
	void addChildren(Object *parent);
	void markObjects(int thread);
	void copy();
	void compact();
	void initializeMarkPhase();
	void preCollect();
	void freeAllLiveObjects();
	void reallocateAllLiveObjects();
	void breadthFirstCopying();
	void depthFirstCopying();
	void hotnessCopying();
	void getAllRoots();
	void emptyHelpers();
	void swap();
	void copyAndForwardObject(Object *o);
};

} 
#endif /* REGIONBASEDMARKSWEEPCOLLECTOR_HPP_ */
