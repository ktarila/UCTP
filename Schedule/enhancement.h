/*
 * enhancement.h
 *
 *  Created on: 30 Aug 2015
 *      Author: patrick
 */

#ifndef ENHANCEMENT_H_
#define ENHANCEMENT_H_

#include "feasibletable.h"
#include "path.h"
#include "roompath.h"
#include <mutex>
#include <chrono>


class Enhancement{

private:
	vector<double>  adjacentMatrix;
	vector<vector<double> >  matrix;
	int periods_in_day;
	int maxPeriod;
	int bestSoFar;


	vector<Course> course;
	vector<string> curCodes;
	vector<Venue> rooms;
	vector<RoomTime> venueTime;
	vector<CourseRoomTime> fullSchedule;
	list<int> crtIndices;




	//vector<CourseRoomTime> global;
	int globalBest;

	//int index = 1;


	//Ant colony optimizaton parameters
	constexpr static double t_min = 0.01;    //minumum pheromone value
	constexpr static double t_max = 10;    //maximum pheromone value
	constexpr static double alpha = 7;      //heuristic weight -- high Ants less sensitive to pheromone trail more sensitive to heuristics
	constexpr static double beta = 3;       //trail weight     -- high means ants are less sensitive to heuristic more senstive to pheromone trail
	constexpr static double rho = 0.02;        //rate of evapouration -- high means fast evapouration rate (range between 0 and 1);



	vector<CourseRoomTime> getCRTsinPeriod(const int& period, const vector<CourseRoomTime>& timetable);
	vector<string> curriculumInTimetable(const vector<CourseRoomTime>& timetable);
	int getDay(int period);
	vector<CourseRoomTime> getCRTsinDay(const int& day, const vector<CourseRoomTime>& timetable);
	vector<Course> coursesInTimetable(const vector<CourseRoomTime>& timetable);
	void constructMatrix();
	void enhancementSequence(vector<int> *, vector<CourseRoomTime>*, int, int*, bool);
	int selectNextCRT(list<int>, int, int);
	CourseRoomTime moveToBest(int, vector<CourseRoomTime>*, bool );
	CourseRoomTime moveToBest2(int, vector<CourseRoomTime>* );
	void updateAntTrail(int numberSCV, vector<int> sequence);
	vector<CourseRoomTime> getClashList(const vector<CourseRoomTime>&, const CourseRoomTime&);
	vector<CourseRoomTime> antColonySoftThread(const int& numberAnts, const int& numberCycles, const int& num_ber, bool accept, vector<CourseRoomTime>* runSchedule);
	void evaporate();


public:
	Enhancement(FeasibleTable); //constructor
	Enhancement();
	~Enhancement();
	int NumberSCV(const vector<CourseRoomTime>& timetable, int* roomStabilityViolations, int* courseWorkingDayViolations, int* consecutiveLectureViolations, int *sizeViolations);
	int NumberSCV(int* roomStabilityViolations, int* courseWorkingDayViolations, int* consecutiveLectureViolations, int *sizeViolations);
	int NumberSCV(const vector<CourseRoomTime>& timetable);
	vector<CourseRoomTime> antColonySoft(const int& numberAnts, const int& numberCycles, const int& num_ber, bool accept, vector<CourseRoomTime>* runSchedule);
	vector<CourseRoomTime> runEnhancement(int, int, int);

};


#endif /* ENHANCEMENT_H_ */
