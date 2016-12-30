/*
 * smethods.cpp
 *
 *  Created on: 17 Jul 2015
 *      Author: patrick
 */
#include "smethods.h"
#include <algorithm>
#include <thread>


SM::SM()
{

}
double SM::get_wall_time()
{
	struct timeval time;
	if (gettimeofday(&time,NULL))
	{
		//  Handle error
		return 0;
	}
	return (double)time.tv_sec + (double)time.tv_usec * .000001;
}


double SM::get_cpu_time()
{
    return (double)clock() / CLOCKS_PER_SEC;
}

vector<CourseRoomTime> SM::getCRTsinPeriod(const int& period, const vector<CourseRoomTime>& timetable)
{
	vector<CourseRoomTime> crtinPeriod;
	for (auto &t:timetable)
	{
		CourseRoomTime tempCRT = t;
		if (tempCRT.getCourse().isEmpty() == false)
		{
			if (tempCRT.getVenueTime().getPeriod() == period )
				crtinPeriod.push_back(tempCRT);
		}
	}
	return crtinPeriod;
}

vector<string> SM::curriculumInTimetable(const vector<CourseRoomTime>& timetable)
{

	vector<string> curri;
	for (auto &t:timetable)  //loop through vector c++11 way
	{
		CourseRoomTime crt = t;
		if (crt.getCourse().isEmpty() == false)
		{
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

int SM::getConsecutiveViolations(const vector<CourseRoomTime>& timetable, const int& j, const int& periods_in_day)
{
	int consecutiveLectureViolations = 0;
	int check = (j + 1)%periods_in_day;
	bool first = false;
	bool last = false;
	if (check == 0)
		last = true;
	else if (check == 1)
		first = true;

	if (first == true) //means no previous period
	{
		vector<CourseRoomTime> periodsubsetTable = SM::getCRTsinPeriod(j, timetable);
		vector<CourseRoomTime> nextperiodsubsetTable = SM::getCRTsinPeriod(j+1, timetable);
		vector<string> curriPeriod = SM::curriculumInTimetable(periodsubsetTable);
		vector<string> nextcurriPeriod = SM::curriculumInTimetable(nextperiodsubsetTable);
		for (auto &curCode: curriPeriod)
		{
			std::vector<string>::iterator findCurriculum = std::find(nextcurriPeriod.begin(), nextcurriPeriod.end(), curCode);
			if (findCurriculum == nextcurriPeriod.end())
			{
				consecutiveLectureViolations++;
			}
		}
	}

	else if (last == true) //means no next period
	{
		vector<CourseRoomTime> periodsubsetTable = SM::getCRTsinPeriod(j, timetable);
		vector<CourseRoomTime> prevperiodsubsetTable = SM::getCRTsinPeriod(j-1, timetable);
		vector<string> curriPeriod = SM::curriculumInTimetable(periodsubsetTable);
		vector<string> prevcurriPeriod = SM::curriculumInTimetable(prevperiodsubsetTable);

		for (auto &curCode: curriPeriod)
		{
			std::vector<string>::iterator findCurriculum1 = std::find(prevcurriPeriod.begin(), prevcurriPeriod.end(), curCode);
			if (findCurriculum1 == prevcurriPeriod.end())
			{
				consecutiveLectureViolations++;
			}
		}

	}
	else if(first == false && last == false )
	{
		//not first or last period
		vector<CourseRoomTime> periodsubsetTable = SM::getCRTsinPeriod(j, timetable);
		vector<CourseRoomTime> nextperiodsubsetTable = SM::getCRTsinPeriod(j+1, timetable);
		vector<CourseRoomTime> prevperiodsubsetTable = SM::getCRTsinPeriod(j-1, timetable);
		vector<string> curriPeriod = SM::curriculumInTimetable(periodsubsetTable);
		vector<string> nextcurriPeriod = SM::curriculumInTimetable(nextperiodsubsetTable);
		vector<string> prevcurriPeriod = SM::curriculumInTimetable(prevperiodsubsetTable);

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
	return consecutiveLectureViolations;
}

int SM::NumberHCV(const vector<CourseRoomTime>& timetable,const int& maximumPeriod)
{
	int roomViolation = 0;
	int curriculumViolation = 0;
	int unavailableViolation = 0;
	int featureSizeViolation = 0;
	int lecturerViolation = 0;

	for (int i =0; i < maximumPeriod; i++)  //get violations in each period
	{
		vector<string> curGlobal;
		vector<string> roomGlobal;
		vector<string> lecturerGlobal;

		vector<Course> periodCourse =  SM::getPeriodCourse(i, timetable);

		vector<Course>::iterator cIterator = periodCourse.begin();
		while (cIterator != periodCourse.end())
		{
			Course c = *cIterator;

			//get curriculum violations
			vector<string> cpcurCodes = c.getCurriculaCodes();
			vector<string>::iterator curIterator = cpcurCodes.begin();
			int cpi =0;
			while (curIterator != cpcurCodes.end())
			{
				string cpCode = *curIterator;
				vector<string>::iterator findCPcode = find(curGlobal.begin(), curGlobal.end(), cpCode);
				if (findCPcode != curGlobal.end())
				{
					curriculumViolation++;
				}
				if (findCPcode == curGlobal.end())
				{
					curGlobal.push_back(cpCode);
				}
				cpi++;
				curIterator++;
			}

			//get room clash violations
			string roomName = SM::getVenueCourse(i, c, timetable);
			vector<string>::iterator findRName = find(roomGlobal.begin(), roomGlobal.end(), roomName);
			if (findRName != roomGlobal.end())
				roomViolation++;
			else if (findRName == roomGlobal.end())
				roomGlobal.push_back(roomName);

			//get room feature or size violations
			vector<string> fsConst = c.getRoomConstraint(); //must be in this vector else a violation

			/*
			vector<string>::iterator rcsItr = fsConst.begin();
			while (rcsItr != fsConst.end())
			{
				string rcs = * rcsItr;
				cout<<rcs<<"\t";
				rcsItr++;
			}
			cout<<endl;
			*/
			vector<string>::iterator findRoom = find(fsConst.begin(), fsConst.end(), roomName);
			if (findRoom == fsConst.end())
			{
				featureSizeViolation++;
				//cout<<c.getCode()<<" "<<roomName<<endl;;

			}


			//get Lecturer violations
			vector<string>::iterator findLecturer = find(lecturerGlobal.begin(), lecturerGlobal.end(), c.getLecturer());
			if (findLecturer != lecturerGlobal.end())
				lecturerViolation++;
			else if (findLecturer == lecturerGlobal.end())
				lecturerGlobal.push_back(c.getLecturer());

			//get unavailable period violations
			vector<int> unavailableConst = c.getUnavailableConstraint();
			vector<int>::iterator findUnavailablePeriod = find(unavailableConst.begin(), unavailableConst.end(), i);
			if (findUnavailablePeriod != unavailableConst.end())
				unavailableViolation++;
			cIterator++;
		}

		periodCourse.erase(periodCourse.begin(), periodCourse.end());
		lecturerGlobal.erase(lecturerGlobal.begin(), lecturerGlobal.end());
		roomGlobal.erase(roomGlobal.begin(), roomGlobal.end());
		curGlobal.erase(curGlobal.begin(), curGlobal.end());
	}
	int HCV = roomViolation + curriculumViolation + unavailableViolation + featureSizeViolation + lecturerViolation;
	//cout<< "\troomViolation: " <<roomViolation <<"\t"<<"curriculumViolation: "<<curriculumViolation<<"\t"<< "unavailableViolation: " <<unavailableViolation <<"\t" <<"featureSizeViolation: "<< featureSizeViolation <<"\t"<<"lecturerViolation: "<< lecturerViolation<<endl;;
	return HCV;
}


/**
 * Get the list of courses taken in a particular period from timetable
 * @param period
 * @return
 */
vector<Course> SM::getPeriodCourse(const int& period, const vector<CourseRoomTime>& timetable)
{
	vector<Course> cees;
	for (auto &c:timetable)
	{
		if (c.getVenueTime().getPeriod() == period)
		{
			Course cous = c.getCourse();
			if (cous.isEmpty() == false)
				cees.push_back(cous);
			//std::vector<Course>::iterator findCourse = std::find(cees.begin(), cees.end(), cous);
			//if (findCourse == cees.end())
				//cees.push_back(cous);
		}
	}
	return cees;
}


/**
 * Get the name of venue of course taken in a particular period from timetable
 * @param period, timetable and course
 * @return
 * Venue name
 */
string SM::getVenueCourse(const int& period, const Course& c, const vector<CourseRoomTime>& timetable)
{
	string v = "";
	for (auto &crt:timetable)
	{
		Course cous = crt.getCourse();
		if (cous == c && crt.getVenueTime().getPeriod() == period)
			return crt.getVenueTime().getRoom().getName();
	}
	return v;
}

void SM::periodViolation(const vector<CourseRoomTime>& timetable,const int& maximumPeriod, int i, int* violations)
{
	int curriculumViolation =0;
	int roomViolation = 0;
	int featureSizeViolation = 0;
	int lecturerViolation = 0;
	int unavailableViolation = 0;



	vector<string> curGlobal;
	vector<string> roomGlobal;
	vector<string> lecturerGlobal;

	vector<Course> periodCourse =  SM::getPeriodCourse(i, timetable);

	vector<Course>::iterator cIterator = periodCourse.begin();
	while (cIterator != periodCourse.end())
	{
		Course c = *cIterator;

		//get curriculum violations
		vector<string> cpcurCodes = c.getCurriculaCodes();
		vector<string>::iterator curIterator = cpcurCodes.begin();
		int cpi =0;
		while (curIterator != cpcurCodes.end())
		{
			string cpCode = *curIterator;
			vector<string>::iterator findCPcode = find(curGlobal.begin(), curGlobal.end(), cpCode);
			if (findCPcode != curGlobal.end())
			{
				curriculumViolation++;
			}
			if (findCPcode == curGlobal.end())
			{
				curGlobal.push_back(cpCode);
			}
			cpi++;
			curIterator++;
		}

		//get room clash violations
		string roomName = SM::getVenueCourse(i, c, timetable);
		vector<string>::iterator findRName = find(roomGlobal.begin(), roomGlobal.end(), roomName);
		if (findRName != roomGlobal.end())
			roomViolation++;
		else if (findRName == roomGlobal.end())
			roomGlobal.push_back(roomName);

		//get room feature or size violations
		vector<string> fsConst = c.getRoomConstraint(); //must be in this vector else a violation

		/*
		vector<string>::iterator rcsItr = fsConst.begin();
		while (rcsItr != fsConst.end())
		{
			string rcs = * rcsItr;
			cout<<rcs<<"\t";
			rcsItr++;
		}
		cout<<endl;
		*/
		vector<string>::iterator findRoom = find(fsConst.begin(), fsConst.end(), roomName);
		if (findRoom == fsConst.end())
		{
			featureSizeViolation++;
			//cout<<c.getCode()<<" "<<roomName<<endl;
			//cout<<"Feature Size Violation: "<<endl;

		}


		//get Lecturer violations
		vector<string>::iterator findLecturer = find(lecturerGlobal.begin(), lecturerGlobal.end(), c.getLecturer());
		if (findLecturer != lecturerGlobal.end())
			lecturerViolation++;
		else if (findLecturer == lecturerGlobal.end())
			lecturerGlobal.push_back(c.getLecturer());

		//get unavailable period violations
		vector<int> unavailableConst = c.getUnavailableConstraint();
		vector<int>::iterator findUnavailablePeriod = find(unavailableConst.begin(), unavailableConst.end(), i);
		if (findUnavailablePeriod != unavailableConst.end())
			unavailableViolation++;
		cIterator++;
	}

	*violations = curriculumViolation + roomViolation + featureSizeViolation + unavailableViolation + lecturerViolation;
}

int SM::ThreadNumberHCV(const vector<CourseRoomTime>& timetable,const  int& maximumPeriod)
{
	int n_hcv[maximumPeriod]; //create vector of number of constraint violations

	std::vector<std::thread> th;
    for(int j = 0; j < maximumPeriod; j++) //create threads of number of ants to run in parallel
    {
    	//th.push_back(std::thread(&ImproveTable::softWalk, this, &antWalk[j], ((i+1)*(j+1)), &n_scv[j], &cyclePath[j]));
    	//void SM::periodViolation(const vector<CourseRoomTime>& timetable,const int& maximumPeriod, int i, int *violations)
    	th.push_back(std::thread(&SM::periodViolation,  timetable, maximumPeriod, j, &n_hcv[j]));
    }


    for(auto &t : th)  //join all ants threads
    {
    	t.join();
    }

    int sum = 0;
    for (int i =0; i < maximumPeriod; i++){
    	sum+=n_hcv[i];
    	if (n_hcv[i] > 0)
    	{
    		cout<<"Violation in "<<i<<" "<<n_hcv[i]<<endl;
    	}

    }


	return sum;
}

FeasibleTable SM::iT;
ImproveTable SM::iiT;
Enhancement  SM::enhance;
int SM::numPeriods = 0;
int SM::sizeTable = 0;

