/*
 * courseroomtime.cpp
 *
 *  Created on: 27 Jun 2015
 *      Author: patrick
 */

#include "courseroomtime.h"
#include <sstream>


CourseRoomTime::CourseRoomTime(Course course, RoomTime roomT)
{
	this->course = course;
	venueTime = roomT;
	update = true;
	empty = false;
}

CourseRoomTime::CourseRoomTime()
{
	empty = true;
	Venue v("empty");
	RoomTime vt(0, v);
	Course c;
	this->course = c;
	this->venueTime = vt;

}

CourseRoomTime::~CourseRoomTime()
{

}


/**
 * Checks if the day of a courseroomtime object is the same as day in parameter
 * @param maxPeriod - Integer Number of Periods in day
 * @param day - Integer value of Day
 * @return - boolean, true if days are the same, else falsei
 */
bool CourseRoomTime::isDay(int maxPeriod, int day)
{
	int period = this->venueTime.getPeriod();
	int period_day =   period / maxPeriod;
	if (period_day == day)
		return true;

	return false;
}

/**
 * Checks if the period of the CourseRoomTime Object is the last period of the day
 * @param periods_in_day - number of periods in a day
 * @return - true if it is the last period, false if it is not
 */
bool CourseRoomTime::isLastPeriod(int periods_in_day)
{
	int period = this->venueTime.getPeriod();
	int per_conv = (period %  periods_in_day) + 1;
	//cout<<"\t\t\t\t Last period: " << periods_in_day << " Given Period: " << per_conv<<endl;
	if (periods_in_day == per_conv)
	{
		//cout<<"returning true"<<endl;
		return true;
	}

	return false;
}

/**
 * Checks if the period of the CourseRoomTime Object is the first period of the day
 * @param periods_in_day - number of periods in a day
 * @return - true if it is the last period, false if it is not
 */
bool CourseRoomTime::isFirstPeriod(int periods_in_day)
{
	int period = this->venueTime.getPeriod();
	int per_conv = (period %  periods_in_day);
	//cout<<"\t\t\t\t Last period: " << maxPeriod << " Given Period: " << period_in_day<<endl;
	if (per_conv == 0)
	{
		//cout<<"returning true"<<endl;
		return true;
	}

	return false;
}

Course CourseRoomTime::getCourse() const {
	return course;
}

void CourseRoomTime::setCourse(Course course) {
	this->course = course;
}

RoomTime CourseRoomTime::getVenueTime() const {
	return venueTime;
}

bool CourseRoomTime::isUpdate() const {
	return update;
}

void CourseRoomTime::setUpdate(bool update) {
	this->update = update;
}

void CourseRoomTime::setVenueTime(RoomTime venueTime) {
	this->venueTime = venueTime;
}

string CourseRoomTime::toString()
{
	std::stringstream sstm;
	//sstm<< "Course Code: " << this->course.getCode() << "  " << this->venueTime.toString();
	sstm<<"["<< this->course.getCode() << "-" << this->venueTime.getRoom().getName() << "-"<<this->venueTime.getPeriod()<<"]";
	return sstm.str();
}

bool operator== (const CourseRoomTime &c1, const CourseRoomTime &c2)
{
	if (c1.course == c2.course && c1.venueTime == c2.venueTime)
		return true;
	else
		return false;
}
