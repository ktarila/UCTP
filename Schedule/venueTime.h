/*
 * venueTime.h
 *
 *  Created on: 25 Jun 2015
 *      Author: patrick
 */

#ifndef VENUETIME_H_
#define VENUETIME_H_

#include "../Venues/venue.h"
#include "../Courses/course.h"

using namespace std;

class RoomTime {
private:
	int period;
	Venue room;

public:
	RoomTime(int, Venue); //constructor
	RoomTime();
	~RoomTime(); //destructor
	int getPeriod() const;
	void setPeriod(int period);
	Venue getRoom() const;
	void setRoom(Venue room);
	bool friend operator== (const RoomTime &c1, const RoomTime &c2);
	bool friend operator!= (const RoomTime &c1, const RoomTime &c2);
	string toString();
	int getDay(int periods_in_day);
	bool canAssignCourse(const Course&);
};


#endif /* VENUETIME_H_ */
