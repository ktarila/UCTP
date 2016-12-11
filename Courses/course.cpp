#include "course.h"
#include "../Schedule/smethods.h"

Course::Course(string code, string lecturer, int weeklyLectures, int minWorkingDays, int numStudents, int doubleLectures)
{
	this->code = code;
	this->lecturer = lecturer;
	this->weeklyLectures = weeklyLectures;
	this->minWorkingDays = minWorkingDays;
	this->numStudents = numStudents;
	this->doubleLectures = doubleLectures;
	this->empty =false;
	
}

Course::Course(string code){
	this->code = code;
	this->empty = false;
}

Course::Course()
{
	this->empty = true;

}


Course::~Course(){
	
}

bool operator!= (const Course &c1, const Course &c2)
{
	return  !(c1 == c2);
}

bool operator== (const Course &c1, const Course &c2)
{
	if (c1.code == c2.code)
		return true;
	else
		return false;
}

bool operator< (const Course &c1, const Course &c2)
{
	auto course1 = c1;
	auto course2 = c2;
	if (course1.getAvailable() < course2.getAvailable())
		return true;
	else
		return false;
}

bool operator> (const Course &c1, const Course &c2)
{
	return  !(c1 < c2);
}

int Course::getAvailable()
{
	int availableP = SM::numPeriods - this->getUnavailableConstraint().size();
	int availableR = this->getRoomConstraint().size();
	return availableP * availableR;
}


vector<string> Course::getCurriculaCodes()
{
	return this->curriculaCodes;
}

void Course::addCurriculaCodes(string curCode)
{
	this->curriculaCodes.push_back(curCode);

}

vector<int> Course::getUnavailableConstraint()
{
	return this->unavailableConstraint;
}

void Course::addUnavailableConstraint(int period)
{
	this->unavailableConstraint.push_back(period);

}

vector<string> Course::getRoomConstraint()
{
	return this->roomConstraint;
}

int Course::getAssignedPeriod() const {
	return assignedPeriod;
}

void Course::setAssignedPeriod(int assignedPeriod) {
	this->assignedPeriod = assignedPeriod;
}

string Course::getCode() const {
	return code;
}

int Course::getDoubleLectures() const {
	return doubleLectures;
}

string Course::getLecturer() const {
	return lecturer;
}

int Course::getMinWorkingDays() const {
	return minWorkingDays;
}

int Course::getNumStudents() const {
	return numStudents;
}

string Course::getTitle() const {
	return title;
}

bool Course::isEmpty() const {
	return empty;
}

int Course::getWeeklyLectures() const {
	return weeklyLectures;
}

void Course::addRoomConstraint(string roomCode)
{
	this->roomConstraint.push_back(roomCode);

}
