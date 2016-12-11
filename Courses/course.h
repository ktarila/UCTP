/*
 * courses.h
 *
 *  Created on: 20 Feb 2015
 *      Author: patrick
 */

// courses.h"

#ifndef COURSE_H_
#define COURSE_H_
#include <iostream>
#include <stdio.h>
#include <string>
#include <vector>

using namespace std;

class Course {
private:
	 string title;       //title of course
	 string code;        //course code, key identifier of course
	 int numStudents; //number of students taking the course
	 string lecturer;    //lecturer taking the course
	 int weeklyLectures;   //number of lectures in one week
	 int minWorkingDays;   //minimum number of working days
	 int doubleLectures;   //number of double periods
	 int assignedPeriod;  //number of periods already assigned in schedule 
	 vector<string> roomConstraint;  //list of rooms (room names)  course can be scheduled, if empty course can be scheduled in any room
	 vector<int> unavailableConstraint;  //list of periods course cannot be scheduled in
	 vector<string> curriculaCodes;  //department/curricula of students taking the course
	 bool empty;


public:
	Course(string, string, int, int, int, int);
	Course(string);
	Course();
	virtual ~Course();
	bool friend operator== (const Course &c1, const Course &c2);
	bool friend operator!= (const Course &c1, const Course &c2);
	bool friend operator< (const Course &c1, const Course &c2);
	bool friend operator> (const Course &c1, const Course &c2);
	vector<string> getCurriculaCodes();
	void addCurriculaCodes(string);
	vector<int> getUnavailableConstraint();
	void addUnavailableConstraint(int);
	vector<string> getRoomConstraint();
	void addRoomConstraint(string);
	int getAssignedPeriod() const;
	void setAssignedPeriod(int assignedPeriod);
	string getCode() const;
	int getDoubleLectures() const;
	string getLecturer() const;
	int getMinWorkingDays() const;
	int getNumStudents() const;
	string getTitle() const;
	int getWeeklyLectures() const;
	bool isEmpty() const;
	int getAvailable();
};

#endif /* COURSE_H_ */

