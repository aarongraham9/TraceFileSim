/*
 * RegionBasedMarkSweepCollector.cpp
 *
 *  Created on: 2015-02-16
 *      Author: GarCoSim
 */

#include "RegionBasedMarkSweepCollector.hpp"

extern int gLineInTrace;
extern FILE* gLogFile;
extern FILE* gDetLog;

extern FILE* gcFile;

extern clock_t start, stop;

namespace traceFileSimulator {

std::set<int>    collectionSet;

// This collector immplements a split-heap copying collection policy
RegionBasedMarkSweepCollector::RegionBasedMarkSweepCollector() {
	fprintf(stderr, "This collector is WIP, please use a different collector!\n");
}

/**
 * Argument indicates the reason for collection: 0 - unknown, 1 - failed alloc, 2 - high watermark
 */
void RegionBasedMarkSweepCollector::collect(int reason) {
	statCollectionReason = reason;
	stop = clock();
	double elapsed_secs = double(stop - start)/CLOCKS_PER_SEC;
	fprintf(stderr, "GC #%d at %0.3fs", statGcNumber + 1, elapsed_secs);

	preCollect(); 

	copy();

	swap();

	updatePointers();

	postCollect();

	stop = clock();
	elapsed_secs = double(stop - start)/CLOCKS_PER_SEC;
	fprintf(stderr, " took %0.3fs\n", elapsed_secs);
}


void printRegionStats() {
    set<int>::iterator it;
    map<void*,int> objects;
    map<void*,int>::iterator j;
    int r,marked,unmarked;
    Object* obj;
    RawObject *rawObject;

    marked = unmarked = 0;
    for (it=collectionSet.begin(); it!=collectionSet.end(); it++) {
        r = (int)*it;
        objects = regions[r]->getObjects();
     
        for (j=objects.begin(); j!=objects.end(); j++) {
        	rawObject = (RawObject*)j->first;
        	obj = (Object*)rawObject->associatedObject;
            if (obj->getVisited()==true) {
               marked++;
            }
            else
               unmarked++;
        }
    }
    printf("\nMarked = %i, Unmarked = %i, Total = %i, Percentage Marked = %.2lf\n",marked,unmarked,marked+unmarked,(double)marked/(marked+unmarked));
}

void RegionBasedMarkSweepCollector::collect(int reason,int thread) {
	statCollectionReason = reason;
	stop = clock();
	double elapsed_secs = double(stop - start)/CLOCKS_PER_SEC;
	fprintf(stderr, "GC #%d at %0.3fs", statGcNumber + 1, elapsed_secs);

	preCollect(); 
	buildCollectionSet(thread);
	markObjects(thread);


	//unmarkObjects();
	printRegionStats();
	postCollect();

	stop = clock();
	elapsed_secs = double(stop - start)/CLOCKS_PER_SEC;
	fprintf(stderr, " took %0.3fs\n", elapsed_secs);
}

void RegionBasedMarkSweepCollector::buildCollectionSet(int thread) {
	int i;
    
    collectionSet.clear();
    for (i=0; i<numRegions; i++) {
        if (regions[i]->getOwner() == thread) 
            collectionSet.insert(i);
    }
}

void RegionBasedMarkSweepCollector::addChildren(Object *parent) {
    Object *child;
    int numChild,childRegion;

    if (parent->getVisited()==false) {
        parent->setVisited(true);
        numChild = parent->getPointersMax();
        for (int i=0; i<numChild; i++) {
            child = parent->getReferenceTo(i);
            if (child && !child->getVisited()) {
                childRegion = child->getRegion(heapAddr,REGIONSIZE);
                if (collectionSet.find(childRegion)!=collectionSet.end())
                   addChildren(child);    
            }
        }
    }
}

void RegionBasedMarkSweepCollector::markObjects(int thread) {
    vector<Object*> roots;
    set<void*>      remset;
    Object*         parent;
    Object*         child;
    int             numChild,currRegion,childRegion;
    set<void*>::iterator j;
    set<int>::iterator   it;


    roots = myObjectContainer->getRoots(thread);
    for (unsigned i=0; i<roots.size(); i++) {
    	parent = roots.at(i);
    	if (parent) {
    	   currRegion = parent->getRegion(heapAddr,REGIONSIZE);
    	   if (collectionSet.find(currRegion)!=collectionSet.end()) {
              addChildren(parent); 
           }
    	}
    }

    for (it=collectionSet.begin(); it!=collectionSet.end(); it++) { //mark escaping objects
    	currRegion = (int)*it;
        remset = regions[currRegion]->getRemset();   
        for (j=remset.begin(); j!=remset.end(); j++) {
        	parent = (Object*)*j;
            numChild = parent->getPointersMax();
            for (int k=0; k<numChild; k++) {
               child = parent->getReferenceTo(k);
               if (child) {
                   childRegion = child->getRegion(heapAddr,REGIONSIZE);
                   if (currRegion == childRegion) {
                   	  child->setVisited(true);               	     
               	   }
               }
            }
        }
    }
    
}

void RegionBasedMarkSweepCollector::swap() {
	/*
	 * we need to do this step because we are simulating everything by maintaining an object list
	 * if we use a real allocator we do not delete the object, we just remove it from the object list
	 * it will then by overwritten afterwards. the real allocator just removes it from the object list
	 */
	int i;
	Object *currentObj;

	vector<Object*> objects = myObjectContainer->getLiveObjects();
	for (i = 0; i < (int)objects.size(); i++) {
		currentObj = objects[i];
		if (currentObj) {
			if (!myAllocator->isInNewSpace(currentObj)) {
				if (!currentObj->isForwarded()) {
					statFreedObjects++;
					statFreedDuringThisGC++;
				}
				myMemManager->requestDelete(currentObj, myGeneration == GENERATIONS - 1 ? 1 : 0);
				//myObjectContainer->deleteObject(currentObj, !myAllocator->isRealAllocator());
			}
		}
	}

	// free entire old space
	myAllocator->freeOldSpace();

	myAllocator->swapHeaps();
}

void RegionBasedMarkSweepCollector::checkWatermark() {
	int size = myAllocator->getRegionSize();
	int free = myAllocator->getFreeSize();
	int ratio = 100 - (100 * free / size);
	if (ratio > myWatermark) {
		collect((int)reasonHighWatermark);
	}
}

void RegionBasedMarkSweepCollector::initializeHeap() {
	myAllocator->setHalfHeapSize(true); //false for region-based; Tristan
}

void RegionBasedMarkSweepCollector::emptyHelpers() {
	while (!myQueue.empty())
		myQueue.pop();
	while (!myStack.empty())
		myStack.pop();
}

void RegionBasedMarkSweepCollector::copy() {
	//set all objects to dead and not visible firs
	initializeMarkPhase();

	emptyHelpers();

	getAllRoots();

	switch(order) {
		case breadthFirst:
			breadthFirstCopying();
			break;
		case depthFirst:
			depthFirstCopying();
			break;
		case hotness:
			hotnessCopying();
			break;
		default:
			break;
	}
}

void RegionBasedMarkSweepCollector::getAllRoots() {
	Object* currentObj;
	int i, j;

	// enqueue all roots
	if (myGeneration == GENERATIONS - 1) {
		//we are performing a glolab GC and can use it to fix possible rem set problems
		//we clear all rem sets and fill them again while performing the marking
		myMemManager->clearRemSets(); // why do does the remset have to be cleared every call to getAllRoots? by Tristan

		vector<Object*> roots;
		for (i = 0; i < NUM_THREADS; i++) {
			roots = myObjectContainer->getRoots(i);
			for (j = 0; j < (int)roots.size(); j++) {
				currentObj = roots[j];
				if (currentObj && !currentObj->getVisited()) {
					currentObj->setVisited(true);
					//currentObj->setAge(currentObj->getAge() + 1);
					//add to rem set if the root is in a younger generation.
					if (currentObj->getGeneration() < myGeneration) {
						myMemManager->requestRemSetAdd(currentObj);
					}
					myQueue.push(currentObj);
					myStack.push(currentObj);
				}
			}
		}
	} else {
		for (j = 0; j < myObjectContainer->getGenRootSize(); j++) {
			currentObj = myObjectContainer->getGenRoot(j);
			if (currentObj && !currentObj->getVisited()) {
				currentObj->setVisited(true);
				//currentObj->setAge(currentObj->getAge() + 1);
				myQueue.push(currentObj);
				myStack.push(currentObj);
			}
		}
	}
}

void RegionBasedMarkSweepCollector::copyAndForwardObject(Object *o) {
	void *addressBefore, *addressAfter;
	addressBefore = (void *) o->getAddress();
	myAllocator->moveObject(o);
	addressAfter = (void *) o->getAddress();
	addForwardingEntry(addressBefore, addressAfter);
}

void RegionBasedMarkSweepCollector::breadthFirstCopying() {
	int i;
	Object* currentObj;
	Object* child;

	//breadth first through the tree using a queue
	while (!myQueue.empty()) {
		currentObj = myQueue.front();
		myQueue.pop();
		int kids = currentObj->getPointersMax();
		copyAndForwardObject(currentObj);
		currentObj->setAge(currentObj->getAge() + 1);
		for (i = 0; i < kids; i++) {
			child = currentObj->getReferenceTo(i);
			//no matter if the child was processed before or not, add it to the rem set.
			if(child && child->getGeneration() < currentObj->getGeneration()){
				myMemManager->requestRemSetAdd(child);
			}
			if (child && !child->getVisited() && child->getGeneration() <= myGeneration) {
				child->setVisited(true);

				myQueue.push(child);
			}
		}
	}
}

void RegionBasedMarkSweepCollector::depthFirstCopying() {
	int i;
	Object* currentObj;
	Object* child;

	//depth first through the tree using a stack
	while (!myStack.empty()) {
		currentObj = myStack.top();
		myStack.pop();
		int kids = currentObj->getPointersMax();
		copyAndForwardObject(currentObj);
		currentObj->setAge(currentObj->getAge() + 1);
		for (i = 0; i < kids; i++) {
			child = currentObj->getReferenceTo(i);
			//no matter if the child was processed before or not, add it to the rem set.
			if(child && child->getGeneration() < currentObj->getGeneration()){
				myMemManager->requestRemSetAdd(child);
			}
			if (child && !child->getVisited() && child->getGeneration() <= myGeneration) {
				child->setVisited(true);

				myStack.push(child);
			}
		}
	}
}

void RegionBasedMarkSweepCollector::hotnessCopying() {
	//TODO
}

void RegionBasedMarkSweepCollector::initializeMarkPhase() {
	Object* currentObj;
	int i;
	vector<Object*> objects = myObjectContainer->getLiveObjects();
	for (i = 0; i < (int)objects.size(); i++) {
		currentObj = objects[i];
		if (currentObj) {
			if (WRITE_DETAILED_LOG == 1) {
				//fprintf(gDetLog, "(%d) MARK: %ld\n", gLineInTrace,
						//(long) currentObj);
			}
			currentObj->setVisited(false);
		}
	}
}

void RegionBasedMarkSweepCollector::preCollect() {
	start = clock();
	statFreedDuringThisGC = 0;
	statGcNumber++;
//	if(myGeneration == GENERATIONS -1 ){
//		promotionPhase();
//	}
}

void RegionBasedMarkSweepCollector::compact() {
	/*only alive objects are left in the container. If I traverse
	 through the live list, I get all elements*/
	//free everything.
	myMemManager->statBeforeCompact(myGeneration);
	freeAllLiveObjects();
	//allocate everything back.
	myMemManager->requestResetAllocationPointer(myGeneration);
	reallocateAllLiveObjects();
	myMemManager->statAfterCompact(myGeneration);
}

void RegionBasedMarkSweepCollector::freeAllLiveObjects() {
	int i;
	
	vector<Object*> objects = myObjectContainer->getLiveObjects();
	for (i = 0; i < (int)objects.size(); i++) {
		Object* currentObj = objects[i];
		if (currentObj) {
			myMemManager->requestFree(currentObj);
		}
	}
	//myAllocator->freeAllSectors();
}

int RegionBasedMarkSweepCollector::promotionPhase() {
	if (gcsSinceLastPromotionPhase == 0) {
		//nothing to do here
		return 0;
	}
	int g, oldi;
	//in case the next generation is too full, a flag to wait
	int noSpaceUpstairs = 0;
	oldi = -1;
	vector<Object*> objects = myObjectContainer->getLiveObjects();
	for (g = 0; g < (int)objects.size(); g++) {
		Object* currentObj = objects[g];
		if (g < oldi) {
			printf("oO\n");
		}
		//if object exists and is not in the oldest generation already
		if (currentObj && currentObj->getGeneration() < (GENERATIONS - 1)) {
			int age = currentObj->getAge();
			if (age
					>= PROMOTIONAGE + PROMOTIONAGE * myGeneration
							/*+ currentObj->getGeneration() * PROMOTIONAGEFACTOR*/) {
				//fprintf(stderr, "(%d) promoting object of age %d\n",gLineInTrace, age);
				if (currentObj->getGeneration() < GENERATIONS - 1) {
					noSpaceUpstairs = myMemManager->requestPromotion(
							currentObj);
					// end the promotion phase because of an "out of memory"
					// exception in the higher generation. It is not the best
					// solution
					if (noSpaceUpstairs == 1) {
						return -1;
					}
				}
			}
		}
		oldi = g;
	}
	return 0;
}

void RegionBasedMarkSweepCollector::reallocateAllLiveObjects() {
	int i;
	vector<Object*> objects = myObjectContainer->getLiveObjects();
	for (i = 0; i < (int)objects.size(); i++) {
		Object* currentObj = objects[i];
		if (currentObj) {
			myMemManager->requestReallocate(currentObj);
		}
	}

}

RegionBasedMarkSweepCollector::~RegionBasedMarkSweepCollector() {
}

} 
