/*
 * improvetable.cpp
 *
 *  Created on: 10 Jul 2015
 *      Author: patrick
 */

#include "improvetable.h"
#include "path.h"
#include "smethods.h"
#include <algorithm>
#include <chrono>
#include <fstream>
#include <functional>
#include <queue> // std::queue
#include <thread>

// random generator function:
int
myrandomI(int i)
{
  unsigned seed1 = std::chrono::system_clock::now().time_since_epoch().count();
  // double seed1 = SM::get_cpu_time();
  // std::cout<<seed1<<" "<<i<<std::endl;
  std::minstd_rand0 g1(seed1);
  return g1() % i;
}

ImproveTable::ImproveTable(vector<RoomTime> venueTime, vector<Venue> rooms,
                           vector<string> curCodes, vector<Course> course,
                           vector<CourseRoomTime> timetable, int maxPeriod,
                           int periods_in_day)
  : course(course)
  , curCodes(curCodes)
  , rooms(rooms)
  , venueTime(venueTime)
  , fullSchedule(timetable)
{
  this->maxPeriod = maxPeriod;
  this->periods_in_day = periods_in_day;
  this->bestSoFar = 50000;

  // dynamic allocation of adjacent matrix
  // cout<<"resizing adjacent matrix"<<endl;
  this->adjacentMatrix.resize(this->fullSchedule.size());
  for (std::size_t i = 0; i < this->fullSchedule.size(); i++) {
    adjacentMatrix[i].resize(this->fullSchedule.size());
  }

  // this->globalBest = this->NumberSCV(this->fullSchedule);
  // cout<<"initializing adjacent matrix"<<endl;
  this->globalBest = this->bestSoFar;
  this->initializeMatrix();
  // cout<<"Finished constructor"<<endl;
}

int
ImproveTable::getMaxPeriod()
{
  return maxPeriod;
}
ImproveTable::~ImproveTable()
{
}

ImproveTable::ImproveTable()
{
}

int
ImproveTable::NumberSCV()
{
  return this->NumberSCV(this->fullSchedule);
}

int
ImproveTable::NumberSCV(const vector<CourseRoomTime>& timetable)
{
  int roomStabilityViolations = 0;
  int courseWorkingDayViolations = 0;
  int consecutiveLectureViolations = 0;
  int sizeViolations = 0;
  return this->NumberSCV(timetable, &roomStabilityViolations,
                         &courseWorkingDayViolations,
                         &consecutiveLectureViolations, &sizeViolations);
}

int
ImproveTable::NumberSCV(const vector<CourseRoomTime>& timetable, int* rsv,
                        int* cwdv, int* clv, int* sv)
{
  int roomStabilityViolations = 0;
  int courseWorkingDayViolations = 0;
  int consecutiveLectureViolations = 0;
  int sizeViolations = 0;

  int courseSize = this->course.size();
  int courseWorkingDays[courseSize];
  for (int i = 0; i < courseSize; i++) {
    courseWorkingDays[i] = 0;
  }

  vector<string> courseRooms[courseSize];
  for (auto& crt : timetable) {
    if (crt.getCourse().isEmpty() == false) {
      string rC = crt.getVenueTime().getRoom().getName();
      int roomSize = crt.getVenueTime().getRoom().getCapacity();
      int numStud = crt.getCourse().getNumStudents();
      int rM = 0;
      if (roomSize < numStud) {
        int diff = numStud - roomSize;
        rM += diff;
      }
      sizeViolations += rM;
      std::vector<Course>::iterator findCourse =
        std::find(this->course.begin(), this->course.end(), crt.getCourse());
      int index = std::distance(this->course.begin(), findCourse);
      std::vector<string>::iterator findRoom =
        std::find(courseRooms[index].begin(), courseRooms[index].end(), rC);
      if (findRoom == courseRooms[index].end()) {
        courseRooms[index].push_back(rC);
      }
    }
  }

  int lastDay = this->getDay(this->maxPeriod - 1);
  // cout<<maxPeriod<<" Last Period";
  for (int i = 0; i <= lastDay; i++) {
    vector<CourseRoomTime> daysubsetTable =
      this->getCRTsinDay(i, timetable); // course room time elements in day i
    vector<string> curriDay =
      this->curriculumInTimetable(daysubsetTable); // curricula in day i
    vector<Course> courseDay = this->coursesInTimetable(daysubsetTable);

    // Getting number of course working days
    for (auto& cInDay : courseDay) {
      Course c = cInDay;
      std::vector<Course>::iterator findCourse =
        std::find(this->course.begin(), this->course.end(), c);
      int index = std::distance(this->course.begin(), findCourse);
      courseWorkingDays[index]++;
    }

    // Getting number of Consecutive curricula violations so far and number of
    // lectures

    int startPeriod = i * this->periods_in_day; // get first period in day i
    int endPeriod =
      startPeriod + this->periods_in_day - 1; // get last period in day i

    for (int j = startPeriod; j <= endPeriod; j++) {
      if (j == startPeriod) // means no previous period
      {
        vector<CourseRoomTime> periodsubsetTable =
          this->getCRTsinPeriod(j, daysubsetTable);
        vector<CourseRoomTime> nextperiodsubsetTable =
          this->getCRTsinPeriod(j + 1, daysubsetTable);
        vector<string> curriPeriod =
          this->curriculumInTimetable(periodsubsetTable);
        vector<string> nextcurriPeriod =
          this->curriculumInTimetable(nextperiodsubsetTable);
        for (auto& curCode : curriPeriod) {
          std::vector<string>::iterator findCurriculum =
            std::find(nextcurriPeriod.begin(), nextcurriPeriod.end(), curCode);
          if (findCurriculum == nextcurriPeriod.end()) {
            consecutiveLectureViolations++;
          }
        }
      }

      else if (j == endPeriod) // means no next period
      {
        vector<CourseRoomTime> periodsubsetTable =
          this->getCRTsinPeriod(j, daysubsetTable);
        vector<CourseRoomTime> prevperiodsubsetTable =
          this->getCRTsinPeriod(j - 1, daysubsetTable);
        vector<string> curriPeriod =
          this->curriculumInTimetable(periodsubsetTable);
        vector<string> prevcurriPeriod =
          this->curriculumInTimetable(prevperiodsubsetTable);

        for (auto& curCode : curriPeriod) {
          std::vector<string>::iterator findCurriculum1 =
            std::find(prevcurriPeriod.begin(), prevcurriPeriod.end(), curCode);
          if (findCurriculum1 == prevcurriPeriod.end()) {
            consecutiveLectureViolations++;
          }
        }

      } else if (j != endPeriod && j != startPeriod) {
        // not first or last period
        vector<CourseRoomTime> periodsubsetTable =
          this->getCRTsinPeriod(j, daysubsetTable);
        vector<CourseRoomTime> nextperiodsubsetTable =
          this->getCRTsinPeriod(j + 1, daysubsetTable);
        vector<CourseRoomTime> prevperiodsubsetTable =
          this->getCRTsinPeriod(j - 1, daysubsetTable);
        vector<string> curriPeriod =
          this->curriculumInTimetable(periodsubsetTable);
        vector<string> nextcurriPeriod =
          this->curriculumInTimetable(nextperiodsubsetTable);
        vector<string> prevcurriPeriod =
          this->curriculumInTimetable(prevperiodsubsetTable);

        for (auto& curCode : curriPeriod) {
          std::vector<string>::iterator findCurriculum =
            std::find(nextcurriPeriod.begin(), nextcurriPeriod.end(), curCode);
          std::vector<string>::iterator findCurriculum1 =
            std::find(prevcurriPeriod.begin(), prevcurriPeriod.end(), curCode);
          if (findCurriculum == nextcurriPeriod.end() &&
              findCurriculum1 == prevcurriPeriod.end()) {
            consecutiveLectureViolations++;
          }
        }
      }
    }
  }

  // finding number of minimum course working violations and room stability
  // violations
  int index = 0;
  for (auto& c : this->course) {
    int minWork = c.getMinWorkingDays();
    if (minWork > courseWorkingDays[index]) {
      int val = minWork - courseWorkingDays[index];
      courseWorkingDayViolations += val;
    }

    // cout<<"Room Stability "<<c.getCode()<<":
    // "<<courseRooms[index].size()<<"\t Minimum Working Days:
    // "<<c.getMinWorkingDays()<<" Table working days:
    // "<<courseWorkingDays[index]<<endl;
    int v = courseRooms[index].size() - 1;
    roomStabilityViolations += v;
    index++;
  }

  // cout<<"Room Stability violations: "<<roomStabilityViolations << " Course
  // working days violations: " <<courseWorkingDayViolations << " consecutive
  // lectures violations " <<consecutiveLectureViolations <<endl;

  *rsv = roomStabilityViolations;
  *cwdv = courseWorkingDayViolations;
  *clv = consecutiveLectureViolations;
  *sv = sizeViolations;
  return (sizeViolations + roomStabilityViolations +
          (5 * courseWorkingDayViolations) +
          (2 * consecutiveLectureViolations));
}

/**
 *
 * @param period
 * @return
 * Get the week day given the period in the time table
 */

int
ImproveTable::getDay(int period)
{
  int day = period / this->periods_in_day;
  return day;
}

/**
 *
 * @param period
 * @return
 * Get the week day given the period in the time table
 */

int
ImproveTable::getPeriodValue(int period, int day)
{
  return (day * this->periods_in_day) + period;
}

/**
 * Gets the subset of the given timetable that occur on the specified day
 * @param day
 * @param timetable
 * @return ArrayList of CourseRoomTime objects from given timetable in the
 * Specified day
 */

vector<CourseRoomTime>
ImproveTable::getCRTsinDay(const int& day,
                           const vector<CourseRoomTime>& timetable)
{
  vector<CourseRoomTime> crtInDay;

  for (auto& t : timetable) {
    CourseRoomTime tempCRT = t;
    if (tempCRT.getCourse().isEmpty() == false) {
      if (tempCRT.isDay(this->periods_in_day, day))
        crtInDay.push_back(tempCRT);
    }
  }
  return crtInDay;
}

/**
 * Gets the curricula in a timetable
 * @param timetable - Ctimetable - ArrayList of CourseRoomTime Objects
 * @return -  vector of curricula in the timetable
 */

vector<string>
ImproveTable::curriculumInTimetable(const vector<CourseRoomTime>& timetable)
{
  vector<string> curri;
  for (auto& t : timetable) // loop through vector c++11 way
  {
    CourseRoomTime crt = t;
    if (crt.getCourse().isEmpty() == false) {
      vector<string> curricula = crt.getCourse().getCurriculaCodes();
      vector<string>::iterator curIterator = curricula.begin();
      while (curIterator != curricula.end()) {
        string curCode = *curIterator;
        vector<string>::iterator findCurCode =
          std::find(curri.begin(), curri.end(), curCode);
        if (findCurCode == curri.end())
          curri.push_back(curCode);
        curIterator++;
      }
    }
  }
  return curri;
}

/**
 * Gets the courses in a timetable
 * @param timetable - Ctimetable - ArrayList of CourseRoomTime Objects
 * @return -  vector of courses in the timetable
 */

vector<Course>
ImproveTable::coursesInTimetable(const vector<CourseRoomTime>& timetable)
{
  vector<Course> curri;
  for (auto& t : timetable) // loop through vector c++11 way
  {
    CourseRoomTime crt = t;
    if (crt.getCourse().isEmpty() == false) {
      Course c = crt.getCourse();
      vector<Course>::iterator findCourse =
        std::find(curri.begin(), curri.end(), c);
      if (findCourse == curri.end())
        curri.push_back(c);
    }
  }
  return curri;
}

/**
 * Gets the number of courseroomtime objects in the last time slot
 * @param timetable
 * @return number of curricula in the last period
 */

int
ImproveTable::numberCurriculumLastTimeSlot(
  const vector<CourseRoomTime>& timetable)
{
  vector<CourseRoomTime> lastP;
  for (auto& t : timetable) {
    CourseRoomTime cTime = t;
    if (cTime.getCourse().isEmpty() == false) {
      if (cTime.isLastPeriod(this->periods_in_day) == true) {
        lastP.push_back(cTime);
      }
    }
  }
  // cout<<lastP.size()<<"CRTs in last"<<endl;
  return this->curriculumInTimetable(lastP).size();
}

/**
 * Gets the subset of given timetable that occur on the specified period
 * @param period
 * @param timetable
 * @return ArrayList of CourseRoomTime objects from the timetable in the
 * Specified period
 */

vector<CourseRoomTime>
ImproveTable::getCRTsinPeriod(const int& period,
                              const vector<CourseRoomTime>& timetable)
{
  vector<CourseRoomTime> crtinPeriod;
  for (auto& t : timetable) {
    CourseRoomTime tempCRT = t;
    if (tempCRT.getCourse().isEmpty() == false) {
      if (tempCRT.getVenueTime().getPeriod() == period)
        crtinPeriod.push_back(tempCRT);
    }
  }
  return crtinPeriod;
}

vector<CourseRoomTime>
ImproveTable::getKempeChain(const vector<CourseRoomTime>& timetable,
                            const vector<CourseRoomTime>& swapList,
                            const int& p1, const int& p2)
{
  vector<CourseRoomTime> crtinPeriodOne;
  vector<CourseRoomTime> crtinPeriodTwo;
  vector<CourseRoomTime> kempeChain;
  auto merged =
    getCRTsinGivenPeriods(p1, p2, timetable, &crtinPeriodOne, &crtinPeriodTwo);
  auto barpatiteMatrix =
    getBarpatiteMatrix(merged, crtinPeriodOne, crtinPeriodTwo);

  // create kempe chain
  queue<CourseRoomTime> toVisit;
  for (auto& crt : swapList) {
    toVisit.push(crt);
    kempeChain.push_back(crt);
  }
  while (!toVisit.empty()) {
    auto current = toVisit.front();
    // and adjacent nodes to kempe chain if not already exists
    auto i = this->getIndex(current, merged);
    for (size_t j = 0; j < merged.size(); j++) {
      if (barpatiteMatrix[i][j] == 1) {
        std::vector<CourseRoomTime>::iterator findCRT =
          std::find(kempeChain.begin(), kempeChain.end(), merged[j]);
        if (findCRT == kempeChain.end()) {
          kempeChain.push_back(merged[j]);
          toVisit.push(merged[j]);
        }
      }
    }
    toVisit.pop();
  }
  return kempeChain;
}

/**
 * Gets the subset of given timetable that occur on the specified periods
 * @param period
 * @param timetable
 * @return ArrayList of CourseRoomTime objects from the timetable in the
 * Specified period
 */

vector<CourseRoomTime>
ImproveTable::getCRTsinGivenPeriods(const int& p1, const int& p2,
                                    const vector<CourseRoomTime>& timetable,
                                    vector<CourseRoomTime>* periodOne,
                                    vector<CourseRoomTime>* periodTwo)
{
  vector<CourseRoomTime> crtinPeriodOne;
  vector<CourseRoomTime> crtinPeriodTwo;
  vector<CourseRoomTime> merged;
  for (auto& t : timetable) {
    CourseRoomTime tempCRT = t;
    if (tempCRT.getCourse().isEmpty() == false) {
      if (tempCRT.getVenueTime().getPeriod() == p1) {
        merged.push_back(tempCRT);
        crtinPeriodOne.push_back(tempCRT);
      }
      if (tempCRT.getVenueTime().getPeriod() == p2) {
        merged.push_back(tempCRT);
        crtinPeriodTwo.push_back(tempCRT);
      }
    }
  }
  *periodOne = crtinPeriodOne;
  *periodTwo = crtinPeriodTwo;
  return merged;
}

vector<vector<int>>
ImproveTable::getBarpatiteMatrix(vector<CourseRoomTime> merged,
                                 vector<CourseRoomTime> p1,
                                 vector<CourseRoomTime> p2)
{
  vector<vector<int>> barpatiteMatrix;
  barpatiteMatrix.resize(merged.size());
  for (std::size_t i = 0; i < merged.size(); i++) {
    barpatiteMatrix[i].resize(merged.size());
  }
  for (auto& p1CRT : p1) {
    auto i = this->getIndex(p1CRT, merged);
    for (auto& p2CRT : p2) {
      auto j = this->getIndex(p2CRT, merged);
      // check of course has common students or lecturers
      bool hasCommon = this->hasCommon(p1CRT.getCourse(), p2CRT.getCourse());
      if (hasCommon == true) {
        barpatiteMatrix[i][j] = 1;
        barpatiteMatrix[j][i] = 1;
      }
    }
  }
  return barpatiteMatrix;
}

vector<Venue>
ImproveTable::getTakenRooms(int period, vector<CourseRoomTime> timetable)
{
  vector<Venue> taken;
  for (auto& crt : timetable) {
    auto vt = crt.getVenueTime();
    auto room = vt.getRoom();
    int p = vt.getPeriod();
    if (p == period)
      taken.push_back(room);
  }
  return taken;
}

vector<Venue>
ImproveTable::getAvailableRooms(int period, vector<CourseRoomTime> timetable,
                                vector<CourseRoomTime> kempeChain)
{
  vector<Venue> takenAll = getTakenRooms(period, timetable);
  vector<Venue> takenKempe = getTakenRooms(period, kempeChain);
  vector<Venue> available = this->rooms;
  // remove all taken from rooms
  for (auto& r : takenAll) {
    available.erase(std::remove(available.begin(), available.end(), r),
                    available.end());
  }

  // add those rooms that will be swapped in kempe chain
  for (auto& r : takenKempe) {
    available.push_back(r);
  }

  return available;
}

vector<CourseRoomTime>
ImproveTable::singleKempeSwap(vector<Venue>* availableRooms, CourseRoomTime crt,
                              int newPeriod,
                              std::vector<CourseRoomTime> timetable)
{
  int index = this->getIndex(crt, timetable);
  auto newT = timetable;
  vector<Venue> ar = *availableRooms;
  Course c = crt.getCourse();
  int cSize = c.getNumStudents();
  Venue v = crt.getVenueTime().getRoom();
  std::vector<Venue>::iterator findRoom = std::find(ar.begin(), ar.end(), v);
  // cout<<"Old CRT: "<<crt.toString()<<endl;

  // if room not already taken
  if (findRoom != ar.end()) {
    auto newVT = crt.getVenueTime();
    newVT.setRoom(v);
    newVT.setPeriod(newPeriod);
    newT[index].setVenueTime(newVT);
    ar.erase(std::remove(ar.begin(), ar.end(), v), ar.end());
    // cout<<"New CRT: "<<newT[index].toString()<<endl;
    *availableRooms = ar;
    return newT;
  } else if (findRoom == ar.end() && ar.size() > 0) {
    // get best available room
    auto best = ar[0];
    int bDifference = ar[0].getCapacity() - cSize;
    for (size_t i = 1; i < ar.size(); i++) {
      int difference = ar[0].getCapacity() - cSize;
      if (difference < bDifference) {
        bDifference = difference;
        best = ar[i];
      }
    }
    auto newVT = crt.getVenueTime();
    newVT.setRoom(best);
    newVT.setPeriod(newPeriod);
    newT[index].setVenueTime(newVT);
    ar.erase(std::remove(ar.begin(), ar.end(), best), ar.end());
    *availableRooms = ar;
    // cout<<"New CRT: "<<newT[index].toString()<<endl;
    return newT;
  }

  // cout<<"Room violation exists "<< ar.size()<<endl;
  return timetable;
}

std::vector<CourseRoomTime>
ImproveTable::kempeChainSwap(vector<CourseRoomTime> kempeChain,
                             vector<CourseRoomTime> timetable, int p1, int p2)
{
  vector<Venue> availP1 = getAvailableRooms(p1, timetable, kempeChain);
  vector<Venue> availP2 = getAvailableRooms(p2, timetable, kempeChain);
  auto tempSchedule = timetable;
  vector<CourseRoomTime> newSchedule;
  for (auto& crt : kempeChain) {
    int cPeriod = crt.getVenueTime().getPeriod();
    if (cPeriod == p1) {
      newSchedule = singleKempeSwap(&availP2, crt, p2, tempSchedule);
      tempSchedule = newSchedule;
    }
    if (cPeriod == p2) {
      newSchedule = singleKempeSwap(&availP1, crt, p1, tempSchedule);
      tempSchedule = newSchedule;
    }
  }

  int rowViolations = 0;
  int colViolations = 0;
  SM::periodViolation(tempSchedule, this->maxPeriod, p1, &rowViolations);
  SM::periodViolation(tempSchedule, this->maxPeriod, p2, &colViolations);

  if (rowViolations == 0 && colViolations == 0) {
    // cout<<" No violation"<<endl;
    return tempSchedule;
  } else {
    // cout<<" A violation exists"<<endl;
    return timetable;
  }
}

bool
ImproveTable::hasCommon(Course c1, Course c2)
{
  // lecturer clash
  if (c1.getLecturer() == c2.getLecturer())
    return true;

  // student clash
  auto c1Curriculum = c1.getCurriculaCodes();
  auto c2Curriculum = c2.getCurriculaCodes();
  for (auto& curr : c1Curriculum) {
    std::vector<string>::iterator findCurriculum =
      std::find(c2Curriculum.begin(), c2Curriculum.end(), curr);
    if (findCurriculum != c2Curriculum.end())
      return true;
  }

  return false;
}

void
ImproveTable::writeTimetableToFile(string name)
{
  ofstream myfile(name);
  if (myfile.is_open()) {
    int lastDay = this->getDay(this->maxPeriod - 1);
    // cout<<"last day is: "<< lastDay << "  "<<this->maxPeriod;
    for (int i = 0; i <= lastDay; i++) {
      // cout<<" Day "<<i;
      vector<CourseRoomTime> daysubsetTable = this->getCRTsinDay(
        i, this->fullSchedule); // course room time elements in day i

      int startPeriod = i * this->periods_in_day; // get first period in day i
      int endPeriod =
        startPeriod + this->periods_in_day - 1; // get last period in day i
      for (int j = startPeriod; j <= endPeriod; j++) {
        // cout<<"Period "<<j<<endl;
        vector<CourseRoomTime> periodsubsetTable =
          this->getCRTsinPeriod(j, daysubsetTable);
        int pe = j % this->periods_in_day;
        myfile << "[Day " << i << " Period " << pe << "] ";
        for (auto& crt : periodsubsetTable) {
          string courseCode = crt.getCourse().getCode();
          string roomCode = crt.getVenueTime().getRoom().getName();
          myfile << courseCode << "-" << roomCode << "-"
                 << crt.getVenueTime().getPeriod() << "----";
        }
        myfile << "\t";
      }
      myfile << endl;
    }
    myfile.close();
  } else
    cout << "Unable to open file";
}

void
ImproveTable::writeTimetableToFile2()
{
  ofstream myfile("FeasibleTable.csv");
  if (myfile.is_open()) {
    for (auto& c : this->course) {
      if (c.isEmpty() == false) {
        myfile << c.getCode() << "\t" << c.getWeeklyLectures() << "\t";
        auto crt = this->getCoursePeriods(c);
        for (auto& crtt : crt) {
          myfile << crtt.getVenueTime().toString() << "\t";
        }
        myfile << endl;
      }
    }

    myfile.close();
  } else
    cout << "Unable to open file";
}

void
ImproveTable::writeTimetableToFileSolutionFormat(
  string solutionName, vector<CourseRoomTime> timetable)
{
  ofstream myfile(solutionName);
  if (myfile.is_open()) {
    for (auto& c : timetable) {
      if (c.getCourse().isEmpty() == false) {
        string course = c.getCourse().getCode();
        string room = c.getVenueTime().getRoom().getName();
        int value = c.getVenueTime().getPeriod();
        int day = this->getDay(value);
        int period = value % this->periods_in_day;
        myfile << course << " " << room << " " << day << " " << period << endl;
      }
    }

    myfile.close();
  } else
    cout << "Unable to open file";
}

vector<CourseRoomTime>
ImproveTable::getCoursePeriods(const Course& course)
{
  vector<CourseRoomTime> crt;
  for (auto c : this->fullSchedule) {
    auto tCourse = c.getCourse();
    if (tCourse == course && tCourse.isEmpty() == false)
      crt.push_back(c);
  }
  return crt;
}

int
ImproveTable::getIndex(CourseRoomTime crt, vector<CourseRoomTime> timetable)
{
  std::vector<CourseRoomTime>::iterator findCRT =
    std::find(timetable.begin(), timetable.end(), crt);
  int index = std::distance(timetable.begin(), findCRT);

  return index;
}

vector<CourseRoomTime>
ImproveTable::removeCRTinPeriod(const int& period,
                                vector<CourseRoomTime> timetable)
{
  vector<CourseRoomTime> filteredPeriod;
  vector<CourseRoomTime> periodCRTs = this->getCRTsinPeriod(period, timetable);
  // cout<<periodCRTs.size()<<" Period Size "<<period<<endl;
  // cout<<timetable.size()<<" timetable Size "<<period<<endl;

  for (auto& c : timetable) {
    if (c.getCourse().isEmpty() == false) {
      auto cc = c;
      std::vector<CourseRoomTime>::iterator findCRT =
        std::find(periodCRTs.begin(), periodCRTs.end(), c);
      if (findCRT == periodCRTs.end())
        filteredPeriod.push_back(cc);
    }
  }

  // cout<<filteredPeriod.size()<<" Filtered Size "<<endl;
  return filteredPeriod;
}

void
ImproveTable::softWalk(const vector<CourseRoomTime>& timetable,
                       const int& antIndex, vector<CourseRoomTime>* newSchedule,
                       int* num_scv, vector<int>* newPath, bool accept)
{
  // cout<<"Ant Walking"<<endl;
  vector<CourseRoomTime> schedule = timetable;
  vector<int> path;
  CourseRoomTime tempCRT;

  auto copySchedule = timetable;

  for (std::size_t i = 0; i < timetable.size(); i++) {
    int rowIndex = i;
    vector<CourseRoomTime> newSchedule;
    auto selectP =
      this->selectNextCRT(schedule, rowIndex, antIndex, &newSchedule);
    schedule = newSchedule;
    path.push_back(selectP);
  }

  // cout<<" The length of path is "<<path.size()<<" The length of schedule is
  // "<<schedule.size()<<endl;
  // schedule = this->applyPath(&path, timetable);
  *newSchedule = schedule;
  *num_scv = this->NumberSCV(schedule);
  *newPath = path;
}

vector<CourseRoomTime>
ImproveTable::getClashList(const vector<CourseRoomTime>& timetable,
                           const CourseRoomTime& crt)
{
  vector<CourseRoomTime> clashList;
  int period = crt.getVenueTime().getPeriod();
  auto crtInP = this->getCRTsinPeriod(period, timetable);
  auto crtRoom = crt.getVenueTime().getRoom();
  auto crtCurricula = crt.getCourse().getCurriculaCodes();
  auto crtLecturer = crt.getCourse().getLecturer();
  for (auto& cpn : crtInP) {
    auto cp = cpn;
    bool l = true;
    int number = 0;
    if (cp == crt) {
      l = false;
      number++;
    }
    if (cp == crt && number > 1) {
      clashList.push_back(crt);
    }
    if (l) {
      // check room
      auto cpRoom = cp.getVenueTime().getRoom();
      if (cpRoom == crtRoom)
        clashList.push_back(cp);
      // check lecturer
      auto cpLecturer = cp.getCourse().getLecturer();
      if (cpLecturer == crtLecturer)
        clashList.push_back(cp);
      // check curriculum clash
      auto cpCurricula = cp.getCourse().getCurriculaCodes();
      for (auto& cpCurC : cpCurricula) {
        std::vector<string>::iterator findCurr =
          std::find(crtCurricula.begin(), crtCurricula.end(), cpCurC);
        if (findCurr != crtCurricula.end())
          clashList.push_back(cp);
      }
    }
  }

  return clashList;
}

int
ImproveTable::selectNextCRT(vector<CourseRoomTime> timetable,
                            const int& rowIndex, const int& antIndex,
                            vector<CourseRoomTime>* newSched)
{
  vector<double> probability(timetable.size());
  // TODO Implement heuristics here
  // vector<double> newHeuristics;

  // for (std::size_t i =0; i < functions.size(); i++)
  for (std::size_t i = 0; i < probability.size(); i++) {
    int colIndex = i;
    double trail = this->adjacentMatrix[rowIndex][colIndex];
    double trailFactor = pow(trail, this->beta);
    double heuristicFactor = pow(1, this->alpha); // heuristics defaults to one
    probability[i] = (trailFactor + heuristicFactor);
  }

  int aIndex = antIndex;
  // cout<<"Probability Size "<<functions.size()<<" "<<aIndex<<endl;
  int retIndex = 0;
  auto retCRT =
    this->selectNeighbourProbability(probability, timetable, aIndex, &retIndex);
  // auto retCRT = this->selectNeighbourProbability(probability, functions,
  // aIndex, &retIndex);
  *newSched = this->bestNeighbour(retIndex, timetable);
  return retIndex;
}

CourseRoomTime
ImproveTable::selectNeighbourProbability(
  vector<double> probability, const vector<CourseRoomTime>& functions,
  const int& antIndex, int* returnIndex)
{
  // cout<<"Here"<<endl;
  // cout<<rt.size()<<"\t";
  int length = functions.size();
  // cout<<rt.size();
  double sum = 0;
  for (int i = 0; i < length; i++) {
    sum += probability[i];
    // cout<<probability[i]<<"\t"<<sum<<"---";
  }
  // cout<<endl;

  for (int i = 0; i < length; i++) {
    probability[i] /= sum;
    // cout<<probability[i]<<"("<<functions[i]<<"), ";
  }
  // cout<<endl;

  // int unsigned seed = time(NULL) * antIndex;
  // this->index++;
  // double number = rand_r(&seed)/(double)RAND_MAX;
  // cout<<seed<<" "<<number<<endl;

  mt19937::result_type seed =
    std::chrono::system_clock::now().time_since_epoch().count() + antIndex;

  auto dice_randa =
    std::bind(std::uniform_real_distribution<double>(0, 1), mt19937(seed));
  double number = dice_randa();
  // cout<<number<<"---";

  double cumulativeProbability = 0;
  for (int i = 0; i < length; i++) {
    cumulativeProbability += probability[i];
    // cout<<cumulativeProbability<<", ";
    if (number <= cumulativeProbability) {
      // cout<<endl;
      *returnIndex = i;
      return functions[i];
    }
  }

  cout << "Error in cumulative Probability" << endl;

  return functions[0]; // should not get to this point
}

vector<CourseRoomTime>
ImproveTable::antColonySoft(const int& numberAnts, const int& numberCycles,
                            const int& num_ber, bool accept)
{
  auto schedule = this->fullSchedule;
  // this->global = schedule;
  vector<CourseRoomTime> global = schedule;

  // int walkCurrent = 0;

  int gBest = this->globalBest;
  cout << "Global best is: " << this->globalBest << endl;
  // cout<<"Size of timetable "<<this->fullSchedule.size()<<endl;

  // System.out.println("Selecting " + number + " random event pairs");
  for (int i = 0; i < numberCycles; i++) {
    // System.out.println("Cycle: " + i);
    // walkCurrent++;
    // auto current = schedule;
    int cycleBest = 50000;
    vector<CourseRoomTime> cb;

    vector<int> cyclePath;
    // vector<RoomPath> cycleRoomPath;
    // cout<<" SCV Before: "<<this->NumberSCV(schedule);

    for (int j = 0; j < numberAnts; j++) {
      // double wall0 =  SM::get_wall_time();
      // double cpu0  = SM::get_cpu_time();

      vector<CourseRoomTime> newSchedule(this->fullSchedule.size());
      int num_scv;
      vector<int> cp;
      // cout<<"Going to soft walk"<<endl;

      this->softWalk(schedule, (j + 1), &newSchedule, &num_scv, &cp, accept);
      // cout<<"Finished soft walk"<<endl;
      // cout<<" SCV After: "<<this->NumberSCV(schedule)<<endl;

      // cout<<"size of cp is: "<<cp.size()<<endl;
      // auto anewSchedule = this->applyPath(&cp, this->fullSchedule);

      // int num_hcv = SM::NumberHCV(newSchedule, this->maxPeriod);
      if (num_scv < cycleBest) {
        cycleBest = num_scv;
        cyclePath = cp;
        // cycleRoomPath = crPath;
        cb = newSchedule;
      }
      // double wall1 = SM::get_wall_time();
      // double cpu1 = SM::get_cpu_time();
      // std::cout <<endl<< "Complete Ant walk in  " <<  wall1 - wall0 <<" wall
      // seconds and "<<  cpu1 - cpu0 <<" CPU seconds"<<endl;
      cout << "Cycle: " << i << " Ant: " << j
           << " Num SCV violations: " << num_scv << endl;
    }
    if (gBest == 0) {
      this->fullSchedule = global;
      return global;
    }
    cout << i << ": Cycle Best is: " << cycleBest
         << " Global Best is:  " << gBest << endl;
    int roomStabilityViolations = 0;
    int courseWorkingDayViolations = 0;
    int consecutiveLectureViolations = 0;
    int sizeViolations = 0;
    // auto temp = this->voteOutInviteIn(1, cb);
    auto temp = cb;
    int s = this->NumberSCV(temp, &roomStabilityViolations,
                            &courseWorkingDayViolations,
                            &consecutiveLectureViolations, &sizeViolations);
    cout << "\t\t\tLocal Search Total SCV: " << s
         << " Room Stability: " << roomStabilityViolations
         << " Course Working Day Violations: " << courseWorkingDayViolations
         << " Consecutive Lecture Violations: " << consecutiveLectureViolations
         << " Room Size Violations: " << sizeViolations << endl;
    // cout<< " Best So far "<< this->bestSoFar <<endl;

    this->updateAntTrail(cycleBest, cyclePath);

    if (cycleBest < gBest) {
      gBest = cycleBest;
      this->globalBest = gBest;
      global = cb;
    }
  }
  this->fullSchedule = global;

  return this->removeEmptyCourses(global);
}

void
ImproveTable::evapourate(int j)
{
  double evapourate = 1 - this->rho;
  int row = this->fullSchedule.size();
  for (int i = 0; i < row; i++) {
    adjacentMatrix[i][j] *= evapourate;
  }
}

void
ImproveTable::updateAntTrail(const int& numSCV_cycleBest,
                             const vector<int>& path)
{
  /*	//Positive and negative rewards
          double reward = (1 / (1 + (double)numSCV_cycleBest -
     (double)this->globalBest )) - 1;
          reward = reward/10;

          if (this->globalBest > numSCV_cycleBest)
                  reward = 1 - (1 / (1 + (double)this->globalBest  -
     (double)numSCV_cycleBest));*/

  // only positive rewards
  double reward = 1 / (1 + (double)numSCV_cycleBest - (double)this->globalBest);

  if (this->globalBest > numSCV_cycleBest)
    reward = 1;

  cout << "\t\t\tReward is: " << reward << endl;

  // evapourate adjacent matrix
  for (std::size_t j = 0; j < this->fullSchedule.size(); j++)
    this->evapourate(j);

  // cout<<"\t\t\tReward is: "<<reward<<endl;

  int length = fullSchedule.size();
  for (int row = 0; row < length; row++) {
  	int col = path[row];
    this->adjacentMatrix[row][col] += reward;
    if (this->adjacentMatrix[row][col] > this->t_max) {
      this->adjacentMatrix[row][col] = this->t_max;
    }
    if (this->adjacentMatrix[row][col] < this->t_min) {
      this->adjacentMatrix[row][col] = this->t_min;
    }
  }
}

void
ImproveTable::initializeMatrix()
{
  for (std::size_t j = 0; j < this->fullSchedule.size(); j++) {
    // th.push_back(std::thread(&ImproveTable::initPeriod, this, j));
    this->initPeriod(j);
  }
}

void
ImproveTable::initPeriod(const int& crtIndex)
{
  // int i = 0;
  // for (auto &colCRT:this->fullSchedule)
  for (std::size_t i = 0; i < this->fullSchedule.size(); i++) {
    /*		auto rowCRT = this->fullSchedule[crtIndex];
                    auto rowCourse = rowCRT.getCourse();
                    auto rowRoomConstraints = rowCourse.getRoomConstraint();
                    auto rowPeriodConstraints =
       rowCourse.getUnavailableConstraint();
                    auto colRoom = colCRT.getVenueTime().getRoom();
                    auto colPeriod = colCRT.getVenueTime().getPeriod();
                    std::vector<string>::iterator findRoom =
       std::find(rowRoomConstraints.begin(), rowRoomConstraints.end(), colRoom);
                    std::vector<int>::iterator findPeriod =
       std::find(rowPeriodConstraints.begin(), rowPeriodConstraints.end(),
       colPeriod);
                    if (findPeriod == rowPeriodConstraints.end() && findRoom !=
       rowRoomConstraints.end()) //room size fits and not in unavailable
                            adjacentMatrix[crtIndex][i] = t_max;
                    else
                            adjacentMatrix[crtIndex][i] = 0;*/
    adjacentMatrix[crtIndex][i] = t_max;
    // i++;
  }
}

vector<CourseRoomTime>
ImproveTable::antColonySoftThread(const int& numberAnts,
                                  const int& numberCycles, const int& num_ber,
                                  bool accept)
{
  auto schedule = this->fullSchedule;
  vector<CourseRoomTime> global = schedule;
  // cout<<"Global best is: "<<this->globalBest <<endl;
  // cout<<"Size of timetable "<<this->fullSchedule.size()<<endl;

  // int walkCurrent = 0;

  // System.out.println("Selecting " + number + " random event pairs");
  for (int i = 0; i < numberCycles; i++) {
    // System.out.println("Cycle: " + i);
    // walkCurrent++;
    // auto current = schedule;
    // int cycleBest = 50000;
    vector<CourseRoomTime> cb(this->fullSchedule.size());

    // double wall0 =  SM::get_wall_time();
    // double cpu0  = SM::get_cpu_time();
    vector<int>* cyclePath = new vector<int>[numberAnts];
    vector<CourseRoomTime>* antWalk = new vector<CourseRoomTime>[numberAnts];

    int n_scv[numberAnts]; // create vector of number of constraint violations

    std::vector<std::thread> th;
    for (int j = 0; j < numberAnts;
         j++) // create threads of number of ants to run in parallel
    {
      // th.push_back(std::thread(&ImproveTable::softWalk, this, &antWalk[j],
      // ((i+1)*(j+1)), &n_scv[j], &cyclePath[j]));
      th.push_back(std::thread(&ImproveTable::softWalk, this, schedule,
                               ((i + 1) * (j + 1)), &antWalk[j], &n_scv[j],
                               &cyclePath[j], accept));
    }

    for (auto& t : th) // join all ants threads
    {
      t.join();
    }
    // double wall1 = SM::get_wall_time();
    // double cpu1 = SM::get_cpu_time();
    // std::cout <<endl<< "ant walked in " <<  wall1 - wall0 <<" wall seconds
    // and "<<  cpu1 - cpu0 <<" CPU seconds"<<endl;

    // find cycle best;
    int cycleb = n_scv[0];
    int cycleBestIndex = 0;
    vector<int> pathBest = cyclePath[0];
    for (int k = 0; k < numberAnts; k++) {
      // int num_hcv = this->NumberHCV(antWalk[k]);
      if (n_scv[k] < cycleb) {
        cycleb = n_scv[k];
        cycleBestIndex = k;
        pathBest = cyclePath[k];
      }
      cout << "Cycle: " << i << " Ant: " << k
           << " Num SCV violations: " << n_scv[k] << endl;
      if (n_scv[k] == 0) {
        this->globalBest = n_scv[k];
        global = antWalk[k];
        cout << "Found Global Best is: " << this->globalBest << " in cycle "
             << i << " Size: " << global.size() << endl;
        delete[] antWalk;
        delete[] cyclePath;
        this->fullSchedule = global;
        return global;
      }
    }
    cout << "\t\t\t" << i << ":  Cycle Best is: " << cycleb
         << " Global Best is:  " << this->globalBest
         << " Size: " << global.size() << endl;

    int roomStabilityViolations = 0;
    int courseWorkingDayViolations = 0;
    int consecutiveLectureViolations = 0;
    int sizeViolations = 0;
    auto temp = antWalk[cycleBestIndex];
    int s = this->NumberSCV(temp, &roomStabilityViolations,
                            &courseWorkingDayViolations,
                            &consecutiveLectureViolations, &sizeViolations);
    cout << "\t\t\tLocal Search Total SCV: " << s
         << " Room Stability: " << roomStabilityViolations
         << " Course Working Day Violations: " << courseWorkingDayViolations
         << " Consecutive Lecture Violations: " << consecutiveLectureViolations
         << " Room Size Violations: " << sizeViolations << endl;
    // cout<< " Best So far "<< this->bestSoFar <<endl;

    this->updateAntTrail(cycleb, pathBest);

    if (cycleb < this->globalBest) {
      this->globalBest = cycleb;
      // this->fullSchedule = temp;
      global = temp;
    }
    delete[] antWalk;
    delete[] cyclePath;
  }

  return this->fullSchedule = global;
}

vector<CourseRoomTime>
ImproveTable::removeEmptyCourses(const vector<CourseRoomTime>& timetable)
{
  vector<CourseRoomTime> newTable;
  for (auto& crt : timetable) {
    if (crt.getCourse().isEmpty() == false)
      newTable.push_back(crt);
  }
  return newTable;
}

vector<CourseRoomTime>
ImproveTable::runImprovement(int numIterations, int numAnts, int numCycles)
{
  // cout<<"running Impromement"<<endl;
  vector<CourseRoomTime> iteBest = this->fullSchedule;
  vector<CourseRoomTime> iterationSchedule = this->fullSchedule;
  bool accept = false;
  auto minimum = this->NumberSCV(iteBest);
  if (minimum == 0)
    return iteBest;
  int i = 0;
  int consecutive = 0;
  // for (int i =0; i < numIterations; i++)
  while (i < numIterations && consecutive < 4) {
    cout << "Iteration: " << i << "  " << consecutive << endl;
    this->initializeMatrix();
    // auto e = this->antColonySoft(numAnts, numCycles, i);
    if (accept == true)
      cout << "accept is true" << endl;
    else
      cout << "accept is false" << endl;

    auto pastSCV = this->NumberSCV(iterationSchedule);
    auto e =
      this->antColonySoftThread(numAnts, numCycles, numIterations, false);
    writeTimetableToFileSolutionFormat("tempResult.sol", e);
    iterationSchedule = e;
    auto currentSCV = this->NumberSCV(e);
    cout << currentSCV << " Current " << pastSCV << " Past" << endl;
    if (pastSCV <= currentSCV) {
      consecutive++;
    } else {
      consecutive = 0;
    }

    auto iterationSCV = currentSCV;
    if (minimum > iterationSCV) {
      minimum = iterationSCV;
      iteBest = e;
    }
    if (minimum == 0) {
      return iteBest;
    }
    if (consecutive == 2)
      accept = true;
    i++;
  }
  return this->removeEmptyCourses(iteBest);
}

int
getRoomStabilityViolations(vector<CourseRoomTime> timetable)
{
  return 0;
}

CourseRoomTime
ImproveTable::moveToBest2(int crtIndex, vector<CourseRoomTime>* timetable)
{
  // cout<<" in move to best 2"<<endl;
  CourseRoomTime c;
  auto t = *timetable;
  CourseRoomTime crt = t[crtIndex];
  auto tempSchedule = *timetable;
  int found = 0;
  unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
  double number = rand_r(&seed) / (double)RAND_MAX;

  std::vector<CourseRoomTime>::iterator findCRT =
    std::find(tempSchedule.begin(), tempSchedule.end(), crt);
  int indexInTS = std::distance(tempSchedule.begin(), findCRT);
  CourseRoomTime crT = *findCRT;
  int violations = -1;
  int v2 = -1;

  if (findCRT != tempSchedule.end() && crT.getCourse().isEmpty() == false) {
    // int min = this->NumberSCV(tempSchedule);
    int min = RAND_MAX;
    auto bestSchedule = tempSchedule;
    auto newTemp = tempSchedule;
    for (std::size_t i = 0; i < this->venueTime.size(); i++) {
      auto eventP = this->venueTime[i];
      if (eventP.canAssignCourse(crt.getCourse())) {
        newTemp = tempSchedule;
        newTemp[indexInTS].setVenueTime(eventP);
        // check number of hard constraint violations in newTimetable CRT

        SM::periodViolation(newTemp, this->maxPeriod, eventP.getPeriod(),
                            &violations);

        SM::periodViolation(newTemp, this->maxPeriod,
                            crt.getVenueTime().getPeriod(), &v2);
        // cout<<eventP.getPeriod()<<" Period "<<violations<<endl;
        // cout<<SM::NumberHCV(newTemp, this->maxPeriod)<<" Number HCV"<<endl;
        // cout<<" Violations: "<<eventP.getPeriod()<<" "<<violations<<"
        // "<<crt.getVenueTime().getPeriod()<<" "<<v2<<"
        // "<<crt.toString()<<endl;
        if (violations == 0 && v2 == 0) {
          // int x = -1;
          // SM::periodViolation(newTemp, this->maxPeriod, eventP.getPeriod(),
          // &x);
          // cout<<" Period Violations "<<x<<endl;
          // cout<<endl<<eventP.getPeriod()<<" "<<violations<<"
          // "<<SM::ThreadNumberHCV(newTemp, this->maxPeriod)<<" Number HCV
          // "<<crt.toString()<<" "<<newTemp[indexInTS].toString()<<endl;
          int nSCurrent = this->NumberSCV(newTemp);
          if (nSCurrent < min) {
            min = nSCurrent;
            bestSchedule = newTemp;
          }
          number = rand_r(&seed) / (double)RAND_MAX;
          if (nSCurrent == min && number > 0.5) {
            // cout<<" best is swap "<<endl;
            min = nSCurrent;
            bestSchedule = newTemp;
          }
          found++;

        } else if (violations > 0) {
          // second CRT return Old
          // return crt;
        }
      }
    }

    if (found == 0) {
      // no available alternative
      // cout<<" Nothing found"<<endl;
      return crt;
    }
    *timetable = bestSchedule;
  } else {
    cout << "did not find CRT" << endl;
  }
  return c;
}

vector<CourseRoomTime>
ImproveTable::singleSwap(int crtOneIndex, int crtTwoIndex,
                         vector<CourseRoomTime> schedule)
{
  vector<CourseRoomTime> timetable = schedule;
  CourseRoomTime crtOne = timetable[crtOneIndex];
  CourseRoomTime crtTwo = timetable[crtTwoIndex];
  vector<CourseRoomTime> swapList;
  swapList.push_back(crtOne);
  swapList.push_back(crtTwo);

  int periodOne = crtOne.getVenueTime().getPeriod();
  Venue roomOne = crtOne.getVenueTime().getRoom();
  int periodTwo = crtTwo.getVenueTime().getPeriod();
  Venue roomTwo = crtTwo.getVenueTime().getRoom();

  // cout<<periodOne<<"  "<<periodTwo<<endl;
  if (periodOne == periodTwo) {
    // cout<<"Same period so swapping rooms"<<endl;
    timetable[crtOneIndex].getVenueTime().setRoom(roomTwo);
    timetable[crtTwoIndex].getVenueTime().setRoom(roomOne);
  } else {
    // cout<<" Different Periods so Kempe Swapping"<<endl;
    auto kempeChain = getKempeChain(timetable, swapList, periodOne, periodTwo);
    // for (auto &kc:kempeChain)
    //{
    // cout<<kc.toString()<<endl;
    //}
    auto temp = kempeChainSwap(kempeChain, timetable, periodOne, periodTwo);
    timetable = temp;
  }

  return timetable;
}

vector<CourseRoomTime>
ImproveTable::singleMove(int crtOneIndex, int newPeriod,
                         vector<CourseRoomTime> schedule)
{
  vector<CourseRoomTime> timetable = schedule;
  CourseRoomTime crtOne = timetable[crtOneIndex];
  vector<CourseRoomTime> swapList;
  swapList.push_back(crtOne);

  int periodOne = crtOne.getVenueTime().getPeriod();
  Venue roomOne = crtOne.getVenueTime().getRoom();
  int periodTwo = newPeriod;

  // cout<<periodOne<<"  "<<periodTwo<<endl;
  if (periodOne == periodTwo) {
    // cout<<"Same period so doing nothing"<<endl;
    // timetable[crtOneIndex].getVenueTime().setRoom(roomTwo);
    // timetable[crtTwoIndex].getVenueTime().setRoom(roomOne);
  } else {
    // cout<<" Different Periods so Kempe Swapping"<<endl;
    auto kempeChain = getKempeChain(timetable, swapList, periodOne, periodTwo);
    // for (auto &kc:kempeChain)
    //{
    // cout<<kc.toString()<<endl;
    //}
    auto temp = kempeChainSwap(kempeChain, timetable, periodOne, periodTwo);
    timetable = temp;
  }

  return timetable;
}

vector<CourseRoomTime>
ImproveTable::bestNeighbour(int crtOneIndex, vector<CourseRoomTime> schedule)
{
  auto best = schedule;
  auto b_scv = this->NumberSCV(schedule);
  //cout<<"best neighbour computation"<<endl;
  // int best_index = 0;
  for (std::size_t i = 0; i < schedule.size(); i++) {
    if ((int)i != crtOneIndex) {
      auto neighbour = singleSwap(crtOneIndex, i, schedule);
      auto neighbour_scv = this->NumberSCV(neighbour);
      if (neighbour_scv < b_scv) {
        b_scv = neighbour_scv;
        best = neighbour;
        // best_index = i;
      }
    }
    for (int j = 0; j <= maxPeriod; j++) {
      auto neighbour = singleMove(crtOneIndex, j, schedule);
      auto neighbour_scv = this->NumberSCV(neighbour);
      if (neighbour_scv < b_scv) {
        b_scv = neighbour_scv;
        best = neighbour;
        // best_index = i;
      }
    }
  }
  // cout<<"Best Index is "<<best_index<<endl;
  return best;
}

vector<CourseRoomTime> ImproveTable::applyImprovementSequence(vector<int> &seq, vector<CourseRoomTime>& timetable)
{
  vector<int> sequence(timetable.size());
  auto schedule = timetable;
  for (std::size_t i = 0; i < seq.size(); i++ ){
    int nextCRT = seq[i];

    //CRT is not out of bounds
    if (nextCRT <= (int)timetable.size()){
      auto tempSchedule = this->bestNeighbour(nextCRT, schedule);
      schedule = tempSchedule;
    }

  }
  return schedule;
}
