/*
 * roompath.cpp
 *
 *  Created on: 28 Aug 2015
 *      Author: patrick
 */

/*
 * path.cpp
 *
 *  Created on: 16 Jul 2015
 *      Author: patrick
 */


#include "roompath.h"
#include <algorithm>
#include "smethods.h"


RoomPath::RoomPath(Venue room, CourseRoomTime crt, int roomIndex)
{
	this->room = room;
	this->courseRTime = crt;
	this->roomIndex = roomIndex;
	this->update = true;


}

RoomPath::RoomPath()
{

}


RoomPath::~RoomPath()
{

}

CourseRoomTime RoomPath::getCourseRTime() const {
	return courseRTime;
}

Venue RoomPath::getRoom() const {
	return room;
}

bool RoomPath::isUpdate() const {
	return update;
}

void RoomPath::setUpdate(bool update) {
	this->update = update;
}

int RoomPath::getRoomIndex() const {
	return roomIndex;
}

vector<CourseRoomTime> RoomPath::applyRoomPath(const vector<CourseRoomTime>& timetable, CourseRoomTime* tCRT, int maxP)
{
	auto newT = timetable;
	auto crt = this->courseRTime;
	CourseRoomTime copyCRT = crt;
	RoomTime rt(crt.getVenueTime().getPeriod(), room);
	CourseRoomTime ncrt(crt.getCourse(), rt);
	if (ncrt == crt)
	{
		//cout<<"same swap "<<period<<" "<<crt.getVenueTime().getPeriod()<<endl;
		return timetable;
	}

	//cout<<crt.toString()<<"Path CRT"<<endl;
	//cout<<ncrt.toString()<<"New CRT"<<endl;
	//cout<<newT.size();
	//cout<<timetable.size();
	auto cIndex = find(newT.begin(), newT.end(), crt);
	if (cIndex != newT.end())
	{
		//int ind = distance(newT.begin(), cIndex);
		//CourseRoomTime* crtt = &(*cIndex);
		//crtt->getVenueTime().setPeriod(period);
		newT.erase(cIndex);
		newT.push_back(ncrt);


		int pViolations;
		SM::periodViolation(newT, maxP, ncrt.getVenueTime().getPeriod(), &pViolations);
		if (pViolations != 0)
		{
			 newT = timetable;
		}


		//newT[ind].setVenueTime(tempVT);
	}

	if (cIndex == newT.end())
	{

		cout<<"Something went wrong in assignment not found"<<crt.toString() <<endl;
	}
	*tCRT = ncrt;
	return newT;
}




