/*
 * defines.hpp
 *
 *  Created on: 2013-09-04
 *      Author: GarCoSim
 */

#ifndef _DEFINES_HPP_
#define _DEFINES_HPP_

// set this either to 1000 (10^2) or 1024 (2^10) depending on how you want to convert byte sizes
#define MAGNITUDE_CONVERSION 1024

#define NUM_THREADS		50
#define ROOTSET_SIZE    50
#define VISUALIZE_GCS 	1

#define ALLOW_SHIFTING 0

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
//#define GENRATIO           0.3
//#define PROMOTIONAGE       3
#define SHIFTING           1
#define SHIFTINGFACTOR     2

//FOR HOTNESS
#define HOTNESS_DECAY        0     // not implemented yet :)
#define HOTNESS_DECAY_FACTOR 0.2

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
					simulatedAlloc
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

#define DEBUGMSG(msg) (fprintf(stderr, "%s:%d - %s\n", __FILE__, __LINE__, msg))

#define DEBUGMSG_IFEQUAL(left, right, msg) if (left == right) DEBUGMSG(msg)

// create some fancy strings for debug output
#define TRAVERSAL_STRING (traversal == (int)breadthFirst ? "breadthFirst" : (traversal == (int)depthFirst ? "depthFirst" : "hotness"))
#define COLLECTOR_STRING (collector == (int)traversalGC ? "traversal" : (collector == (int)markSweepGC ? "markSweep" : "copying"))
#define ALLOCATOR_STRING (allocator == (int)simulatedAlloc ? "simulated" : "real")

#define CREATE_GLOBAL_FILENAME(name) (globalFilename = (name).substr(0, (name).find(".trace")))

#define CHECK_CLASS(object, string) (!strcmp(object->getClassName(), string))

#define CONVERT(size) (size >= 1073741824 ? size / 1073741824 : (size >= 1048576 ? size / 1048576 : (size >= 1024 ? size / 1024 : size)))
#define MAGNITUDE(size) (size >= 1073741824 ? 'G' : (size >= 1048576 ? 'M' : (size >= 1024 ? 'K' : size)))

// for thread groups
#define OBJ_THREAD(object) (object->getThread())
#define T_TO_TG(object) (threadToThreadGroup[OBJ_THREAD(object)])
#define TGROUP(object) (threadGroup.at(threadToThreadGroup[OBJ_THREAD(object)]))
#define CHECK_THREAD(parent, child) (OBJ_THREAD(parent) != OBJ_THREAD(child))
#define IS_IN_THREAD_GROUP(object) (threadToThreadGroup[OBJ_THREAD(object)] != NO_THREAD_GROUP)
#define IS_NOT_IN_THREAD_GROUP(object) (threadToThreadGroup[OBJ_THREAD(object)] == NO_THREAD_GROUP)
#define CHECK_THREAD_GROUPS(parent, child) (threadToThreadGroup[OBJ_THREAD(child)] != threadToThreadGroup[OBJ_THREAD(parent)])
#define DIFFERENT_THREAD_GROUPS(parent, child) (IS_NOT_IN_THREAD_GROUP(parent) || IS_NOT_IN_THREAD_GROUP(child) || CHECK_THREAD_GROUPS(parent, child))

#endif
