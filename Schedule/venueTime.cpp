/*
 * venueTime.cpp
 *
 *  Created on: 25 Jun 2015
 *      Author: patrick
 */


#include "venueTime.h"
#include <sstream>
#include <algorithm>


RoomTime::RoomTime(int period, Venue v): period(period), room(v)
{
	//this->period = period;
	//this->room = v;
}

RoomTime::RoomTime()
{

}

int RoomTime::getPeriod() const {
	return period;
}

void RoomTime::setPeriod(int period) {
	this->period = period;
}

Venue RoomTime::getRoom() const {
	//Venue v = *room;
	return this->room;
}

void RoomTime::setRoom(Venue room) {
	this->room = room;
}

RoomTime::~RoomTime()
{
	//delete room;

}

string RoomTime::toString()
{
	std::stringstream sstm;
	sstm<< "Period: " << period << "  " << this->getRoom().toString();
	return sstm.str();
}

bool operator== (const RoomTime &c1, const RoomTime &c2)
{
	if (c1.room == c2.room && c1.period == c2.period)
		return true;
	else
		return false;
}

bool operator!= (const RoomTime &c1, const RoomTime &c2)
{
	return  !(c1 == c2);
}


/**
 * @return
 * Get the week day given roomtime object
 */
int RoomTime::getDay(int periods_in_day)
{
	int day =   this->period / periods_in_day;
	return day;
}

bool RoomTime::canAssignCourse(const Course& course)
{
	//int courseSize = course.getNumStudents();
	//if (this->room.getCapacity() < courseSize)
		//return false;
	auto c = course;
	auto cup = c.getUnavailableConstraint();
	auto findPeriod = std::find(cup.begin(), cup.end(), this->period);
	if (findPeriod != cup.end())
		return false;

	return true;
}







