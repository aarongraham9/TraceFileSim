/*
 * Simulator.cpp
 *
 *  Created on: Sep 3, 2013
 *      Author: GarCoSim
 */

#include "Simulator.hpp"

using namespace std;

extern int gLineInTrace;
extern int gAllocations;
extern int forceAGCAfterEveryStep;
extern int multithreaded;

namespace traceFileSimulator {

TraceFileLine Simulator::tracefileBuffer[TRACEFILE_BUFFER_SIZE];
unsigned int Simulator::tracefileBufferPointer;
unsigned int Simulator::tracefileBufferPointerInternal;
bool Simulator::tracefileBufferStartup;
bool Simulator::tracefileBufferFull;
ifstream Simulator::myTraceFile;
int Simulator::myLastStepWorked;
pthread_mutex_t Simulator::tracefileMutex;
pthread_cond_t Simulator::tracefileCondition;

Simulator::Simulator(char* traceFilePath, int heapSize, int highWatermark, int garbageCollector, int traversal, int allocator) {
	myLastStepWorked = 1;
	myTraceFile.open(traceFilePath);
	if(!myTraceFile.good()){
		fprintf(stderr, "File open failed.\n");
		exit(1);
	}

	myMemManager = new MemoryManager(heapSize, highWatermark, garbageCollector, traversal, allocator);

	if (!myMemManager->loadClassTable((string)traceFilePath))
		fprintf(stdout, "No class table found\n");

	counter = 0;

	if (multithreaded) {
		tracefileBufferStartup = false;
		pthread_mutex_init(&tracefileMutex, NULL);
		pthread_cond_init(&tracefileCondition, NULL);
		pthread_create(&tracefileReadThread, NULL, tracefileReader, this);
		while (!tracefileBufferStartup); // we wait for the tracefile reader to warm up before we actually start processing
	}

	clock_gettime(CLOCK_MONOTONIC, &start);
	seconds = 0;
}

void Simulator::initializeTraceFileLine(TraceFileLine *line) {
	// a value of -1 indicates that the field is not present
	line->type = ' ';
	line->classID = 0; // class id's are 1-indexed, so special case (initialize to 0)
	line->fieldIndex = -1;
	line->fieldOffset = -1;
	line->fieldSize  = -1;
	line->fieldType = -1;
	line->objectID = -1;
	line->parentID = -1;
	line->parentSlot = -1;
	line->maxPointers = -1;
	line->size = -1;
	line->threadID = -1;
}

void Simulator::getNextLine(TraceFileLine *line){
	if(myTraceFile.eof()) {
		myLastStepWorked = false;
		return;
	}

	string currentLineString = "";
	getline(myTraceFile, currentLineString);
	char *currentLine = (char*) currentLineString.c_str();
	gLineInTrace++;

	if (!line)
		return; // caller didn't care about the parsed attributes

	initializeTraceFileLine(line);
	line->type = currentLine[0];

	string buffer = "";
	char attributeID;
	int val, i = 1;
	while (currentLine[i] != '\0') {
		while (currentLine[i] == ' ') // burn extra whitespace between attributes
			i++;

		attributeID = currentLine[i++];
		buffer.clear();
		while (currentLine[i]!=' ' && currentLine[i]!='\0')
			buffer.append(1, currentLine[i++]);

		val = atoi(buffer.c_str());

		switch (attributeID) {
			case ('C'):
				line->classID = val; break;
			case ('I'):
				line->fieldIndex = val; break;
			case ('F'):
				line->fieldOffset = val; break;
			case ('S'):
				line->size = val; break;
			case ('V'):
				line->fieldType = val; break;
			case ('O'):
				line->objectID = val; break;
			case ('P'):
				line->parentID = val; break;
			case ('#'):
				line->parentSlot = val; break;
			case ('N'):
				line->maxPointers = val; break;
			case ('T'):
				line->threadID = val; break;
			default:
				fprintf(stderr, "Invalid form in getNextLine, execution should never reach this line\n");
				break;
		}
	}
}

void *Simulator::tracefileReader(void *This) {
	tracefileBufferPointer = 0;
	tracefileBufferPointerInternal = 0;
	tracefileBufferFull = false;
	string currentLineString;
	string buffer;
	char attributeID;
	int val, i = 1;
	char *currentLine;

	while (!myTraceFile.eof()) {
		pthread_mutex_lock(&tracefileMutex);

		currentLineString = "";
		getline(myTraceFile, currentLineString);
		currentLine = (char*) currentLineString.c_str();
		gLineInTrace++;

		tracefileBuffer[tracefileBufferPointerInternal].type = currentLine[0];

		buffer = "";
		i = 1;
		while (currentLine[i] != '\0') {
			while (currentLine[i] == ' ') // burn extra whitespace between attributes
				i++;

			attributeID = currentLine[i++];
			buffer.clear();
			while (currentLine[i]!=' ' && currentLine[i]!='\0')
				buffer.append(1, currentLine[i++]);

			val = atoi(buffer.c_str());

			switch (attributeID) {
				case ('C'):
					tracefileBuffer[tracefileBufferPointerInternal].classID = val; break;
				case ('I'):
					tracefileBuffer[tracefileBufferPointerInternal].fieldIndex = val; break;
				case ('F'):
					tracefileBuffer[tracefileBufferPointerInternal].fieldOffset = val; break;
				case ('S'):
					tracefileBuffer[tracefileBufferPointerInternal].size = val; break;
				case ('V'):
					tracefileBuffer[tracefileBufferPointerInternal].fieldType = val; break;
				case ('O'):
					tracefileBuffer[tracefileBufferPointerInternal].objectID = val; break;
				case ('P'):
					tracefileBuffer[tracefileBufferPointerInternal].parentID = val; break;
				case ('#'):
					tracefileBuffer[tracefileBufferPointerInternal].parentSlot = val; break;
				case ('N'):
					tracefileBuffer[tracefileBufferPointerInternal].maxPointers = val; break;
				case ('T'):
					tracefileBuffer[tracefileBufferPointerInternal].threadID = val; break;
				default:
					fprintf(stderr, "Invalid form in getNextLine, execution should never reach this line\n");
					break;
			}
		}

		tracefileBufferPointerInternal++;
		if (tracefileBufferPointerInternal == 10)
			tracefileBufferStartup = true;
		if (tracefileBufferPointerInternal == TRACEFILE_BUFFER_SIZE)
			tracefileBufferPointerInternal = 0;
		if (tracefileBufferPointerInternal == tracefileBufferPointer)
			pthread_cond_wait(&tracefileCondition, &tracefileMutex);
		pthread_mutex_unlock(&tracefileMutex);
	}
	myLastStepWorked = false;
	return NULL;
}

TraceFileLine Simulator::getNextLine(){
	pthread_mutex_lock(&tracefileMutex);

	TraceFileLine line = tracefileBuffer[tracefileBufferPointer];
	tracefileBufferPointer++;

	// roll over at the edge of the buffer
	if (tracefileBufferPointer == TRACEFILE_BUFFER_SIZE)
		tracefileBufferPointer = 0;

	// if we hit a certain emptiness of our buffer we start filling it again
	if (abs(tracefileBufferPointerInternal - tracefileBufferPointer) <= TRACEFILE_BUFFER_SIZE * 0.7) // we set a 70% margin for our buffer
		pthread_cond_signal(&tracefileCondition);

	pthread_mutex_unlock(&tracefileMutex);
	return line;
}

void Simulator::lastStats() {
	myMemManager->lastStats();
}

int Simulator::doNextStep(){
	TraceFileLine line;

	if (multithreaded)
		line = getNextLine();
	else
		getNextLine(&line);

	clock_gettime(CLOCK_MONOTONIC, &current);
	if (ONE_SECOND_PASSED) {
		clock_gettime(CLOCK_MONOTONIC, &start);
		seconds++;
		printf("[%3ds] Line in tracefile: %7d\n", seconds, gLineInTrace);
	}
	if(myLastStepWorked){
		//if content exists, advice the MM(memory manager) to execute
		switch(line.type) {
			case 'w':
				referenceOperation(line);
				break;
			case 'a':
				allocateToRootset(line);
				//next line is a '+', which we skip since it adds the newly created object
				//to the rootset, which already happened in the simulator
				if (multithreaded)
					(void)getNextLine();
				else
					getNextLine(NULL);
				break;
			case '+':
				addToRoot(line);
				break;
			case '-':
				deleteRoot(line);
				break;
			case 'c': // for now we ignore the class option
				// currently doesn't do anything
				referenceOperationClassField(line);
				break;
			case 'r': // for now we ignore the class option
				// currently doesn't do anything
				readOperation(line);
				break;
			case 's': // for now we ignore the class option
				storeOperation(line);
				break;	

			default:
				//gLineInTrace++;
			break;
		}

	}
	// commented by mazder
	if (forceAGCAfterEveryStep) 
		myMemManager->forceGC();

	/*This line calls a garbage collec after each line. Usually useful
	 * in order to analyse the actual heap use of the file.*/
	//myMemManager->evalCollect();

	/*print stats to log file after each line
	 *.. usually used for result collection and comparison*/
	//myMemManager->printStats();


	return 0;
}

int Simulator::lastStepWorked(){
	if(myLastStepWorked == 1){
		return 1;
	}
	return 0;
}


void Simulator::allocateToRootset(TraceFileLine line){
	myMemManager->allocateObjectToRootset(line.threadID, line.objectID, line.size, line.maxPointers, line.classID);
}

void Simulator::deleteRoot(TraceFileLine line){
	myMemManager->requestRootDelete(line.threadID, line.objectID);
}

void Simulator::addToRoot(TraceFileLine line){
	myMemManager->requestRootAdd(line.threadID, line.objectID);
}

void Simulator::referenceOperation(TraceFileLine line){
	myMemManager->setPointer(line.threadID, line.parentID, line.parentSlot, line.objectID);

	if(line.fieldOffset != -1){
		/* when fieldOffset is given */
	}
	else{
		/* when fieldIndex is given */
	}

}

void Simulator::cleanup() {
	if(multithreaded) {
		pthread_join(tracefileReadThread, NULL);
		pthread_mutex_destroy(&tracefileMutex);
		pthread_cond_destroy(&tracefileCondition);
	}
}

// Added by Mazder

void Simulator::referenceOperationClassField(TraceFileLine line){
	/* Insert code here to store object reference into a class, when only fieldOffest of the reference slot is given*/

}
void Simulator::readOperation(TraceFileLine line){
	bool staticFlag = false; 	 // To decide reading is either from a class field ( static field) or an object field    
	bool offsetFlag = false; 	 // to decide either offest or index is given

	if (line.classID != -1)
		staticFlag = true;
	if (line.fieldOffset != -1)
		offsetFlag = true;

	if(staticFlag){
		/* read access to class field */
		if(offsetFlag){
			/* when fieldoffset is given */ 
		}
		else{
			/* when fieldIndex is given */ 
		}

	}
	else{
		/* read access to object field */
		if(offsetFlag){
			/* when fieldoffset is given */ 
		}
		else{
			/* when fieldoffset is given */ 
		}
	}

}

/* This is to store static primitive field in class and primitive field in an object */

void Simulator::storeOperation(TraceFileLine line){
	bool staticFlag = false; 	 // To decide reading is either from a class field ( static field) or an object field    
	bool offsetFlag = false; 	 // to decide either offest or index is given

	if (line.classID != -1)
		staticFlag = true;
	if (line.fieldOffset != -1)
		offsetFlag = true;

	if(staticFlag){
		/* read access to class field */
		if(offsetFlag){
			/* when fieldoffset is given */ 
		}
		else{
			/* when fieldIndex is given */ 
		}

	}
	else{
		/* read access to object field */
		if(offsetFlag){
			/* when fieldoffset is given */ 
		}
		else{
			/* when fieldoffset is given */ 
		}
	}

}

void Simulator::printStats(){
	myMemManager->printStats();
}

Simulator::~Simulator() {

}

}
