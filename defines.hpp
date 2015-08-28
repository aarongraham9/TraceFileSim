/*
 * defines.hpp
 *
 *  Created on: 2013-09-04
 *      Author: GarCoSim
 */

#ifndef _DEFINES_HPP_
#define _DEFINES_HPP_

// set this either to 1000 (10^3) or 1024 (2^10) depending on how you want to convert byte sizes
#define MAGNITUDE_CONVERSION 1024

// this parameter needs some fine tuning
#define TRACEFILE_BUFFER_SIZE 10000

#define NUM_THREADS		50
#define ROOTSET_SIZE    50
#define VISUALIZE_GCS 	1
#define OBJECT_HEADER_SIZE 16

#define CLASS_OBJECT    -1
#define NO_THREAD_GROUP -1
#define NULL_OBJECT     0

//DEBUGGING
#define DEBUG_MODE            0
#define WRITE_DETAILED_LOG    0
#define WRITE_HEAPMAP         0
#define WRITE_ALLOCATION_INFO 0

//GENERATIONAL GC
#define GENERATIONS        1
#define GEN_DEBUG          0
#define GENRATIO           0.3
#define PROMOTIONAGE       3
#define PROMOTIONAGEFACTOR 0
#define SHIFTING           1
#define SHIFTINGFACTOR     2

enum traversalEnum {
					breadthFirst = 0,
					depthFirst,
					hotness
				};

enum collectorEnum {
						copyingGC = 0,
						markSweepGC,
						traversalGC
				};

enum allocatorEnum {
						realAlloc = 0,
				};

enum gcReason {
					reasonStatistics = 0,
					reasonFailedAlloc,
					reasonHighWatermark,
					reasonDebug,
					reasonShift,
					reasonEval,
					reasonForced
				};

// create some fancy strings for debug output
#define TRAVERSAL_STRING (traversal == (int)breadthFirst ? "breadthFirst" : (traversal == (int)depthFirst ? "depthFirst" : "hotness"))
#define COLLECTOR_STRING (collector == (int)traversalGC ? "traversal" : (collector == (int)markSweepGC ? "markSweep" : "copying"))
#define ALLOCATOR_STRING "real"

#define CREATE_GLOBAL_FILENAME(name) (globalFilename = (name).substr(0, (name).find(".trace")))

//added by Tristan
#define MAX64BIT 0xFFFFFFFFFFFFFFFF //18446744073709551615 
#define MAX32BIT 0xFFFFFFFF         //4294967295           
#define MAX16BIT 0xFFFF             //65535   				
#define MAX8BIT  0xFF               //255   

#define CONVERT(size) (size >= 1073741824 ? size / 1073741824 : (size >= 1048576 ? size / 1048576 : (size >= 1024 ? size / 1024 : size)))
#define MAGNITUDE(size) (size >= 1073741824 ? 'G' : (size >= 1048576 ? 'M' : (size >= 1024 ? 'K' : size)))

#define CHECK_CLASS(object, string) (!strcmp(object->getClassName(), string))

#endif
