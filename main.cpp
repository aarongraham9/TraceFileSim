/*
 * main.cpp
 *
 *  Created on: Sep 3, 2013
 *      Author: GarCoSim
 */

#include "Main/Simulator.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <ctime>
#include <string.h>

#include "defines.hpp"

using namespace traceFileSimulator;
using namespace std;
//logging vars
int gLineInTrace;
int gAllocations;
FILE* gLogFile;
FILE* gDetLog;
int forceAGCAfterEveryStep = 0;
string globalFilename;
float genRatio;
short promotionAge;

int getHeapSizeInBytes(char *arg) {
	char suffix;

	// if we have no suffix we can skip this check
	if (isdigit(arg[strlen(arg) - 1]))
		return atoi(arg);

	suffix = arg[strlen(arg) - 1];

	// get rid of a trailing b/B
	if (suffix == 'B' || suffix == 'b')
		suffix = arg[strlen(arg) - 2];

	switch(suffix) {
		case 'K':
		case 'k':
			return atoi(arg) * MAGNITUDE_CONVERSION;
		case 'M':
		case 'm':
			return atoi(arg) * MAGNITUDE_CONVERSION * MAGNITUDE_CONVERSION;
		case 'G':
		case 'g':
			return atoi(arg) * MAGNITUDE_CONVERSION * MAGNITUDE_CONVERSION * MAGNITUDE_CONVERSION;
		default:
			return atoi(arg);
	}
}

int setArgs(int argc, char *argv[], const char *option, const char *shortOption) {
	int i;

	for (i = 1; i < argc; i++) {
		if ((!strcmp("--force", option) || !strcmp("-f", shortOption)) && (!strcmp(argv[i], option) || !strcmp(argv[i], shortOption))) {
			return 1;
		}
		if (!strcmp(argv[i], option) || !strcmp(argv[i], shortOption)) {
			if (!strcmp(option, "--collector") || !strcmp(shortOption, "-c")) {
				if (!strcmp(argv[i + 1], "copying"))
					return (int)copyingGC;
				if (!strcmp(argv[i + 1], "markSweep"))
					return (int)markSweepGC;
				if (!strcmp(argv[i + 1], "traversal"))
					return (int)traversalGC;
				return -1;
			} else if (!strcmp(option, "--traversal") || !strcmp(shortOption, "-t")) {
				if (!strcmp(argv[i + 1], "breadthFirst"))
					return (int)breadthFirst;
				if (!strcmp(argv[i + 1], "depthFirst"))
					return (int)depthFirst;
				if (!strcmp(argv[i + 1], "hotness"))
					return (int)hotness;
				return -1;
			} else if (!strcmp(option, "--allocator") || !strcmp(shortOption, "-a")) {
				if (!strcmp(argv[i + 1], "real"))
					return (int)realAlloc;
				if (!strcmp(argv[i + 1], "simulated"))
					return (int)simulatedAlloc;
				return -1;
			} else if (!strcmp(option, "--heapsize") || !strcmp(shortOption, "-h")) {
				return getHeapSizeInBytes(argv[i + 1]);
			} else {
				return atoi(argv[i + 1]); // be careful! we expect the next one to be a number, otherwise we crash instantly
			}
		}
	}

	return -1;
}

/* ----------------------------------------------------- MAIN ------------------------------------------------------------ */

int main(int argc, char *argv[]) {
	if(argc < 2) {
		fprintf(stderr, "Usage: TraceFileSimulator traceFile [OPTIONS]\n" \
						"Options:\n" \
						"  --watermark x, -w x       uses x percent as the high watermark (default: 90)\n" \
						"  --heapsize x,  -h x       uses x bytes for the heap size (default: 200k)\n" \
						"  --collector x, -c x       uses x as the garbage collector (valid: copying, markSweep, traversal, default: markSweep)\n" \
						"  --traversal x, -t x       uses x as the traversal algorithm (valid: breadthFirst depthFirst hotness, default: breadthFirst)\n" \
						"  --allocator x, -a x       uses x as the allocator (valid: real, simulated, default: real)\n" \
						"  --genratio x,  -g x       sets the size of the nursery in percent (valid: 0-100, default: 30)\n" \
						"  --promotion x, -p x       sets the promotion age in GCs (valid: 1-99999, default: 3)\n" \
						);
		exit(1);
	}

	fprintf(stderr, "TraceFileSimulator v%s\n\n", VERSION);

	if(WRITE_DETAILED_LOG) {
		gDetLog = fopen("detailed.log","w+");
	}

	char *filename    = argv[1];
	int heapSize      = setArgs(argc, argv, "--heapsize",  "-h");
	int highWatermark = setArgs(argc, argv, "--watermark", "-w");
	int traversal     = setArgs(argc, argv, "--traversal", "-t");
	int collector     = setArgs(argc, argv, "--collector", "-c");
	int allocator     = setArgs(argc, argv, "--allocator", "-a");
	genRatio 		  = setArgs(argc, argv, "--genratio", "-g");
	promotionAge	  = setArgs(argc, argv, "--promotion", "-p");
	forceAGCAfterEveryStep = setArgs(argc, argv, "--force", "-f");

	if (highWatermark == -1)
		highWatermark = 90;
	if (traversal == -1)
		traversal = (int)breadthFirst;
	if (collector == -1)
		collector = (int)markSweepGC;
	if (allocator == -1)
		allocator = (int)simulatedAlloc;
	if (heapSize == -1) {
		if (collector != (int)traversalGC)
			heapSize = 200000;
		else
			heapSize = 512000;
	}
	if (genRatio == -1)
		genRatio = 0.3f;
	else
		genRatio /= 100.0f;
	if (promotionAge == -1)
		promotionAge = 3;
	if (forceAGCAfterEveryStep == -1)
		forceAGCAfterEveryStep = 0;

	CREATE_GLOBAL_FILENAME((string)filename);

	string logFileName;
	if (forceAGCAfterEveryStep)
		logFileName = globalFilename + "Forced.log";
	else
		logFileName = globalFilename + ".log";
	{ // remove after testing
		char outChar[50];
		sprintf(outChar, "_C%d_H%dKB_G%d_P%d", collector, CONVERT(heapSize), setArgs(argc, argv, "--genratio", "-g"), promotionAge);
		logFileName = globalFilename + (string)outChar + ".log";
	}

	//set up global logfile
	gLogFile = fopen(logFileName.c_str(), "w+");
	fprintf(gLogFile, "TraceFileSimulator v%s\nCollector: %s\nTraversal: %s\nAllocator: %s\nHeapsize: %d %cBytes (%d Bytes)\nWatermark: %d\nGenRatio: %.2f\nPromotion Age: %d\n\n", 
			VERSION, COLLECTOR_STRING, TRAVERSAL_STRING, ALLOCATOR_STRING, CONVERT(heapSize), MAGNITUDE(heapSize), heapSize, highWatermark, genRatio, promotionAge);
	fprintf(gLogFile, "%8s | %14s | %10s | %14s "
			"| %13s | %10s | %10s | %10s | %7s\n",
			"Line", "GC Reason", "Total GCs", "Objects Freed", "Live Objects",
			"Heap Used", "Free Heap", "Generation", "GC Time");

	fprintf(stderr, "Using tracefile '%s' with a heap size of %d %cBytes and a high watermark of %d\n", filename, CONVERT(heapSize), MAGNITUDE(heapSize), highWatermark);
	fprintf(stderr, "The collector is '%s' and the selected traversal is '%s'\n", COLLECTOR_STRING, TRAVERSAL_STRING);
	fprintf(stderr, "The allocator is '%s'\n", ALLOCATOR_STRING);
	if (forceAGCAfterEveryStep)
		fprintf(stderr, "Forcing a GC after every step\n");

	//start measuring time
	clock_t start = clock();

	Simulator* simulator = new Simulator(filename, heapSize, highWatermark, collector, traversal, allocator);

	while(simulator->lastStepWorked() == 1) {
		simulator->doNextStep();
		if(WRITE_DETAILED_LOG) {
			fflush(gDetLog);
		}
	}
	simulator->printStats();
	simulator->lastStats();

	clock_t end = clock();
	double elapsed_secs = double(end - start)/CLOCKS_PER_SEC;
	//double elapsed_msecs = (double)(double)(end - start)/(CLOCKS_PER_SEC/1000);
	printf("Simulation ended successfully, execution took %0.3f seconds\n", elapsed_secs);
	fprintf(gLogFile,"Execution finished after %0.3f seconds\n", elapsed_secs);

	fclose(gLogFile);

	return EXIT_SUCCESS;
}


