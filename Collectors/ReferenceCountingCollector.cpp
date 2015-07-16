/*
 * ReferenceCountingCollector.hpp
 *
 *  Created on: 2015-07-07
 *      Author: GarCoSim
 */

#include "ReferenceCountingCollector.hpp"

extern int gLineInTrace;
extern FILE* gLogFile;
extern FILE* gDetLog;
extern FILE* gcFile;
extern clock_t start, stop;

namespace traceFileSimulator {

ReferenceCountingCollector::ReferenceCountingCollector() {
	// for now, do nothing
}

void ReferenceCountingCollector::collect(int reason) {
	statCollectionReason = reason;
	preCollect();

	Object *current, *old;
	int slot;
	while(!doomed.empty()) {
		current = doomed.front();
		doomed.pop();
		for(slot=0; slot<current->getPointersMax(); slot++) {
			old = current->getReferenceTo(slot);
			current->setPointer(slot, NULL);
			if(old && old->getReferenceCount() == 0)
				doomed.push(old);
		}
		myMemManager->requestDelete(current, 0);
		statFreedObjects++;
		statFreedDuringThisGC++;
	}

	postCollect();
}

void ReferenceCountingCollector::preCollect() {
	start = clock();
	statFreedDuringThisGC = 0;
	statGcNumber++;
}

ReferenceCountingCollector::~ReferenceCountingCollector() {
}

}
