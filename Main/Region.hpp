/*
 * Region.hpp
 *
 *  Created on: September 1, 2015
 *      Author: Tristan
 */

#ifndef REGION_HPP_
#define REGION_HPP_

#include <vector>
#include <set>
#include <map>
#include <list>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <math.h>
#include "Object.hpp"

#define REGIONSIZE 524288//4194304

using std::vector;
namespace traceFileSimulator {

class Region {
public:
	virtual ~Region();	
	Region(void *address, int size,int owner);
	
    void setSize(int size);
    int  getSize();

    void  setAddress(void* address);
    void* getAddress();
    
    void setOwner(int owner); 
    int  getOwner();       

    void setCurrFree(int free); 
    int  getCurrFree(); 

    void incNumObj(); 
    int  getNumObj(); 

    void  setCurrFreeAddr(void* addr); 
    void* getCurrFreeAddr(); 

    int   getRegion(long address);

    void  setAge(int age);
    void  incAge();
    int   getAge();

    void  insertRemset(void *obj);
    void  eraseRemset(void *obj);

    std::set<void*>     getRemset();
    std::map<void*,int> getObjects(); //real Address

    void addObject(void* obj,int size);
    void delObject(void* obj);

private:
	int   mySize;
	void* myAddress;
	int   myOwner;       //assigned thread
    int   myAge;
    std::set<void*>     myRemset;   //remembered set 
    std::map<void*,int> myObjects;      //pointer to objects in the region

    int   numObj;        //how many objects in the region
	int   currFree;      //how much free space in a region
	void  *currFreeAddr; //address of the free area in the region, (logical address)
};

extern Region**      regions;
extern int           numRegions;
extern unsigned long heapAddr;
extern int           arrayletID;

extern std::list<int>   freeList;
extern std::vector<int> edenList;

extern long sumObj;
extern long sumFree;
extern int  trigReason;

} 
#endif /* REGION_HPP_ */
