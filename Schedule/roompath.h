/*
 * roompath.h
 *
 *  Created on: 28 Aug 2015
 *      Author: patrick
 */

#ifndef ROOMPATH_H_
#define ROOMPATH_H_

#include "courseroomtime.h"

class RoomPath {

private:
	Venue room;
	CourseRoomTime courseRTime;
	int roomIndex;
	bool update;




public:
	RoomPath(Venue, CourseRoomTime, int);
	RoomPath();
	~RoomPath();
	vector<CourseRoomTime> applyRoomPath(const vector<CourseRoomTime>& timetable, CourseRoomTime* tCRT, int maxP);
	CourseRoomTime getCourseRTime() const;
	Venue getRoom() const;
	int getRoomIndex() const;
	bool isUpdate() const;
	void setUpdate(bool update);

};



#endif /* ROOMPATH_H_ */
