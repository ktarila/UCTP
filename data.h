/*
 * data.h
 *
 *  Created on: 03 June 2015
 *      Author: patrick
 */

#ifndef DATA_H_
#define DATA_H_

/*
 * data.h
 *
 *  Created on: 18 Mar 2015
 *      Author: patrick
 */

#include <iostream>
#include <fstream>
#include <string>
#include <vector>


#include "Courses/course.h"
#include "Venues/venue.h"
#include "Curricula/curricula.h"

using namespace std;

class Data {
private:
	string name;
	int num_courses;
	int num_rooms;
	int num_days;
	int num_periods_per_day;
	int num_curricula;
	int num_roomConstraints;
	int num_unavailabilityConstraint;
	int minDaily;
	int maxDaily;
	vector<Course> courses;
	vector<Venue> rooms;
	vector<Curricula> curriculum;
	void addRoomSizeConstraints();

public:
	Data(const char*); //constructor
	~Data(); //destructor
	vector<Course> getCourses() const;
	vector<Curricula> getCurriculum() const;
	int getMaxDaily() const;
	int getMinDaily() const;
	string getName() const;
	int getNumCourses() const;
	int getNumCurricula() const;
	int getNumDays() const;
	int getNumPeriodsPerDay() const;
	int getNumRoomConstraints() const;
	int getNumRooms() const;
	int getNumUnavailabilityConstraint() const;
	vector<Venue> getRooms() const;

};


#endif /* DATA_H_ */


