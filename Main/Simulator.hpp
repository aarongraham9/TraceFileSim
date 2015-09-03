/*
 * Simulator.hpp
 *
 *  Created on: Sep 3, 2013
 *      Author: GarCoSim
 */

#ifndef SIMULATOR_HPP_
#define SIMULATOR_HPP_
 
#include <stdio.h>
#include <fstream>
#include <time.h>
#include "MemoryManager.hpp"
#include <string>
#include <stdlib.h>
 #include <pthread.h>

#define ONE_SECOND_PASSED (current.tv_sec - start.tv_sec >= 1.0f)
//#define ONE_SECOND_PASSED ((double(clock() - start) / CLOCKS_PER_SEC) >= 1.0f)  // not feasible anymore due to multithreaded execution!

using namespace std;

namespace traceFileSimulator {

typedef struct TraceFileLine {
	char type;
	int classID;
	int fieldIndex;
	int fieldOffset;
	int fieldSize;
	int fieldType;
	int objectID;
	int parentID;
	int parentSlot;
	int maxPointers;
	int size;
	int threadID;
} TraceFileLine;

class Simulator {
public:
	Simulator(char* traceFilePath, int heapSize, int highWatermark, int garbageCollector, int traversal, int allocator);
	virtual ~Simulator();
	int lastStepWorked();
	int doNextStep();
	void printStats();
	void lastStats();
	void cleanup();

private:
	void initializeTraceFileLine(TraceFileLine *line);
	TraceFileLine getNextLine();
	void getNextLine(TraceFileLine *line);
	void allocateToRootset(TraceFileLine line);
	void referenceOperation(TraceFileLine line);
	void deleteRoot(TraceFileLine line);
	void addToRoot(TraceFileLine line);
	void referenceOperationClassField(TraceFileLine line);
	void readOperation(TraceFileLine line);
	void storeOperation(TraceFileLine line);

	static ifstream myTraceFile;
	
	static int myLastStepWorked;
	MemoryManager* myMemManager;
	
	//debug
	int counter;
	struct timespec start;
	struct timespec current;
	int seconds;

	static void *tracefileReader(void *This);
	static TraceFileLine tracefileBuffer[TRACEFILE_BUFFER_SIZE];
	static unsigned int tracefileBufferPointer;
	static unsigned int tracefileBufferPointerInternal;
	static bool tracefileBufferStartup;
	static bool tracefileBufferFull;

	pthread_t tracefileReadThread;
	static pthread_mutex_t tracefileMutex;
	static pthread_cond_t tracefileCondition;
};

} 
#endif /* SIMULATOR_HPP_ */
