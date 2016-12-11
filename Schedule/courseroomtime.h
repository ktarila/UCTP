/*
 * courseroomtime.h
 *
 *  Created on: 27 Jun 2015
 *      Author: patrick
 */

#ifndef COURSEROOMTIME_H_
#define COURSEROOMTIME_H_

#include "../Courses/course.h"
#include "venueTime.h"

using namespace std;

class CourseRoomTime {
private:
	Course course;
	RoomTime venueTime;
	bool update;

public:
	CourseRoomTime(Course, RoomTime); //constructor
	CourseRoomTime();
	~CourseRoomTime(); //destructor
	bool isLastPeriod(int maxPeriod); //Check of CRT object is the last period of the day
	bool isFirstPeriod(int maxPeriod); //Check of CRT object is the first period of the day
	bool isDay(int maxPeriod, int day); //Check of CRT object is in a particular day
	bool friend operator== (const CourseRoomTime &c1, const CourseRoomTime &c2);
	string toString();
	Course getCourse() const;
	void setCourse(Course course);
	RoomTime getVenueTime() const;
	void setVenueTime(RoomTime venueTime);
	bool isUpdate() const;
	void setUpdate(bool update);
	bool empty;
};


#endif /* COURSEROOMTIME_H_ */
