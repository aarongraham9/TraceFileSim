/*
 * ReferenceCountingCollector.hpp
 *
 *  Created on: 2015-07-07
 *      Author: GarCoSim
 */

#ifndef REFERENCECOUNTINGCOLLECTOR_HPP_
#define REFERENCECOUNTINGCOLLECTOR_HPP_

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

class ReferenceCountingCollector : public Collector {
public:
	ReferenceCountingCollector();
	virtual ~ReferenceCountingCollector();
	void collect(int reason);
private:
	void preCollect();
};

}
#endif /* REFERENCECOUNTINGCOLLECTOR_HPP_ */
