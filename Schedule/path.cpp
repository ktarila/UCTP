/*
 * path.cpp
 *
 *  Created on: 16 Jul 2015
 *      Author: patrick
 */


#include "path.h"
#include <algorithm>


Path::Path(int newPeriod, CourseRoomTime rowCRT, CourseRoomTime colCRT)
{
	this->period = newPeriod;
	this->colCRT = colCRT;
	this->rowCRT = rowCRT;
	this->update = true;

}

Path::Path()
{

}


Path::~Path()
{

}

bool Path::isUpdate() const {
	return update;
}


CourseRoomTime Path::getColCrt() const {
	return colCRT;
}

void Path::setColCrt(CourseRoomTime colCrt) {
	colCRT = colCrt;
}

void Path::setRowCrt(CourseRoomTime rowCrt) {
	rowCRT = rowCrt;
}

CourseRoomTime Path::getRowCrt() const {
	return rowCRT;
}

void Path::setUpdate(bool update) {
	this->update = update;
}

int Path::getPeriod() const {
	return period;
}

vector<CourseRoomTime> Path::applyPath(const vector<CourseRoomTime>& timetable, Path* tCRT)
{
	auto newT = timetable;
	CourseRoomTime returnCRT;
	Path newPath(1, returnCRT, returnCRT);
	//pointer to empty CRT if successful
	*tCRT = newPath;
	if (this->colCRT == this->rowCRT)
	{
		//cout<<"same swap "<<period<<" "<<crt.getVenueTime().getPeriod()<<endl;
		return timetable;
	}
	if (this->colCRT.getCourse().isEmpty() == true && this->rowCRT.getCourse().isEmpty() == false) //is a row move not swap
	{
		int rowIndex = -1;
		auto rowIterator = find(newT.begin(), newT.end(), this->rowCRT);
		if (rowIterator != newT.end())
		{
			rowIndex = distance(newT.begin(), rowIterator);
		}
		if (rowIndex != -1 )
		{
			/*auto colRTPeriod = this->colCRT.getVenueTime().getPeriod();
			auto newRowVenueTime = this->rowCRT.getVenueTime();
			newRowVenueTime.setPeriod(colRTPeriod);
			newT[rowIndex].setVenueTime(newRowVenueTime);*/

			auto colRT = this->colCRT.getVenueTime();
			newT[rowIndex].setVenueTime(colRT);

			//pointer to new rowCRT and empty column CRT
			newPath.setRowCrt(newT[rowIndex]);
			*tCRT = newPath;
			return newT;
		}
		else
		{
			cout<<"Row CRT not found for move"<<endl;
			return timetable;
		}

	}

	if (this->rowCRT.getCourse().isEmpty() == true && this->colCRT.getCourse().isEmpty() == false) //is a col move not swap
	{
		int colIndex = -1;
		auto colIterator = find(newT.begin(), newT.end(), this->colCRT);
		if (colIterator != newT.end())
		{
			colIndex = distance(newT.begin(), colIterator);
		}
		if (colIndex != -1 )
		{
			/*auto rowRTPeriod = this->rowCRT.getVenueTime().getPeriod();
			auto newColVenueTime = this->colCRT.getVenueTime();
			newColVenueTime.setPeriod(rowRTPeriod);
			newT[colIndex].setVenueTime(newColVenueTime);*/

			auto rowRT = this->rowCRT.getVenueTime();
			newT[colIndex].setVenueTime(rowRT);

			//pointer to new colCRT and empty row CRT
			newPath.setColCrt(newT[colIndex]);
			*tCRT = newPath;
			return newT;
		}
		else
		{
			cout<<"Col CRT not found for move"<<endl;
			return timetable;
		}

	}

	//it is a swap
	if (this->colCRT.getCourse().isEmpty() == false && this->rowCRT.getCourse().isEmpty() == false) {
		int rowIndex = -1;
		int colIndex = -1;
		auto rowIterator = find(newT.begin(), newT.end(), this->rowCRT);
		if (rowIterator != newT.end())
		{
			rowIndex = distance(newT.begin(), rowIterator);
		}
		auto colIterator = find(newT.begin(), newT.end(), this->colCRT);
		if (colIterator != newT.end())
		{
			colIndex = distance(newT.begin(), colIterator);
		}

		if (rowIndex != -1 && colIndex != -1)
		{
/*			auto rowPeriod = newT[rowIndex].getVenueTime().getPeriod();
			auto colPeriod = newT[colIndex].getVenueTime().getPeriod();

			auto newRowVenueTime = newT[rowIndex].getVenueTime();
			newRowVenueTime.setPeriod(colPeriod);

			auto newColVenueTime = newT[colIndex].getVenueTime();
			newColVenueTime.setPeriod(rowPeriod);


			newT[rowIndex].setVenueTime(newRowVenueTime);
			newT[colIndex].setVenueTime(newColVenueTime);*/


			auto rowCourse = newT[rowIndex].getCourse();
			auto colCourse = newT[colIndex].getCourse();
			newT[rowIndex].setCourse(colCourse);
			newT[colIndex].setCourse(rowCourse);

			Path newPath(1, newT[rowIndex], newT[colIndex]);

			//pointer to new rowCRT and new column CRT
			newPath.setRowCrt(newT[rowIndex]);
			newPath.setColCrt(newT[colIndex]);
			*tCRT = newPath;
			return newT;
		}
		else
		{
			cout<<"Row CRT or Col CRT not found for swap"<<endl;
			return timetable;
		}
	}

	//is a null swap -- both are empty
	if (this->colCRT.getCourse().isEmpty() == true && this->rowCRT.getCourse().isEmpty() == true)
	{
		cout<<" Is a swap that will not change the timetable"<<endl;
		return timetable;
	}

	cout<<" Error in apply path --- Should not get to this point"<<endl;
	return timetable;
}
