/*
 * path.h
 *
 *  Created on: 16 Jul 2015
 *      Author: patrick
 */

#ifndef PATH_H_
#define PATH_H_

#include "courseroomtime.h"

class Path {

private:
	int period;
	CourseRoomTime colCRT;
	CourseRoomTime rowCRT;
	bool update;



public:
	Path(int, CourseRoomTime, CourseRoomTime);
	Path();
	~Path();
	vector<CourseRoomTime> applyPath(const vector<CourseRoomTime>& timetable, Path* tCRT);
	int getPeriod() const;
	bool isUpdate() const;
	void setUpdate(bool update);
	CourseRoomTime getColCrt() const;
	CourseRoomTime getRowCrt() const;
	void setColCrt(CourseRoomTime colCrt);
	void setRowCrt(CourseRoomTime rowCrt);

};


#endif /* PATH_H_ */
