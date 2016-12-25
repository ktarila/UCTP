/*
 * enhancement.cpp
 *
 *  Created on: 30 Aug 2015
 *      Author: patrick
 */

#include "enhancement.h"
#include "improvetable.h"
#include "smethods.h"
#include <algorithm>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <thread>
#include <fstream>


Enhancement::Enhancement(FeasibleTable ftable)
{

	this->maxPeriod =  ftable.getMaxPeriod();
	this->periods_in_day = ftable.getPeriodsInDay();
	this->fullSchedule = ftable.getFeasibleTable();
	this->course = ftable.getCourse();
	this->rooms = ftable.rooms;
	this->curCodes = ftable.getCurCodes();
	this->venueTime = ftable.getVenueTime();
	//for (auto &vt:venueTime)
	//{
		//cout<<vt.toString()<<endl;
	//}


	this->bestSoFar = 50000;

	this->globalBest = this->NumberSCV(this->fullSchedule);
	this->constructMatrix();

}

Enhancement::Enhancement()
{

}

Enhancement::~Enhancement()
{

}

void Enhancement::constructMatrix()
{
/*	//dynamic allocation of adjacent matrix
	this->adjacentMatrix.resize(this->fullSchedule.size());
	//this->crtIndices.resize(this->fullSchedule.size());
	for (std::size_t i = 0; i < this->fullSchedule.size(); i++)
	{
		this->adjacentMatrix[i] = this->t_max;
		this->crtIndices.push_back(i);
	}*/

	//dynamic allocation of adjacent matrix
	this->matrix.resize(this->fullSchedule.size());
	for (std::size_t i =0; i < this->fullSchedule.size(); i++)
	{
		this->crtIndices.push_back(i);
		matrix[i].resize(this->fullSchedule.size());
		for( std::size_t j =0; j< this->fullSchedule.size(); j++)
			matrix[i][j]=this->t_max;
	}
}

int Enhancement::NumberSCV( int* roomStabilityViolations, int* courseWorkingDayViolations, int* consecutiveLectureViolations, int *sizeViolations)
{
	return this->NumberSCV(this->fullSchedule, roomStabilityViolations, courseWorkingDayViolations, consecutiveLectureViolations, sizeViolations);
}

int Enhancement::NumberSCV(const vector<CourseRoomTime>& timetable)
{
	int  roomStabilityViolations = 0; int  courseWorkingDayViolations = 0; int  consecutiveLectureViolations = 0; int sizeViolations=0;
	return this->NumberSCV(timetable, &roomStabilityViolations, &courseWorkingDayViolations, &consecutiveLectureViolations, &sizeViolations);

}

int Enhancement::NumberSCV(const vector<CourseRoomTime>& timetable,  int* rsv, int* cwdv, int* clv, int *sv)
{
	int roomStabilityViolations =   0;
	int courseWorkingDayViolations =   0;
	int consecutiveLectureViolations =   0;
	int sizeViolations = 0;

	int courseSize = this->course.size();
	int courseWorkingDays[courseSize];
	for (int i =0; i < courseSize; i++)
	{
		courseWorkingDays[i] = 0;
	}

	vector<string> courseRooms[courseSize];
	for (auto &crt:timetable)
	{
		if (crt.getCourse().isEmpty() == false)
		{
			string rC = crt.getVenueTime().getRoom().getName();
			int roomSize = crt.getVenueTime().getRoom().getCapacity();
			int numStud = crt.getCourse().getNumStudents();
			int rM = 0;
			if (roomSize < numStud)
			{
				int diff = numStud - roomSize;
				rM+=diff;
			}
			sizeViolations+=rM;
			std::vector<Course>::iterator findCourse = std::find(this->course.begin(), this->course.end(), crt.getCourse());
			int index = std::distance(this->course.begin(), findCourse);
			std::vector<string>::iterator findRoom = std::find(courseRooms[index].begin(), courseRooms[index].end(), rC);
			if (findRoom == courseRooms[index].end())
			{
				courseRooms[index].push_back(rC);
			}
		}
	}

	int lastDay = this->getDay(this->maxPeriod - 1);
	//cout<<maxPeriod<<" Last Period";
	for (int i =0; i <= lastDay; i++)
	{
		vector<CourseRoomTime> daysubsetTable = this->getCRTsinDay(i, timetable);  //course room time elements in day i
		vector<string> curriDay = this->curriculumInTimetable(daysubsetTable);      //curricula in day i
		vector<Course> courseDay = this->coursesInTimetable(daysubsetTable);

		//Getting number of course working days
		for (auto &cInDay:courseDay)
		{
			Course c = cInDay;
			std::vector<Course>::iterator findCourse = std::find(this->course.begin(), this->course.end(), c);
			int index = std::distance(this->course.begin(), findCourse);
			courseWorkingDays[index]++;

		}


		//Getting number of Consecutive curricula violations so far and number of lectures

		int startPeriod = i * this->periods_in_day;          //get first period in day i
		int endPeriod = startPeriod + this->periods_in_day -1;  //get last period in day i

		for (int j = startPeriod; j <= endPeriod; j++)
		{
			if (j == startPeriod) //means no previous period
			{
				vector<CourseRoomTime> periodsubsetTable = this->getCRTsinPeriod(j, daysubsetTable);
				vector<CourseRoomTime> nextperiodsubsetTable = this->getCRTsinPeriod(j+1, daysubsetTable);
				vector<string> curriPeriod = this->curriculumInTimetable(periodsubsetTable);
				vector<string> nextcurriPeriod = this->curriculumInTimetable(nextperiodsubsetTable);
				for (auto &curCode: curriPeriod)
				{
					std::vector<string>::iterator findCurriculum = std::find(nextcurriPeriod.begin(), nextcurriPeriod.end(), curCode);
					if (findCurriculum == nextcurriPeriod.end())
					{
						consecutiveLectureViolations++;
					}
				}
			}

			else if (j == endPeriod) //means no next period
			{
				vector<CourseRoomTime> periodsubsetTable = this->getCRTsinPeriod(j, daysubsetTable);
				vector<CourseRoomTime> prevperiodsubsetTable = this->getCRTsinPeriod(j-1, daysubsetTable);
				vector<string> curriPeriod = this->curriculumInTimetable(periodsubsetTable);
				vector<string> prevcurriPeriod = this->curriculumInTimetable(prevperiodsubsetTable);

				for (auto &curCode: curriPeriod)
				{
					std::vector<string>::iterator findCurriculum1 = std::find(prevcurriPeriod.begin(), prevcurriPeriod.end(), curCode);
					if (findCurriculum1 == prevcurriPeriod.end())
					{
						consecutiveLectureViolations++;
					}
				}

			}
			else if(j != endPeriod && j != startPeriod )
			{
				//not first or last period
				vector<CourseRoomTime> periodsubsetTable = this->getCRTsinPeriod(j, daysubsetTable);
				vector<CourseRoomTime> nextperiodsubsetTable = this->getCRTsinPeriod(j+1, daysubsetTable);
				vector<CourseRoomTime> prevperiodsubsetTable = this->getCRTsinPeriod(j-1, daysubsetTable);
				vector<string> curriPeriod = this->curriculumInTimetable(periodsubsetTable);
				vector<string> nextcurriPeriod = this->curriculumInTimetable(nextperiodsubsetTable);
				vector<string> prevcurriPeriod = this->curriculumInTimetable(prevperiodsubsetTable);

				for (auto &curCode: curriPeriod)
				{
					std::vector<string>::iterator findCurriculum = std::find(nextcurriPeriod.begin(), nextcurriPeriod.end(), curCode);
					std::vector<string>::iterator findCurriculum1 = std::find(prevcurriPeriod.begin(), prevcurriPeriod.end(), curCode);
					if (findCurriculum == nextcurriPeriod.end() && findCurriculum1 == prevcurriPeriod.end())
					{
						consecutiveLectureViolations++;
					}
				}
			}
		}

	}

	//finding number of minimum course working violations and room stability violations
	int index = 0;
	for (auto &c:this->course)
	{
		int minWork = c.getMinWorkingDays();
		if (minWork > courseWorkingDays[index])
		{
			int val = minWork - courseWorkingDays[index];
			courseWorkingDayViolations+= val;
		}

		//cout<<"Room Stability "<<c.getCode()<<":  "<<courseRooms[index].size()<<"\t Minimum Working Days: "<<c.getMinWorkingDays()<<" Table working days: "<<courseWorkingDays[index]<<endl;
		int v = courseRooms[index].size() - 1;
		roomStabilityViolations+=v;
		index++;

	}


	//cout<<"Room Stability violations: "<<roomStabilityViolations << " Course working days violations: " <<courseWorkingDayViolations << " consecutive lectures violations " <<consecutiveLectureViolations <<endl;

	*rsv = roomStabilityViolations;
	*cwdv = courseWorkingDayViolations;
	*clv = consecutiveLectureViolations;
	*sv = sizeViolations;
	return (sizeViolations + roomStabilityViolations + (5 * courseWorkingDayViolations) + (2 * consecutiveLectureViolations));
}



/**
 * Gets the subset of given timetable that occur on the specified period
 * @param period
 * @param timetable
 * @return ArrayList of CourseRoomTime objects from the timetable in the Specified period
 */

vector<CourseRoomTime> Enhancement::getCRTsinPeriod(const int& period, const vector<CourseRoomTime>& timetable)
{
	vector<CourseRoomTime> crtinPeriod;
	for (auto &t:timetable)
	{
		if (t.getCourse().isEmpty() == false)
		{
			CourseRoomTime tempCRT = t;
			if (tempCRT.getVenueTime().getPeriod() == period )
				crtinPeriod.push_back(tempCRT);
		}
	}
	return crtinPeriod;
}


/**
 * Gets the curricula in a timetable
 * @param timetable - Ctimetable - ArrayList of CourseRoomTime Objects
 * @return -  vector of curricula in the timetable
 */

vector<string> Enhancement::curriculumInTimetable(const vector<CourseRoomTime>& timetable)
{

	vector<string> curri;
	for (auto &t:timetable)  //loop through vector c++11 way
	{
		if (t.getCourse().isEmpty() == false)
		{
			CourseRoomTime crt = t;
			vector<string> curricula = crt.getCourse().getCurriculaCodes();
			vector<string>::iterator curIterator = curricula.begin();
			while (curIterator != curricula.end())
			{
				string curCode = *curIterator;
				vector<string>::iterator findCurCode = std::find(curri.begin(), curri.end(), curCode);
				if (findCurCode == curri.end())
					curri.push_back(curCode);
				curIterator++;
			}
		}
	}
	return curri;
}

/**
 *
 * @param period
 * @return
 * Get the week day given the period in the time table
 */

int Enhancement::getDay(int period)
{
	int day =   period /  this->periods_in_day;
	return day;
}

/**
 * Gets the subset of the given timetable that occur on the specified day
 * @param day
 * @param timetable
 * @return ArrayList of CourseRoomTime objects from given timetable in the Specified day
 */

vector<CourseRoomTime> Enhancement::getCRTsinDay(const int &day, const vector<CourseRoomTime> &timetable)
{
	vector<CourseRoomTime> crtInDay;

	for (auto &t:timetable)
	{
		if (t.getCourse().isEmpty() == false)
		{
			CourseRoomTime tempCRT = t;
			if (tempCRT.isDay(this->periods_in_day, day))
				crtInDay.push_back(tempCRT);
		}
	}
	return crtInDay;
}


/**
 * Gets the courses in a timetable
 * @param timetable - Ctimetable - ArrayList of CourseRoomTime Objects
 * @return -  vector of courses in the timetable
 */

vector<Course> Enhancement::coursesInTimetable(const vector<CourseRoomTime>& timetable)
{

	vector<Course> curri;
	for (auto &t:timetable)  //loop through vector c++11 way
	{
		if (t.getCourse().isEmpty() == false)
		{
			CourseRoomTime crt = t;
			Course c = crt.getCourse();
			vector<Course>::iterator findCourse = std::find(curri.begin(), curri.end(), c);
			if (findCourse == curri.end())
				curri.push_back(c);
		}
	}
	return curri;
}

void Enhancement::enhancementSequence(vector<int> * seq, vector<CourseRoomTime>* newSchedule, int ant, int* numberAntSCV, bool accept)
{
	vector<int> sequence(this->fullSchedule.size());
	auto schedule = this->fullSchedule;

	list<int> indices = this->crtIndices;
	int row =0;
	while (indices.size() > 0)
	{
		//select next CRT
		int nextCRT = this->selectNextCRT(indices, ant, row);
		std::list<int>::iterator findIndex = std::find(indices.begin(), indices.end(), nextCRT);


		//cout<<"New CRT size is: "<<indices.size()<<endl;
		sequence[row]=nextCRT;
		//move nextCRT to its best period and room
		auto singleClashCRT = this->moveToBest(nextCRT, &schedule, accept);
		indices.remove(nextCRT);
		//cout<<nextCRT<<",";
		row++;
	}
	//cout<<endl;
	*seq = sequence;
	*newSchedule = schedule;
	*numberAntSCV = this->NumberSCV(schedule);

}

CourseRoomTime Enhancement::moveToBest(int crtIndex, vector<CourseRoomTime>* timetable, bool accept )
{
	CourseRoomTime c;
	CourseRoomTime crt = this->fullSchedule[crtIndex];
	auto tempSchedule = *timetable;
	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	double number = rand_r(&seed)/(double)RAND_MAX;
	accept = true;

	std::vector<CourseRoomTime>::iterator findCRT = std::find(tempSchedule.begin(), tempSchedule.end(), crt);
	int indexInTS = std::distance(tempSchedule.begin(), findCRT);
	if (findCRT != tempSchedule.end())
	{
		int min = this->NumberSCV(tempSchedule);
		//int min = RAND_MAX;
		auto bestSchedule = tempSchedule;
		auto newTemp = tempSchedule;

		//swap with course // swap room // swap time

		auto swapTemp = tempSchedule;
		for (auto &swapCRT:swapTemp)
		{
			if (swapCRT.getCourse().isEmpty() == false)
			{

				//swap course
				newTemp = tempSchedule;
				std::vector<CourseRoomTime>::iterator tempITE = std::find(tempSchedule.begin(), tempSchedule.end(), swapCRT);
				int swapIndex = std::distance(tempSchedule.begin(), tempITE);
				auto swapC = swapCRT;
				int period1 = swapCRT.getVenueTime().getPeriod();
				int period2 = crt.getVenueTime().getPeriod();
				newTemp[indexInTS].setCourse(swapCRT.getCourse());
				newTemp[swapIndex].setCourse(crt.getCourse());
				int v1 = 0, v2=0;
				SM::periodViolation(newTemp, this->maxPeriod, period1, &v1);
				SM::periodViolation(newTemp, this->maxPeriod, period2, &v2);
				Course c1 = crt.getCourse();
				Course c2 = swapCRT.getCourse();
				bool l = true;
				number = rand_r(&seed)/(double)RAND_MAX;
				if (v1 == 0 && v2 == 0 &&  l)
				{
					int nSCurrent = this->NumberSCV(newTemp);
					if (nSCurrent < min)
					{
						//cout<<" best is swap "<<endl;
						min = nSCurrent;
						bestSchedule = newTemp;
					}
					if (nSCurrent == min && number > 0.5)
					{
						//cout<<" best is swap "<<endl;
						min = nSCurrent;
						bestSchedule = newTemp;
					}


				}

				//swap room
					newTemp = tempSchedule;
				RoomTime newcrtRoomTime = crt.getVenueTime();
				newcrtRoomTime.setRoom(swapCRT.getVenueTime().getRoom());
				RoomTime newswapRoomTime = swapCRT.getVenueTime();
				newcrtRoomTime.setRoom(crt.getVenueTime().getRoom());
				newTemp[indexInTS].setVenueTime(newcrtRoomTime);
				newTemp[swapIndex].setVenueTime(newswapRoomTime);

				SM::periodViolation(newTemp, this->maxPeriod, period1, &v1);
				SM::periodViolation(newTemp, this->maxPeriod, period2, &v2);
				if (v1 == 0 && v2 == 0)
				{
					int nSCurrent = this->NumberSCV(newTemp);
					if (nSCurrent < min)
					{
						//cout<<" best is swap "<<endl;
						min = nSCurrent;
						bestSchedule = newTemp;
					}

				}

				//swap time
				newTemp = tempSchedule;
				RoomTime roomtime_crt = crt.getVenueTime();
				roomtime_crt.setPeriod(period1);
				RoomTime roomtime_swap = swapCRT.getVenueTime();
				roomtime_swap.setPeriod(period2);
				newTemp[indexInTS].setVenueTime(roomtime_crt);
				newTemp[swapIndex].setVenueTime(roomtime_swap);
				SM::periodViolation(newTemp, this->maxPeriod, period1, &v1);
				SM::periodViolation(newTemp, this->maxPeriod, period2, &v2);
				if (v1 == 0 && v2 == 0)
				{
					int nSCurrent = this->NumberSCV(newTemp);
					if (nSCurrent < min)
					{
						//cout<<" best is swap "<<endl;
						min = nSCurrent;
						bestSchedule = newTemp;
					}

				}
	

			}
		}


		//bool moved = false;
		//move to period
/*		for (int i =0; i < this->maxPeriod; i++)
		{
			newTemp = tempSchedule;
			RoomTime roomtime_periodMove = crt.getVenueTime();
			roomtime_periodMove.setPeriod(i);
			newTemp[indexInTS].setVenueTime(roomtime_periodMove);
			int v1 = 0;
			SM::periodViolation(newTemp, this->maxPeriod, i, &v1);
			if (v1 == 0)
			{
				int nSCurrent = this->NumberSCV(newTemp);
				if (nSCurrent < min)
				{
					//cout<<" best is swap "<<endl;
					min = nSCurrent;
					bestSchedule = newTemp;
				}

			}

		}

		//move to room
		for (auto &r:this->rooms)
		{
			newTemp = tempSchedule;
			RoomTime roomtime_RoomMove = crt.getVenueTime();
			roomtime_RoomMove.setRoom(r);
			newTemp[indexInTS].setVenueTime(roomtime_RoomMove);
			int v1 = 0;
			SM::periodViolation(newTemp, this->maxPeriod, roomtime_RoomMove.getPeriod(), &v1);
			if (v1 == 0)
			{
				int nSCurrent = this->NumberSCV(newTemp);
				if (nSCurrent < min)
				{
					//cout<<" best is swap "<<endl;
					min = nSCurrent;
					bestSchedule = newTemp;
				}

			}
		}*/

		//move to new rt
		for (std::size_t i =0; i < this->venueTime.size(); i++)
		{
			auto eventP = this->venueTime[i];
			//bool l = true;
			//if (crt.getVenueTime() == eventP && accept == false)
				//l = false;
			//else
				//l = true;
			if (eventP.canAssignCourse(crt.getCourse()) && crt.getVenueTime() != eventP )
			{
				newTemp = tempSchedule;
				newTemp[indexInTS].setVenueTime(eventP);
				//check number of hard constraint violations in newTimetable CRT
				int violations = 0;
				SM::periodViolation(newTemp, this->maxPeriod, eventP.getPeriod(), &violations);
				if (violations == 0)
				{
					int nSCurrent = this->NumberSCV(newTemp);
					if (nSCurrent < min)
					{
						//moved = true;
						min = nSCurrent;
						bestSchedule = newTemp;
					}
					number = rand_r(&seed)/(double)RAND_MAX;
					if (nSCurrent == min && number > 0.5)
					{
						//cout<<" best is swap "<<endl;
						min = nSCurrent;
						bestSchedule = newTemp;
					}

				}
				else if ( violations > 0 && accept == true)
				{

					//cout<<"in violations more than zero"<<endl;
					//to do ---- check if it's a single CRT Violation and return it for next move
					auto clashList = this->getClashList(newTemp, newTemp[indexInTS]);
					//check if single clash
					/*
					cout<<clashList.size()<<endl;
					for (auto &clasL:clashList)
					{
						cout<<clasL.toString()<<" ";
					}
					cout<<endl;
					*/
					if (clashList.size() == 1)
					{
						//cout<<"clash size equal to 1"<<endl;
						auto copyNewTemp = newTemp;
						auto findCourse = std::find(copyNewTemp.begin(), copyNewTemp.end(), clashList[0]);
						auto cIndex = std::distance(copyNewTemp.begin(), findCourse);
						auto newC = this->moveToBest2(cIndex, &copyNewTemp);
						if (newC.empty == true)
						{
							//cout<<" Safely swapped "<<endl;
							int nSCurrent = this->NumberSCV(copyNewTemp);
							//int nSCurrent = this->NumberSCV(newTemp);
							if (nSCurrent < min)
							{
								min = nSCurrent;
								bestSchedule = copyNewTemp;
							}

						}
					}

				}

			}
		}
		/*
		if (!moved)
			cout<<" Best is swap"<<endl;
		else
			cout<<" Best is move"<<endl;
		*/
		*timetable = bestSchedule;
	}
	return c;
}

int Enhancement::selectNextCRT(list<int> timetableSet, int cycleSeed, int row)
{
	int sum=0;
	vector<double> probability(timetableSet.size());
	vector<int> index(timetableSet.size());

	int k= 0;
	for (auto &crtIndex:timetableSet)
	{
		//cout<<"CRT Index: "<<crtIndex;
		index[k] = crtIndex;

		//for single array
		/*
		index[k] = crtIndex;
		probability[k]=this->adjacentMatrix[crtIndex];
		//double trail = this->adjacentMatrix[crtIndex];
		//probability[k] = trail;
		sum+=this->adjacentMatrix[crtIndex];
		k++;
		*/



		//for multi dimentional array

		//probability[k]=this->matrix[row][crtIndex];
		double trail = this->matrix[row][crtIndex];
		double trailFactor = pow(trail, this->beta);
		probability[k] = trailFactor;
		sum+=probability[k];
		k++;

	}

	for (std::size_t j =0; j < probability.size(); j++)
	{
		probability[j]/=sum;
		//cout<< " " <<probability[j]<<", "<<j;
	}

	//int unsigned seed = time(NULL) * cycleSeed;
	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	double number = rand_r(&seed)/(double)RAND_MAX;
	//cout<<" Generated Random "<<number<<endl;

	double cumulativeProbability = 0;
	for (size_t i =0; i < probability.size(); i++)
	{
		//cout<<number<<" Index "<<i<<endl;
		cumulativeProbability += probability[i];
		//cout<<" Cumulative Probability: "<<cumulativeProbability<<" Probability "<<probability[i]<<"Size: "<<probability.size()<<endl;
		if ( number <= cumulativeProbability)
		{
			//cout<<" returning "<<i<<endl;
			return index[i];
		}
	}

	cout<<"Error in cumulative Probability"<<endl;
	return index[0];  //should not get to this point
}

vector<CourseRoomTime> Enhancement::antColonySoft(const int& numberAnts, const int& numberCycles, const int& num_ber, bool accept, vector<CourseRoomTime>* runSchedule)
{
	vector<CourseRoomTime> global = this->fullSchedule;
	vector<CourseRoomTime> globe;
	int globeBest = 1000000;

	for (int i =0; i < numberCycles; i++)
	{
		vector<CourseRoomTime> cycleBestSchedule;
		int cycleBestSCV = 50000;
		vector<int> cycleBestSequence;
		for (int j =0; j < numberAnts; j++)
		{
			vector<int> antSequence;
			vector<CourseRoomTime> antSchedule;
			int antSCV = 0;
			this->enhancementSequence(&antSequence, &antSchedule, j, &antSCV, accept);
			if (antSCV < cycleBestSCV)
			{
				cycleBestSequence = antSequence;
				cycleBestSchedule = antSchedule;
				cycleBestSCV = antSCV;
			}
			cout<<"Cycle: " << i << " Ant: " << j << " Num SCV violations: " << antSCV<<endl;
		}

		cout << i << ": Cycle Best is: " << cycleBestSCV << " Global Best is:  "<< globalBest << " Size: "<< global.size() << endl;
		int roomStabilityViolations = 0;
		int courseWorkingDayViolations = 0;
		int consecutiveLectureViolations = 0;
		int sizeViolations = 0;
		//auto temp = this->voteOutInviteIn(100, cb);
		//auto temp = antWalk[cycleBestIndex];
		int s = this->NumberSCV(cycleBestSchedule, &roomStabilityViolations,
				&courseWorkingDayViolations, &consecutiveLectureViolations, &sizeViolations);
		cout << "\t\t\tLocal Search Total SCV: " << s << " Room Stability: "
				<< roomStabilityViolations << " Course Working Day Violations: "
				<< courseWorkingDayViolations
				<< " size Violations "<< sizeViolations
				<< " Consecutive Lecture Violations: "
				<< consecutiveLectureViolations << endl;
		//cout<< " Best So far "<< this->bestSoFar <<endl;

		//updateAnt trail
		this->updateAntTrail(cycleBestSCV, cycleBestSequence);
		if (cycleBestSCV < this->globalBest)
		{
			this->globalBest = cycleBestSCV;
			global = cycleBestSchedule;
		}

		if (cycleBestSCV < globeBest)
		{
			globeBest = cycleBestSCV;
			globe = cycleBestSchedule;
		}

		if (this->globalBest == 0)
		{
			this->fullSchedule = global;
			return global;
		}
	}

	*runSchedule = globe;
	this->fullSchedule = global;
	return globe;
}


void Enhancement::updateAntTrail(int numSCV_cycleBest, vector<int> sequence)
{
	this->evaporate();

	double reward = 1 / (1 + (double) numSCV_cycleBest - (double)this->globalBest);

	if (this->globalBest >  numSCV_cycleBest)
		reward = 1;


	cout<<"\t\t\tReward is: "<<reward<<endl;

	auto size = fullSchedule.size();
	cout<<sequence.size()<<endl;

	for (size_t i = 0; i < size; i++ )
	{

		int index = sequence[i];
		//cout<<i<<","<<index<<endl;

         //For single matrix
		/*
		double positionPenalty = i/(double)size;
		double positionReward = reward - positionPenalty;
		if (positionReward < 0)
			positionReward = 0;
		this->adjacentMatrix[index]+=positionReward;

		if (this->adjacentMatrix[index] > this->t_max)
		{
			this->adjacentMatrix[index] = this->t_max;
		}

		if (this->adjacentMatrix[index] < this->t_min)
		{
			this->adjacentMatrix[index] = this->t_min;
		}
		*/


		//For double matrix
		this->matrix[i][index]+=reward;

		if (this->matrix[i][index] > this->t_max)
		{
			this->matrix[i][index] = this->t_max;
		}

		if (this->matrix[i][index] < this->t_min)
		{
			this->matrix[i][index] = this->t_min;
		}
	}
	//for (size_t i =0; i < this->adjacentMatrix.size(); i++)
		//cout<<this->adjacentMatrix[i]<<",";
	//cout<<endl;

}

void Enhancement::evaporate()
{
	double evaporate = 1 - this->rho;
	int row = this->fullSchedule.size();
	for (int i =0; i < row; i++)
	{
		//this->adjacentMatrix[i] *= evaporate;

		for (int j = 0; j < row; j++)
		{
			matrix[i][j] *= evaporate;
		}


	}
}

vector<CourseRoomTime> Enhancement::getClashList(const vector<CourseRoomTime>& timetable, const CourseRoomTime& crt)
{
	vector<CourseRoomTime> clashList;
	int period = crt.getVenueTime().getPeriod();
	auto crtInP = this->getCRTsinPeriod(period, timetable);
	auto crtRoom = crt.getVenueTime().getRoom();
	auto crtCurricula = crt.getCourse().getCurriculaCodes();
	auto crtLecturer = crt.getCourse().getLecturer();
	for (auto &cpn:crtInP)
	{
		auto cp = cpn;
		bool l = true;
		int number = 0;
		if (cp == crt)
		{
			l = false;
			number++;
		}
		if (cp == crt && number > 1)
		{
			clashList.push_back(crt);
		}
		if (l)
		{
			//check room
			auto cpRoom = cp.getVenueTime().getRoom();
			if (cpRoom == crtRoom)
				clashList.push_back(cp);
			//check lecturer
			auto cpLecturer = cp.getCourse().getLecturer();
			if (cpLecturer == crtLecturer)
				clashList.push_back(cp);
			//check curriculum clash
			auto cpCurricula = cp.getCourse().getCurriculaCodes();
			for (auto &cpCurC: cpCurricula)
			{
				std::vector<string>::iterator findCurr = std::find(crtCurricula.begin(), crtCurricula.end(), cpCurC);
				if (findCurr != crtCurricula.end())
					clashList.push_back(cp);

			}


		}
	}

	return clashList;
}

CourseRoomTime Enhancement::moveToBest2(int crtIndex, vector<CourseRoomTime>* timetable )
{
	//cout<<" in move to best 2"<<endl;
	CourseRoomTime c;
	auto t = *timetable;
	CourseRoomTime crt = t[crtIndex];
	auto tempSchedule = *timetable;
	int found = 0;
	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	double number = rand_r(&seed)/(double)RAND_MAX;

	std::vector<CourseRoomTime>::iterator findCRT = std::find(tempSchedule.begin(), tempSchedule.end(), crt);
	int indexInTS = std::distance(tempSchedule.begin(), findCRT);
	CourseRoomTime crT = *findCRT;
	if (findCRT != tempSchedule.end() && crT.getCourse().isEmpty() == false)
	{
		//int min = this->NumberSCV(tempSchedule);
		int min = RAND_MAX;
		auto bestSchedule = tempSchedule;
		auto newTemp = tempSchedule;
		for (std::size_t i =0; i < this->venueTime.size(); i++)
		{
			auto eventP = this->venueTime[i];
			if (eventP.canAssignCourse(crt.getCourse()))
			{
				newTemp = tempSchedule;
				newTemp[indexInTS].setVenueTime(eventP);
				//check number of hard constraint violations in newTimetable CRT
				int violations = 0;
				SM::periodViolation(newTemp, this->maxPeriod, eventP.getPeriod(), &violations);
				if (violations == 0)
				{
					int nSCurrent = this->NumberSCV(newTemp);
					if (nSCurrent < min)
					{
						min = nSCurrent;
						bestSchedule = newTemp;
					}
					number = rand_r(&seed)/(double)RAND_MAX;
					if (nSCurrent == min && number > 0.5)
					{
						//cout<<" best is swap "<<endl;
						min = nSCurrent;
						bestSchedule = newTemp;
					}
					found++;

				}
				else if ( violations > 0)
				{
					//second CRT return Old
					//return crt;

				}

			}
		}

		if (found == 0)
		{
			//no available alternative
			return crt;
		}
		*timetable = bestSchedule;
	}
	return c;
}

vector<CourseRoomTime> Enhancement::runEnhancement(int numIterations, int numAnts, int numCycles)
{
	vector<CourseRoomTime> iteBest = this->fullSchedule;
	vector<CourseRoomTime> iterationSchedule = this->fullSchedule;
	bool accept = false;
	auto minimum = this->NumberSCV(iteBest);
	if (minimum == 0)
		return iteBest;
	int i =0; int consecutive = 0;
	//for (int i =0; i < numIterations; i++)
	while(i < numIterations && consecutive < 3)
	{
		cout<<"Iteration: "<<i<<endl;
		//this->constructMatrix();
		//auto e = this->antColonySoft(numAnts, numCycles, i);
		if (accept == true)
			cout<<"accept is true"<<endl;
		else
			cout<<"accept is false"<<endl;

		auto pastSCV = this->NumberSCV(iterationSchedule);
		auto e = this->antColonySoftThread(numAnts, numCycles, i, accept, &iterationSchedule);
/*		if (accept == true)
		{
			accept = false;
		}*/
		auto currentSCV = this->NumberSCV(iterationSchedule);
		if (pastSCV <= currentSCV)
		{
			consecutive++;
			/*
			bool x = !accept;
			accept = x;


			*/
			//accept = true;
/*			if (accept == true)
			{
				cout<<"Reached the minima ... likely local"<<endl;
				return iteBest;
			}*/
			if (accept == true)
				accept = false;
			else
				accept = true;
			//accept = false;
/*			cout<<"Going to improve table to escape local minima"<<endl;
			ImproveTable impTable(this->rooms, this->curCodes, this->course, e, this->maxPeriod-1, this->periods_in_day);
			iterationSchedule = impTable.antColonySoftThread(8, 10, i);
			e = iterationSchedule;
			this->fullSchedule = e;*/
		}
		else
		{
			consecutive = 0;
		}

		auto iterationSCV = this->NumberSCV(e);
		cout<<iterationSCV<<" iteration scv"<<endl;
		if (minimum > iterationSCV)
		{
			minimum=iterationSCV;
			iteBest = e;
		}
		if (minimum == 0)
		{
			return iteBest;
		}
		i++;
	}
	return iteBest;
}

vector<CourseRoomTime> Enhancement::antColonySoftThread(const int& numberAnts, const int& numberCycles, const int& num_ber, bool accept, vector<CourseRoomTime>* runSchedule)
{
	vector<CourseRoomTime> global = this->fullSchedule;
	vector<CourseRoomTime> thisSchedule;
	int thisSCVSch = 1000000;

	int i = 0;
	//int consecutive = 0;
	while (i < numberCycles)
	{
		//int cycleb = 5000000;
		vector<CourseRoomTime>* antSchedule = new vector<CourseRoomTime>[numberAnts]; //create vector of results
		vector<int>* antSequence = new vector<int>[numberAnts];
		int num_scv[numberAnts]; //create vector of number of constraint violations
		std::vector<std::thread> th;
		for(int j = 0; j < numberAnts; j++) //create threads of number of ants to run in parallel
		{
			th.push_back(std::thread(&Enhancement::enhancementSequence, this, &antSequence[j], &antSchedule[j], ((i+1)*(j+1)), &num_scv[j], accept));
		}
		for(auto &t : th)  //join all ants threads
		{
			t.join();
		}

		//find cycle best;
		int cycleb = num_scv[0];
		int cycleBestIndex = 0;
		for (int k = 0; k < numberAnts; k++)
		{
			//int num_hcv = this->NumberHCV(antWalk[k]);
			if (num_scv[k] < cycleb)
			{
				cycleb = num_scv[k];
				cycleBestIndex = k;
			}
			cout<<"Cycle: " << i << " Ant: " << k << " Num SCV violations: " << num_scv[k]<<endl;
			if (num_scv[k] == 0)
			{
				global = antSchedule[k];
				this->fullSchedule = global;
				delete[] antSchedule;
				delete[] antSequence;
				return global;

			}
		}

		//update trail
		//this->updateAntTrail(cycleb, antSequence[cycleBestIndex]);
		//cout<<antWalk[cycleBestIndex].size() << " Size of cycle best "<<endl;
		if (cycleb < this->globalBest)
		{
			this->globalBest = cycleb;
			global = antSchedule[cycleBestIndex];
		}

		if (cycleb < thisSCVSch)
		{
			thisSCVSch = cycleb;
			thisSchedule = antSchedule[cycleBestIndex];
		}
		int roomStabilityViolations = 0;
		int courseWorkingDayViolations = 0;
		int consecutiveLectureViolations = 0;
		int sizeViolations = 0;
		//auto temp = this->voteOutInviteIn(100, cb);
		//auto temp = antWalk[cycleBestIndex];
		int s = this->NumberSCV(antSchedule[cycleBestIndex], &roomStabilityViolations,
				&courseWorkingDayViolations, &consecutiveLectureViolations, &sizeViolations);
		cout << "\t\t\tLocal Search Total SCV: " << s << " Room Stability: "
				<< roomStabilityViolations << " Course Working Day Violations: "
				<< courseWorkingDayViolations
				<< " Size violations: "<< sizeViolations
				<< " Consecutive Lecture Violations: "
				<< consecutiveLectureViolations << endl;
		cout<<"Global Best is: " << this->globalBest<<endl;

		delete[] antSchedule;
		delete[] antSequence;
		i++;
	}

	*runSchedule = thisSchedule;
	this->fullSchedule = global;
	return global;

}

vector<CourseRoomTime> Enhancement::applyEnhancementSequence(vector<int> &seq, vector<CourseRoomTime>& timetable,  bool accept)
{
	vector<int> sequence(this->fullSchedule.size());
	auto schedule = timetable;
	for (std::size_t i = 0; i < seq.size(); i++ ){
		int nextCRT = seq[i];

		//CRT is not out of bounds
		if (nextCRT <= (int)timetable.size()){
			auto tempSchedule = this->seqMoveToBest(nextCRT, schedule, accept);
			schedule = tempSchedule;
		}

	}
	return schedule;
}


vector<CourseRoomTime> Enhancement::seqMoveToBest(int crtIndex, vector<CourseRoomTime> &timetable, bool accept )
{
	CourseRoomTime c;
	CourseRoomTime crt = timetable[crtIndex];
	auto tempSchedule = timetable;
	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	double number = rand_r(&seed)/(double)RAND_MAX;
	

	std::vector<CourseRoomTime>::iterator findCRT = std::find(tempSchedule.begin(), tempSchedule.end(), crt);
	int indexInTS = std::distance(tempSchedule.begin(), findCRT);
	if (findCRT != tempSchedule.end())
	{
		int min = this->NumberSCV(tempSchedule);
		//int min = RAND_MAX;
		auto bestSchedule = tempSchedule;
		auto newTemp = tempSchedule;

		//swap with course // swap room // swap time

		auto swapTemp = tempSchedule;
		for (auto &swapCRT:swapTemp)
		{
			if (swapCRT.getCourse().isEmpty() == false)
			{

				//swap course
				newTemp = tempSchedule;
				std::vector<CourseRoomTime>::iterator tempITE = std::find(tempSchedule.begin(), tempSchedule.end(), swapCRT);
				int swapIndex = std::distance(tempSchedule.begin(), tempITE);
				auto swapC = swapCRT;
				int period1 = swapCRT.getVenueTime().getPeriod();
				int period2 = crt.getVenueTime().getPeriod();
				newTemp[indexInTS].setCourse(swapCRT.getCourse());
				newTemp[swapIndex].setCourse(crt.getCourse());
				int v1 = 0, v2=0;
				SM::periodViolation(newTemp, this->maxPeriod, period1, &v1);
				SM::periodViolation(newTemp, this->maxPeriod, period2, &v2);
				Course c1 = crt.getCourse();
				Course c2 = swapCRT.getCourse();
				bool l = true;
				number = rand_r(&seed)/(double)RAND_MAX;
				if (v1 == 0 && v2 == 0 &&  l)
				{
					int nSCurrent = this->NumberSCV(newTemp);
					if (nSCurrent < min)
					{
						//cout<<" best is swap "<<endl;
						min = nSCurrent;
						bestSchedule = newTemp;
					}
					if (nSCurrent == min && number > 0.5)
					{
						//cout<<" best is swap "<<endl;
						min = nSCurrent;
						bestSchedule = newTemp;
					}


				}

				//swap room
					newTemp = tempSchedule;
				RoomTime newcrtRoomTime = crt.getVenueTime();
				newcrtRoomTime.setRoom(swapCRT.getVenueTime().getRoom());
				RoomTime newswapRoomTime = swapCRT.getVenueTime();
				newcrtRoomTime.setRoom(crt.getVenueTime().getRoom());
				newTemp[indexInTS].setVenueTime(newcrtRoomTime);
				newTemp[swapIndex].setVenueTime(newswapRoomTime);

				SM::periodViolation(newTemp, this->maxPeriod, period1, &v1);
				SM::periodViolation(newTemp, this->maxPeriod, period2, &v2);
				if (v1 == 0 && v2 == 0)
				{
					int nSCurrent = this->NumberSCV(newTemp);
					if (nSCurrent < min)
					{
						//cout<<" best is swap "<<endl;
						min = nSCurrent;
						bestSchedule = newTemp;
					}

				}

				//swap time
				newTemp = tempSchedule;
				RoomTime roomtime_crt = crt.getVenueTime();
				roomtime_crt.setPeriod(period1);
				RoomTime roomtime_swap = swapCRT.getVenueTime();
				roomtime_swap.setPeriod(period2);
				newTemp[indexInTS].setVenueTime(roomtime_crt);
				newTemp[swapIndex].setVenueTime(roomtime_swap);
				SM::periodViolation(newTemp, this->maxPeriod, period1, &v1);
				SM::periodViolation(newTemp, this->maxPeriod, period2, &v2);
				if (v1 == 0 && v2 == 0)
				{
					int nSCurrent = this->NumberSCV(newTemp);
					if (nSCurrent < min)
					{
						//cout<<" best is swap "<<endl;
						min = nSCurrent;
						bestSchedule = newTemp;
					}

				}
	

			}
		}

		//move to new rt
		for (std::size_t i =0; i < this->venueTime.size(); i++)
		{
			auto eventP = this->venueTime[i];
			//bool l = true;
			//if (crt.getVenueTime() == eventP && accept == false)
				//l = false;
			//else
				//l = true;
			if (eventP.canAssignCourse(crt.getCourse()) && crt.getVenueTime() != eventP )
			{
				newTemp = tempSchedule;
				newTemp[indexInTS].setVenueTime(eventP);
				//check number of hard constraint violations in newTimetable CRT
				int violations = 0;
				SM::periodViolation(newTemp, this->maxPeriod, eventP.getPeriod(), &violations);
				if (violations == 0)
				{
					int nSCurrent = this->NumberSCV(newTemp);
					if (nSCurrent < min)
					{
						//moved = true;
						min = nSCurrent;
						bestSchedule = newTemp;
					}
					number = rand_r(&seed)/(double)RAND_MAX;
					if (nSCurrent == min && number > 0.5)
					{
						//cout<<" best is swap "<<endl;
						min = nSCurrent;
						bestSchedule = newTemp;
					}

				}
				else if ( violations > 0 && accept == true)
				{

					//cout<<"in violations more than zero"<<endl;
					//to do ---- check if it's a single CRT Violation and return it for next move
					auto clashList = this->getClashList(newTemp, newTemp[indexInTS]);
					//check if single clash
					/*
					cout<<clashList.size()<<endl;
					for (auto &clasL:clashList)
					{
						cout<<clasL.toString()<<" ";
					}
					cout<<endl;
					*/
					if (clashList.size() == 1)
					{
						//cout<<"clash size equal to 1"<<endl;
						auto copyNewTemp = newTemp;
						auto findCourse = std::find(copyNewTemp.begin(), copyNewTemp.end(), clashList[0]);
						auto cIndex = std::distance(copyNewTemp.begin(), findCourse);
						auto newC = this->moveToBest2(cIndex, &copyNewTemp);
						if (newC.empty == true)
						{
							//cout<<" Safely swapped "<<endl;
							int nSCurrent = this->NumberSCV(copyNewTemp);
							//int nSCurrent = this->NumberSCV(newTemp);
							if (nSCurrent < min)
							{
								min = nSCurrent;
								bestSchedule = copyNewTemp;
							}

						}
					}

				}

			}
		}
		/*
		if (!moved)
			cout<<" Best is swap"<<endl;
		else
			cout<<" Best is move"<<endl;
		*/
		return bestSchedule;
	}
	return timetable;
	
}
