/*
 * feasibletable.h
 *
 *  Created on: 27 Jun 2015
 *      Author: patrick
 */

#ifndef FEASIBLETABLE_H_
#define FEASIBLETABLE_H_

#include "venueTime.h"
#include "../Curricula/curricula.h"
#include "courseroomtime.h"

#include <stdlib.h>
#include <stdio.h>
#include <list>

using namespace std;

class FeasibleTable {
private:
	vector<vector<double> >  adjacentMatrix;
	vector<Course> course;
	vector<RoomTime> venueTime;
	int periods_in_day;
	int maxPeriod;
	vector<string> curCodes;
	vector<int> indexReset;
	bool check; //parameter that checks if CRT object should be updated or not
	int globalBest;  // the best result so far, least number of HCV
	vector<CourseRoomTime> feasibleTable;

	//Ant colony optimizaton parameters
	constexpr static double t_min = 0.01;    //minumum pheromone value
	constexpr static double t_max = 10;    //maximum pheromone value
	constexpr static double alpha = 8;      //heuristic weight -- high Ants less sensitive to pheromone trail more sensitive to heuristics
	constexpr static double beta = 2;       //trail weight     -- high means ants are less sensitive to heuristic more senstive to pheromone trail
	constexpr static double rho = 0.05;        //rate of evapouration -- high means fast evapouration rate (range between 0 and 1);



	void createVenueTimeList(vector<Venue> rooms);
	void initializeAdjacentMatrix();
	void antGraphWalk(vector<CourseRoomTime>* timetable, int cycle, int* num_hcv, int* num_scv);
	RoomTime antCourseWalk(Course c, vector<RoomTime> tabuVenueTime, list<int> tabuDays, vector<string> curriculaPeriod[], vector<string> lecturerPeriod[], int cycle, vector<CourseRoomTime> timetable);
	vector<RoomTime> getVenueTimeVertices(const Course &);
	vector<RoomTime> filterVenueListTabu(vector<RoomTime>, vector<RoomTime>);   //VenueTime Tabu Filter
	vector<RoomTime> filterCurriculumPeriodTabu(Course c, vector<string> curriculaPeriod[], vector<RoomTime> fVenueTabu); //Curriclum period Tabu Filter
	vector<RoomTime> filterLecturerPeriodTabu(Course c, vector<string> lecturerPeriod[], vector<RoomTime> fCurPerTabu);  //Lecturer period Tabu Filter
	vector<RoomTime> filterDayListTabu(vector<RoomTime> roomTimes, list<int> dayTabu); //day Tabu filter;
	int getAdjacentCourseIndex(Course);
	int getAdjacentRoomTimeIndex(RoomTime);
	RoomTime selectRoom(double probability[], vector<RoomTime> venuetime, int cycle);
	vector<Course> getPeriodCourse(int period, vector<CourseRoomTime> timetable);
	string getVenueCourse(int period, Course c, vector<CourseRoomTime> timetable);
	void updateAntTrail(int HCViolations, vector<CourseRoomTime> timetable);
	void evapourate();


public:
	FeasibleTable(vector<Course>, vector<Venue>, int maxPeriod,  int periods_in_day, vector<Curricula>); //constructor
	FeasibleTable();
	~FeasibleTable(); //destructor
	int NumberHCV();
	int NumberHCV(vector<CourseRoomTime> timetable);
	void antColony(int numAnts, int numCycles);
	void printAdjMat();
	void printRoomTime();
	void antColonyThread(int numAnts, int numCycles);
	void writeTimetableToFile();
	vector<Course> getCourse() const;
	vector<string> getCurCodes() const;
	int getMaxPeriod() const;
	int getPeriodsInDay() const;
	vector<RoomTime> getVenueTime() const;
	vector<CourseRoomTime> getFeasibleTable() const;
	void setFeasibleTable(vector<CourseRoomTime> timetable);
	vector<Venue> rooms;
	//int static sizeTable;
	//int static numPeriods;
};


#endif /* FEASIBLETABLE_H_ */
