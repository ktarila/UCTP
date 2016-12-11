/*
 * venue.h
 *
 *  Created on: 21 Jun 2015
 *      Author: patrick
 */


#ifndef VENUE_H_
#define VENUE_H_
#include <iostream>
#include <stdio.h>
#include <string>

using namespace std;


class Venue {
private:

	string name;  //room name
	int capacity;  //room capacity
	int site;      //location of room


public:
	Venue(string, int, int);
	Venue(string);
	Venue();
	virtual ~Venue();
	bool friend operator== (const Venue &c1, const Venue &c2);
	bool friend operator!= (const Venue &c1, const Venue &c2);
	string toString();
	string getName();
	void setName(string);
	int getCapacity() const;
	int getSite() const;

};


#endif /* VENUE_H_ */
