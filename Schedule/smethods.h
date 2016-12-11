/*
 * smethods.h
 *
 *  Created on: 17 Jul 2015
 *      Author: patrick
 */

#ifndef SMETHODS_H_
#define SMETHODS_H_


#include <time.h>
#include <ctime>
#include <sys/time.h>
#include <chrono>
#include <random>

#include "courseroomtime.h"
#include "feasibletable.h"
#include "improvetable.h"


class SM {

private:



public:
	int static sizeTable;
	int static numPeriods;
	SM();
	int static NumberHCV(const vector<CourseRoomTime>& timetable,const  int& maximumPeriod);
	vector<Course> static getPeriodCourse(const int& period, const vector<CourseRoomTime>& timetable);
	string static getVenueCourse(const int& period, const Course& c, const vector<CourseRoomTime>& timetable);
	double static get_wall_time();
	double static get_cpu_time();
	void static periodViolation(const vector<CourseRoomTime>& timetable,const int& maximumPeriod, int period, int* violations);
	int static ThreadNumberHCV(const vector<CourseRoomTime>& timetable,const  int& maximumPeriod);
	int static getConsecutiveViolations(const vector<CourseRoomTime>& timetable, const int& period, const int& periods_in_day);
	int myrandom (int i);
	vector<CourseRoomTime> static getCRTsinPeriod(const int& period, const vector<CourseRoomTime>& timetable);
	vector<string> static curriculumInTimetable(const vector<CourseRoomTime>& timetable);
	static FeasibleTable iT;
	static ImproveTable iiT;
};


#endif /* SMETHODS_H_ */
