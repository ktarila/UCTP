/*
 * feasibletable.cpp
 *
 *  Created on: 27 Jun 2015
 *      Author: patrick
 */

#include "feasibletable.h"
#include "improvetable.h"
#include "smethods.h"
#include <algorithm>
#include <fstream>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <time.h>
#include <vector>

// random generator function:
int myrandomF(int i) {
  unsigned seed1 = std::chrono::system_clock::now().time_since_epoch().count();
  // double seed1 = SM::get_cpu_time();
  // std::cout<<seed1<<" "<<i<<std::endl;
  std::minstd_rand0 g1(seed1);
  return g1() % i;
}

FeasibleTable::FeasibleTable(vector<Course> course, vector<Venue> rooms,
                             int maxPeriod, int periods_in_day,
                             vector<Curricula> curCodes)
    : course(course), periods_in_day(periods_in_day), rooms(rooms) {
  this->check = true;
  // this->course = course;
  this->maxPeriod = maxPeriod + 1;
  // cout<<"Maximum Period is: "<<this->maxPeriod;
  // this->periods_in_day = periods_in_day;
  // this->rooms = rooms;
  this->createVenueTimeList(rooms);
  this->globalBest = 5000000; // set global best to very large number

  // dynamic allocation of adjacent matrix
  this->adjacentMatrix.resize(this->course.size());
  for (std::size_t i = 0; i < this->course.size(); i++) {
    adjacentMatrix[i].resize(this->venueTime.size());
  }

  // initialize adjacent matrix
  this->initializeAdjacentMatrix();

  // save curriculum codes to a vector
  vector<Curricula>::iterator cIterator = curCodes.begin();
  while (cIterator != curCodes.end()) {
    Curricula *curri = &(*cIterator);
    this->curCodes.push_back(curri->getCode());
    ++cIterator;
  }

  // save lecture list
  this->setLectureList();
  cout<<lectureList.size();
}

FeasibleTable::~FeasibleTable() {}

void FeasibleTable::setLectureList() {
  // cout<<"Graph walk Iteration: "<<cycle<<" with num HCV ";
  // vector<CourseRoomTime> crt;
  std::vector<Course> listOfLectures;

  // copy of course list
  std::vector<Course>::iterator courseIterator = this->course.begin();
  std::vector<Course> copyCourse;
  while (courseIterator != this->course.end()) {
    Course c = *courseIterator;
    copyCourse.push_back(c);
    ++courseIterator;
  }

  std::vector<Course>::iterator copyCourseIterator = copyCourse.begin();
  while (copyCourseIterator != copyCourse.end()) {
    Course c = *copyCourseIterator;
    int numWeeklyLectures = c.getWeeklyLectures();
    for (int i = 0; i < numWeeklyLectures; i++) {
      listOfLectures.push_back(c);
    }
    ++copyCourseIterator;
  }
  this->lectureList = listOfLectures;
}

/**
 *
 * @param rooms
 * @param maxPeriod
 * Creates all possible combination of VenueTimes.
 */
void FeasibleTable::createVenueTimeList(vector<Venue> rooms) {
  vector<Venue>::iterator vIterator;
  vIterator = rooms.begin();
  while (vIterator != rooms.end()) {
    Venue v = *vIterator;
    for (int i = 0; i < this->maxPeriod; i++) {
      RoomTime rt(i, v);
      this->venueTime.push_back(rt);
    }
    ++vIterator;
  }
  // this->printRoomTime();
  // cout<<&v;
}

vector<Course> FeasibleTable::getCourse() const { return course; }

vector<Course> FeasibleTable::getLectureList() const { return lectureList; }

vector<string> FeasibleTable::getCurCodes() const { return curCodes; }

int FeasibleTable::getMaxPeriod() const { return maxPeriod; }

int FeasibleTable::getPeriodsInDay() const { return periods_in_day; }

vector<CourseRoomTime> FeasibleTable::getFeasibleTable() const {
  return feasibleTable;
}

vector<RoomTime> FeasibleTable::getVenueTime() const { return venueTime; }

void FeasibleTable::setFeasibleTable(vector<CourseRoomTime> timetable) {
  this->feasibleTable = timetable;
}

void FeasibleTable::printRoomTime() {
  vector<RoomTime>::iterator rtIterator;
  rtIterator = this->venueTime.begin();
  while (rtIterator != this->venueTime.end()) {
    RoomTime v = *rtIterator;

    cout << v.getPeriod() << " " << v.toString() << endl;

    ++rtIterator;
  }
}

/**
 *
 * @param initial
 * Creates the adjacent matrix for the timetabling problem
 * Parameter initial is the initial weight (pheromone) of edges
 * Edges from course to VenueTime exists if that course can be
 * assigned to that venue and time.
 */
void FeasibleTable::initializeAdjacentMatrix() {
  // this->printRoomTime();
  int i = 0;
  // int j = 0;
  std::vector<Course>::iterator courseIterator = this->course.begin();
  std::vector<RoomTime>::iterator roomTimeIterator;

  while (courseIterator != this->course.end()) {
    int j;
    Course *c = &(*courseIterator);
    vector<string> roomConstr = c->getRoomConstraint();
    vector<int> unAvailConstr = c->getUnavailableConstraint();
    roomTimeIterator = this->venueTime.begin();
    j = 0;

    /*
    std::vector<int>::iterator unIter = unAvailConstr.begin();
    cout<<"Unavailable constraints  for "<<c->getCode()<<"  ";
    while (unIter != unAvailConstr.end())
    {
            int per = *unIter;
            cout<<per<<"\t";
            unIter++;
    }
    cout<<endl;
    */

    while (roomTimeIterator != this->venueTime.end()) {
      RoomTime rt = *roomTimeIterator;
      // cout<<rt->toString();
      // Venue* ven = rt->getRoom();
      string roomCode = rt.getRoom().getName();
      // cout<<ven->toString();
      int period = rt.getPeriod();
      // cout<<period<<" "<<roomCode;

      // check if room code constraint exists
      std::vector<string>::iterator findRoom =
          std::find(roomConstr.begin(), roomConstr.end(), roomCode);

      // check if period constraint exists
      std::vector<int>::iterator findPeriod =
          std::find(unAvailConstr.begin(), unAvailConstr.end(), period);

      // can put in room (exists in constraint list) and can put in period (not
      // exist in unavail constraint list)
      if (findRoom != roomConstr.end() && findPeriod == unAvailConstr.end()) {
        this->adjacentMatrix[i][j] = this->t_max;
        // cout<<i<<"\t"<<j<<"\t"<<this->adjacentMatrix[i][j]<<"
        // "<<c->getCode()<<" "<<rt.toString()<<endl;

      } else {
        this->adjacentMatrix[i][j] = 0;
        // cout<<i<<"\t"<<j<<"\t"<<this->adjacentMatrix[i][j]<<"
        // "<<c->getCode()<<" "<<rt.toString()<<endl;
      }
      j++;
      ++roomTimeIterator;
    }
    ++courseIterator;
    i++;
  }
}


/**
 * A graph walk that constructs a timetable at the end of walk
 * Timetable may not be feasible
 */
void FeasibleTable::antGraphWalk(vector<CourseRoomTime> *timetable, int cycle,
                                 int *num_hcv, int *num_scv) {
  // cout<<"Graph walk Iteration: "<<cycle<<" with num HCV ";
  // vector<CourseRoomTime> crt;
  std::vector<RoomTime> tabuRoomTime;
  // std::array<std::vector<string>, 3> curriculaPeriod; // default construction
  vector<string> curriculaPeriod[this->maxPeriod];
  vector<string> lecturerPeriod[this->maxPeriod];

  // copy of course list

  std::vector<Course> copyCourse;

  std::vector<Course>::iterator courseIterator = this->lectureList.begin();
  while (courseIterator != this->lectureList.end()) {
    Course c = *courseIterator;
    copyCourse.push_back(c);
    ++courseIterator;
  }

  // unsigned int rSeed = time(NULL) * cycle;
  // cout<<"\t\t"<<rSeed;

  // std::srand(rSeed);
  SM s;
  // std::random_shuffle(
  //     copyCourse.begin(), copyCourse.end(),
  //     myrandomF); // shuffle courses to select random to schedule


  // while not all courses have not been scheduled to a timetable
  // select a random course and schedule
  std::sort(copyCourse.begin(), copyCourse.end());
  std::vector<Course>::iterator copyCourseIterator = copyCourse.begin();
  while (copyCourseIterator != copyCourse.end()) {
    Course c = *copyCourseIterator;
    // cout<<c.getCode()<<"\t"<<endl;
    // cout<<c.getAvailable()<<"\t"<<endl;

    // int doublePeriods = c.getDoubleLectures();
    // int tabuDaysSize = c.getMinWorkingDays();
    // int numWeeklyLectures = c.getWeeklyLectures();
    list<int> tabuDays;
    // int assigned =0;

    vector<CourseRoomTime> tempTimetable = *timetable;
    RoomTime rt = antCourseWalk(c, tabuRoomTime, tabuDays, curriculaPeriod,
                                lecturerPeriod, cycle, tempTimetable);
    CourseRoomTime courseroomtime(c, rt);
    courseroomtime.setUpdate(this->check);

    std::vector<CourseRoomTime>::iterator findCRT =
        std::find(timetable->begin(), timetable->end(), courseroomtime);
    if (findCRT != timetable->end()) {
      cout << "Same course in same time and room twice" << endl;
      // timetable->push_back(courseroomtime);
    } else {
      // timetable->push_back(courseroomtime);
    }
    timetable->push_back(courseroomtime);
    // assigned++;

    // add roomtime to tabu list
    std::vector<RoomTime>::iterator findRT =
        std::find(tabuRoomTime.begin(), tabuRoomTime.end(), rt);
    if (findRT == tabuRoomTime.end())
      tabuRoomTime.push_back(rt);

    // add curriculums to curriculumperiod tabu
    vector<string> cCodes = c.getCurriculaCodes();
    std::vector<string>::iterator itcCodes = cCodes.begin();
    while (itcCodes != cCodes.end()) {
      string code_str = *itcCodes;
      std::vector<string>::iterator findCode_str =
          std::find(curriculaPeriod[rt.getPeriod()].begin(),
                    curriculaPeriod[rt.getPeriod()].end(), code_str);
      if (findCode_str == curriculaPeriod[rt.getPeriod()].end())
        curriculaPeriod[rt.getPeriod()].push_back(code_str);
      ++itcCodes;
    }

    // add lecturer to lecturerperiod
    std::vector<string>::iterator findLecturer =
        std::find(lecturerPeriod[rt.getPeriod()].begin(),
                  lecturerPeriod[rt.getPeriod()].end(), c.getLecturer());
    if (findLecturer == lecturerPeriod[rt.getPeriod()].end())
      lecturerPeriod[rt.getPeriod()].push_back(c.getLecturer());

    /*
     * Add day to tabu Day list of size number of minimum working days.
     * To avoid minimum working days constraint
     */
    int day = rt.getDay(this->periods_in_day);
    int tabuSize = tabuDays.size();
    if (tabuSize == c.getMinWorkingDays()) {
      tabuDays.pop_back();
      tabuDays.push_front(day);
    } else {
      tabuDays.push_front(day);
    }

    // cout<<"Assigned "<<c.getCode()<<"  "<<assigned<<" times"<<endl;

    ++copyCourseIterator;
  }
  // cout<<endl<<"********************"<<endl;
  ImproveTable impTable(getVenueTime(), rooms, getCurCodes(), getCourse(),
                        *timetable, getMaxPeriod(), getPeriodsInDay());
  int scv = impTable.NumberSCV();
  // int scv = SM::getConsecutiveViolations(*timetable, this)
  int hcv = this->NumberHCV(*timetable);
  *num_hcv = hcv;
  *num_scv = scv;
  // cout<<*num_hcv<<endl;
}

/**
 * Assign a RoomTime to a course
 */
RoomTime FeasibleTable::antCourseWalk(Course c, vector<RoomTime> tabuVenueTime,
                                      list<int> tabuDays,
                                      vector<string> curriculaPeriod[],
                                      vector<string> lecturerPeriod[],
                                      int cycle,
                                      vector<CourseRoomTime> timetable) {
  this->check = true;
  vector<RoomTime> adjacentVertices =
      getVenueTimeVertices(c); // get possible roomtimes for course
  // cout<<adjacentVertices.size()<<" adjacentVertices"<<endl;
  vector<RoomTime> L1Filter = filterVenueListTabu(
      tabuVenueTime, adjacentVertices); // remove roomperiods already assigned
  // cout<<L1Filter.size()<<" L1Filter"<<endl;
  vector<RoomTime> L2Filter = this->filterCurriculumPeriodTabu(
      c, curriculaPeriod,
      L1Filter); // remove RoomPeriods that will cause curriculum clash
  // cout<<L2Filter.size()<<" L2Filter"<<endl;
  vector<RoomTime> L3Filter = this->filterLecturerPeriodTabu(
      c, lecturerPeriod,
      L2Filter); // remove RoomPeriods that will cause lecturer clash
  // cout<<L3Filter.size()<<" L3Filter"<<endl;
  vector<RoomTime> L4Filter = this->filterDayListTabu(
      L3Filter, tabuDays); // try to meet minimum days constraint
  // cout << L4Filter.size() << " L4Filter" << endl;


  // set probabilities of edges
  double probability[L4Filter.size()];

  for (std::size_t i = 0; i < L4Filter.size(); i++) {
    RoomTime rtTemp = L4Filter[i];
    double venueClash = 0;
    double studentClash = 0;
    double dayConstraint = 0;
    double lecturerClash = 0;

    // check if there is a room clash
    std::vector<RoomTime>::iterator findRT =
        std::find(tabuVenueTime.begin(), tabuVenueTime.end(), rtTemp);
    if (findRT != tabuVenueTime.end()) {
      venueClash = 1;
      this->check = false;
    }

    // check if there is a lecturer clash
    std::vector<string>::iterator findLecturer =
        std::find(lecturerPeriod[rtTemp.getPeriod()].begin(),
                  lecturerPeriod[rtTemp.getPeriod()].end(), c.getLecturer());
    if (findLecturer != lecturerPeriod[rtTemp.getPeriod()].end()) {
      lecturerClash = 1;
      this->check = false;
    }

    // check if violates day constraint
    std::list<int>::iterator findDay = std::find(
        tabuDays.begin(), tabuDays.end(), rtTemp.getDay(this->periods_in_day));
    if (findDay != tabuDays.end()) {
      dayConstraint = 0.3;
    }

    // check if there is a student clash
    std::vector<string> cCurr = c.getCurriculaCodes();
    std::vector<string>::iterator cCurrIterator = cCurr.begin();
    bool l = false;
    while (cCurrIterator != cCurr.end() && !l) {
      string curCod = *cCurrIterator;
      std::vector<string>::iterator findCurriculum =
          std::find(curriculaPeriod[rtTemp.getPeriod()].begin(),
                    curriculaPeriod[rtTemp.getPeriod()].end(), curCod);
      if (findCurriculum != curriculaPeriod[rtTemp.getPeriod()].end())
        l = true;
      cCurrIterator++;
    }
    if (l) {
      studentClash = 1;
      this->check = false;
    }

    auto temp = timetable;
    CourseRoomTime tempCRT(c, rtTemp);
    temp.push_back(tempCRT);
    int conViolate = SM::getConsecutiveViolations(temp, rtTemp.getPeriod(),
                                                  this->getPeriodsInDay());
    conViolate -= this->curCodes.size();

    double trail = this->adjacentMatrix[this->getAdjacentCourseIndex(c)]
                                       [this->getAdjacentRoomTimeIndex(rtTemp)];
    double heuristic =
        ((2.3 - studentClash + venueClash + lecturerClash + dayConstraint) *
         conViolate) -
        conViolate;

    double trailFactor = pow(trail, this->beta);
    double heuristicFactor = pow(heuristic, this->alpha);
    // double heuristicFactor = 0;
    probability[i] = trailFactor + heuristicFactor;
  }

  // cout<<"here";

  // return L4Filter[0]; //change to probability

  return this->selectRoom(probability, L4Filter, cycle);
}

/**
 *
 * @param tabuList
 * @param vt
 * @return
 * Returns a new arrayList of venue times
 * That is filtered, meaning it does not contain any venue time element in the
 * tabuList
 * Tabulist is the list of all venue time elements already assigned to a course
 */
vector<RoomTime>
FeasibleTable::filterVenueListTabu(vector<RoomTime> tabuList,
                                   vector<RoomTime> vt) // VenueTime Tabu Filter
{
  vector<RoomTime> tabuFiltered;
  std::vector<RoomTime>::iterator rtIterator = vt.begin();
  while (rtIterator != vt.end()) {
    RoomTime rt = *rtIterator;
    std::vector<RoomTime>::iterator findRT =
        std::find(tabuList.begin(), tabuList.end(), rt);
    if (findRT == tabuList.end()) // Not in tabuList
      tabuFiltered.push_back(rt);
    ++rtIterator;
  }
  if (tabuFiltered.size() > 0)
    return tabuFiltered;

  // cout<< " A clash will happen on room time"<< endl;

  return vt; // Return old which will result in a clash
}

vector<RoomTime> FeasibleTable::getVenueTimeVertices(const Course &c) {
  vector<RoomTime> adjacent;

  std::vector<Course>::iterator findIter =
      std::find(this->course.begin(), this->course.end(), c);
  if (findIter != this->course.end()) {
    int index = std::distance(this->course.begin(), findIter);
    // cout<<course[index].getCode()<<"  "<<index<<"---";
    int amount = 0;
    for (std::size_t i = 0; i < this->venueTime.size(); i++) {
      if (this->adjacentMatrix[index][i] > 0) {
        adjacent.push_back(*(&this->venueTime[i]));
        amount++;
      } else {
        /// cout<<"Not adding for "<<index << " "<<i<<endl;
      }
    }
  } else {
    cout << "No such course";
    return adjacent;
  }
  // cout<<adjacent.size()<<" The number of possible venue times "<<
  // amount<<endl;
  return adjacent;
}

/**
 *
 * @return
 * Returns a new arrayList of venue times
 * That is filtered, meaning it does not contain any venue time element that
 * that will result in a curriculum clash
 */
vector<RoomTime> FeasibleTable::filterCurriculumPeriodTabu(
    Course c, vector<string> curriculaPeriod[], vector<RoomTime> fVenueTabu) {
  vector<RoomTime> currFiltered;
  std::vector<RoomTime>::iterator rtIterator;
  rtIterator = fVenueTabu.begin();

  std::vector<string> curriculumCodes = c.getCurriculaCodes();
  std::vector<string>::iterator ccIterator;

  while (rtIterator != fVenueTabu.end()) {
    RoomTime rt = *rtIterator;
    ccIterator = curriculumCodes.begin();
    bool l = true;
    while (ccIterator != curriculumCodes.end() && l) {
      string code = *ccIterator;
      std::vector<string>::iterator findCode =
          std::find(curriculaPeriod[rt.getPeriod()].begin(),
                    curriculaPeriod[rt.getPeriod()].end(), code);
      if (findCode != curriculaPeriod[rt.getPeriod()].end())
        l = false;
      ++ccIterator;
    }
    if (l) // if no clash add to filtered
      currFiltered.push_back(rt);
    ++rtIterator;
  }

  if (currFiltered.size() > 0)
    return currFiltered;

  // cout<< "Curriculum clash will occur" <<endl;

  return fVenueTabu; // return old vector if filtered is empty
}

/**
 * @param tabuList
 * @param vt
 * @return
 * Returns a new arrayList of venue times
 * That is filtered, meaning it does not contain any venue time element that
 * that will result in a lecturer clash
 */
vector<RoomTime> FeasibleTable::filterLecturerPeriodTabu(
    Course c, vector<string> lecturerPeriod[], vector<RoomTime> fCurPerTabu) {
  vector<RoomTime> lecFiltered;
  std::vector<RoomTime>::iterator rtIterator;
  rtIterator = fCurPerTabu.begin();

  std::string lecturer = c.getLecturer();

  while (rtIterator != fCurPerTabu.end()) {
    RoomTime rt = *rtIterator;
    std::vector<string>::iterator findLecturer =
        std::find(lecturerPeriod[rt.getPeriod()].begin(),
                  lecturerPeriod[rt.getPeriod()].end(), lecturer);

    if (findLecturer ==
        lecturerPeriod[rt.getPeriod()]
            .end()) // if not assigning same lecturer to same period
      lecFiltered.push_back(rt);
    ++rtIterator;
  }

  if (lecFiltered.size() > 0)
    return lecFiltered;
  return fCurPerTabu; // return old if filtered is empty
}

/**
 *
 * @param roomTimes
 * @param dayTabu
 * @return
 * Remove room time elements with those days in the tabu list
 */
vector<RoomTime> FeasibleTable::filterDayListTabu(vector<RoomTime> roomTimes,
                                                  list<int> dayTabu) {
  vector<RoomTime> dayFiltered;
  std::vector<RoomTime>::iterator rtIterator = roomTimes.begin();

  while (rtIterator != roomTimes.end()) {
    RoomTime rt = *rtIterator;
    std::list<int>::iterator findDay = std::find(
        dayTabu.begin(), dayTabu.end(), rt.getDay(this->periods_in_day));

    if (findDay == dayTabu.end()) // if not in day tabu
      dayFiltered.push_back(rt);

    ++rtIterator;
  }

  if (dayFiltered.size() > 0)
    return dayFiltered;

  return roomTimes; // return old if filtered is empty
}

/**
 * Get the index of given course on course vector
 */
int FeasibleTable::getAdjacentCourseIndex(Course c) {
  std::vector<Course>::iterator findCourse =
      std::find(this->course.begin(), this->course.end(), c);
  int index = distance(this->course.begin(), findCourse);

  return index;
}

/**
 * get the index of Roomtime on roomtime vector
 */
int FeasibleTable::getAdjacentRoomTimeIndex(RoomTime rt) {
  std::vector<RoomTime>::iterator findRT =
      std::find(this->venueTime.begin(), this->venueTime.end(), rt);
  int index = distance(this->venueTime.begin(), findRT);

  return index;
}

/**
 * Return a course on given room time vector determined by probability array
 */
RoomTime FeasibleTable::selectRoom(double probability[], vector<RoomTime> rt,
                                   int cycle) {
  // cout<<"Here"<<endl;
  // cout<<rt.size()<<"\t";
  int length = rt.size();
  // cout<<rt.size();
  double sum = 0;
  for (int i = 0; i < length; i++) {
    sum += probability[i];
  }

  for (int i = 0; i < length; i++) {
    probability[i] /= sum;
  }

  int unsigned seed = time(NULL) * cycle;
  double number = rand_r(&seed) / (double)RAND_MAX;

  double cumulativeProbability = 0;
  for (int i = 0; i < length; i++) {
    cumulativeProbability += probability[i];
    if (number <= cumulativeProbability) {
      return rt[i];
    }
  }

  cout << "Error in cumulative Probability" << endl;
  return rt[0]; // should not get to this point
}

/**
 * Finds the number of hard constraint violations for a given complete timetable
 * Each hard Constraint weighs one.
 * Hard Constraints violations are:
 * RC - if two courses are assigned to the same room in the same period, two
 * courses in the same venue
 * SC - Students are assigned multiple courses in the same period, two
 * curriculums in the same period
 * LC - Lecturer is assigned multiple courses in the same period
 * FC - Venue assigned to a course doesn't have the required features
 * SC - Size of venue assigned to a course is less than number of students
 * taking the course
 * UPC - Course assigned a period when the lecturer is not available to teach a
 * course
 * @param timetable - vector of CourseRoomTime Objects, a complete time table
 * @return integer - Number of hard constraint violations
 * When the number of HCVs is zero a feasible timetable has been found
 */
int FeasibleTable::NumberHCV(vector<CourseRoomTime> timetable) {
  int roomViolation = 0;
  int curriculumViolation = 0;
  int unavailableViolation = 0;
  int featureSizeViolation = 0;
  int lecturerViolation = 0;

  for (int i = 0; i < this->maxPeriod; i++) // get violations in each period
  {
    vector<string> curGlobal;
    vector<string> roomGlobal;
    vector<string> lecturerGlobal;

    vector<Course> periodCourse = this->getPeriodCourse(i, timetable);

    vector<Course>::iterator cIterator = periodCourse.begin();
    while (cIterator != periodCourse.end()) {
      Course c = *cIterator;

      // get curriculum violations
      vector<string> cpcurCodes = c.getCurriculaCodes();
      vector<string>::iterator curIterator = cpcurCodes.begin();
      int cpi = 0;
      while (curIterator != cpcurCodes.end()) {
        string cpCode = *curIterator;
        vector<string>::iterator findCPcode =
            find(curGlobal.begin(), curGlobal.end(), cpCode);
        if (findCPcode != curGlobal.end()) {
          curriculumViolation++;
        }
        if (findCPcode == curGlobal.end()) {
          curGlobal.push_back(cpCode);
        }
        cpi++;
        ++curIterator;
      }

      // get room clash violations
      string roomName = this->getVenueCourse(i, c, timetable);
      vector<string>::iterator findRName =
          find(roomGlobal.begin(), roomGlobal.end(), roomName);
      if (findRName != roomGlobal.end())
        roomViolation++;
      else if (findRName == roomGlobal.end())
        roomGlobal.push_back(roomName);

      // get room feature or size violations
      vector<string> fsConst =
          c.getRoomConstraint(); // must be in this vector else a violation

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
      vector<string>::iterator findRoom =
          find(fsConst.begin(), fsConst.end(), roomName);
      if (findRoom == fsConst.end()) {
        featureSizeViolation++;
        cout << c.getCode() << " " << roomName << endl;
        ;
      }

      // get Lecturer violations
      vector<string>::iterator findLecturer =
          find(lecturerGlobal.begin(), lecturerGlobal.end(), c.getLecturer());
      if (findLecturer != lecturerGlobal.end())
        lecturerViolation++;
      else if (findLecturer == lecturerGlobal.end())
        lecturerGlobal.push_back(c.getLecturer());

      // get unavailable period violations
      vector<int> unavailableConst = c.getUnavailableConstraint();
      vector<int>::iterator findUnavailablePeriod =
          find(unavailableConst.begin(), unavailableConst.end(), i);
      if (findUnavailablePeriod != unavailableConst.end())
        unavailableViolation++;
      ++cIterator;
    }

    periodCourse.erase(periodCourse.begin(), periodCourse.end());
    lecturerGlobal.erase(lecturerGlobal.begin(), lecturerGlobal.end());
    roomGlobal.erase(roomGlobal.begin(), roomGlobal.end());
    curGlobal.erase(curGlobal.begin(), curGlobal.end());
  }
  int HCV = roomViolation + curriculumViolation + unavailableViolation +
            featureSizeViolation + lecturerViolation;
  // cout << "\troomViolation: " << roomViolation << "\t"
  //      << "curriculumViolation:" << curriculumViolation << "\t"
  //      << "unavailableViolation: " << unavailableViolation << "\t"
  //      << "featureSizeViolation: " << featureSizeViolation << "\t"
  //      << "lecturerViolation: " << lecturerViolation << endl;
  // ;
  return HCV;
}

/**
 * Get the list of courses taken in a particular period from timetable
 * @param period
 * @return
 */
vector<Course>
FeasibleTable::getPeriodCourse(int period, vector<CourseRoomTime> timetable) {
  vector<Course> cees;
  std::vector<CourseRoomTime>::iterator tIterator = timetable.begin();
  while (tIterator != timetable.end()) {
    CourseRoomTime c = *tIterator;
    if (c.getVenueTime().getPeriod() == period &&
        c.getCourse().isEmpty() == false) {
      Course cous = c.getCourse();
      cees.push_back(cous);
      // std::vector<Course>::iterator findCourse = std::find(cees.begin(),
      // cees.end(), cous);
      // if (findCourse == cees.end())
      // cees.push_back(cous);
    }
    ++tIterator;
  }
  return cees;
}

/**
 * Get the name of venue of course taken in a particular period from timetable
 * @param period, timetable and course
 * @return
 * Venue name
 */
string FeasibleTable::getVenueCourse(int period, Course c,
                                     vector<CourseRoomTime> timetable) {
  string v = "";
  std::vector<CourseRoomTime>::iterator crtIterator = timetable.begin();
  while (crtIterator != timetable.end()) {
    CourseRoomTime crt = *crtIterator;
    Course cous = crt.getCourse();
    if (cous == c && crt.getVenueTime().getPeriod() == period &&
        cous.isEmpty() == false)
      return crt.getVenueTime().getRoom().getName();
    ++crtIterator;
  }
  return v;
}

void FeasibleTable::evapourate() {
  double evapourate = 1 - this->rho;
  for (std::size_t i = 0; i < this->course.size(); i++) {
    for (std::size_t j = 0; j < this->venueTime.size(); j++) {
      this->adjacentMatrix[i][j] *= evapourate;
      if (this->adjacentMatrix[i][j] < this->t_min &&
          this->adjacentMatrix[i][j] != 0)
        this->adjacentMatrix[i][j] = this->t_min;
    }
  }
}

void FeasibleTable::updateAntTrail(int timetableQuality,
                                   vector<CourseRoomTime> timetable) {
  double reward = 1 / (1 + (double)timetableQuality -
  (double)this->globalBest);

  if (this->globalBest == timetableQuality)
    reward = 0.9;

  if (this->globalBest > timetableQuality)
    reward = 1;

  // Positive and negative rewards
  // double reward =
  //     (1 / (1 + (double)timetableQuality - (double)this->globalBest)) - 1;
  // reward = reward / 10;

  // if (this->globalBest == timetableQuality){
  //   reward = 1;
  // }


  // if (this->globalBest > timetableQuality)
  //   // reward = 1;
  //   reward =
  //       1 - (1 / (1 + (double)this->globalBest - (double)timetableQuality));

  cout << "\t\t\t\t\tReward: " << reward << endl;

  this->evapourate();
  vector<CourseRoomTime>::iterator crtIterator = timetable.begin();
  while (crtIterator != timetable.end()) {
    CourseRoomTime crt = *crtIterator;
    int indexCourse = this->getAdjacentCourseIndex(crt.getCourse());
    int indexRoomTime = this->getAdjacentRoomTimeIndex(crt.getVenueTime());

    adjacentMatrix[indexCourse][indexRoomTime] +=
        reward; // Add reward based on quality of solution
    if (adjacentMatrix[indexCourse][indexRoomTime] > t_max) {
      adjacentMatrix[indexCourse][indexRoomTime] = t_max;
    }
    if (adjacentMatrix[indexCourse][indexRoomTime] < t_min) {
      adjacentMatrix[indexCourse][indexRoomTime] = t_min;
    }
    ++crtIterator;
  }
}

void FeasibleTable::printAdjMat() {
  // cout<<"here 1 "<<this->course.size()<<"\t" <<this->venueTime.size() <<endl;
  for (std::size_t i = 0; i < this->course.size(); i++) {
    // cout<<"Here 2"<<endl;
    for (std::size_t j = 0; j < this->venueTime.size(); j++) {
      int val = adjacentMatrix[i][j];
      cout << "[ " << i << " ][ " << j << " ]= " << val << "\t";
      // cout<<"here 3"<<endl;
      // cout<<"course " << i << " VenueTime "<< j<< "- ";
      // cout<<this->adjacentMatrix[j][j]<<"\t";
    }
    cout << endl;
  }
}

FeasibleTable::FeasibleTable() {}

int FeasibleTable::NumberHCV() { return this->NumberHCV(this->feasibleTable); }

void FeasibleTable::antColonyThread(int numAnts, int numCycles) {
  vector<CourseRoomTime> globe;
  ImproveTable tempTable(getVenueTime(), rooms, getCurCodes(), getCourse(),
                         globe, getMaxPeriod(), getPeriodsInDay());
  bool found = false;
  int number = 0;

  for (int i = 0; i < numCycles; i++) {
    // int cycleb = 5000000;
    vector<CourseRoomTime> *antWalk =
        new vector<CourseRoomTime>[numAnts]; // create vector of results
    int num_hcv[numAnts]; // create vector of number of hard constraint
                          // violations
    int num_scv[numAnts]; // create vector of number of soft constraint
                          // violations
    std::vector<std::thread> th;
    for (int j = 0; j < numAnts;
         j++) // create threads of number of ants to run in parallel
    {
      th.push_back(std::thread(&FeasibleTable::antGraphWalk, this, &antWalk[j],
                               ((i + 1) * (j + 1)), &num_hcv[j], &num_scv[j]));
    }

    for (auto &t : th) // join all ants threads
    {
      t.join();
    }

    // find cycle best;
    int cycleb = RAND_MAX;
    int cycleBestIndex = 0;
    for (int k = 0; k < numAnts; k++) {
      // int num_hcv = this->NumberHCV(antWalk[k]);
      // int antValue = (num_hcv[k] * num_scv[k]) + num_scv[k];
      int antValue;
      if (found == false) {
        antValue = num_hcv[k];
      } else {
        antValue = (num_hcv[k] * num_scv[k]) + num_scv[k];
      }
      if (antValue < cycleb) {
        cycleb = antValue;
        cycleBestIndex = k;
      }
      cout << "Cycle: " << i << " Ant: " << k
           << " Num HCV violations: " << num_hcv[k]
           << "   Num SCV Violations: " << num_scv[k]
           << " Ant Value: " << antValue << endl;
      if (num_scv[k] == 0 && num_hcv[k] == 0)
        if (num_hcv[k] == 0) {
          globe = antWalk[k];
          this->feasibleTable = globe;
          cout << "Found Global Best is: " << this->globalBest << " in cycle "
               << i << " Size: " << globe.size() << " SCV: " << num_scv[k]
               << " HCV: " << num_hcv[k] << endl;
          delete[] antWalk;
          return;
        }
    }

    // update trail
    cout << this->globalBest << " " << antWalk[cycleBestIndex].size()
         << " Size of cycle best "
         << " Quality of cycle best [" << num_hcv[cycleBestIndex] << " , "
         << num_scv[cycleBestIndex] << " ]" << endl;
    this->updateAntTrail(cycleb, antWalk[cycleBestIndex]);
    if (cycleb < this->globalBest) {
      this->globalBest = cycleb;
      globe = antWalk[cycleBestIndex];
      tempTable.writeTimetableToFileSolutionFormat("tempResult.sol", globe);
    }
    if (num_hcv[cycleBestIndex] == 0) {
      found = true;
      number++;
    }
    if (number == 1) {
      this->globalBest = num_scv[cycleBestIndex];
    }
    // cout<<"Global Best is: " << this->globalBest<<endl;

    // delete antwalk vector array to avoid memory leak

    delete[] antWalk;
  }

  cout << "Found Global Best is: " << this->globalBest << " in " << numCycles
       << " cycles " << endl;

  this->feasibleTable = globe;
  // return globe;
}

void FeasibleTable::antColony(int numAnts, int numCycles) {
  vector<CourseRoomTime> globe;
  ImproveTable tempTable(getVenueTime(), rooms, getCurCodes(), getCourse(),
                         globe, getMaxPeriod(), getPeriodsInDay());
  bool found = false;
  int number = 0;

  for (int i = 0; i < numCycles; i++) {
    vector<CourseRoomTime> cyclebest;
    int cycleb = 5000000;
    for (int j = 0; j < numAnts; j++) {
      vector<CourseRoomTime> antWalk;
      int hcv_ant = 0;
      int scv_ant = 0;

      this->antGraphWalk(&antWalk, (j + 1), &hcv_ant, &scv_ant);
      int antValue;
      if (found == false) {
        antValue = hcv_ant;
      } else {
        antValue = (hcv_ant * scv_ant) + scv_ant;
      }
      if (cycleb > antValue) {
        cycleb = antValue;
        cyclebest = antWalk;
      }
      cout << "Cycle: " << i << " Ant: " << j
           << " Num HCV violations: " << hcv_ant << endl;
      if (scv_ant == 0 && hcv_ant == 0) {
        this->globalBest = hcv_ant;
        globe = cyclebest;
        cout << "Found Global Best is: " << this->globalBest << " in cycle "
             << i << endl;
        this->feasibleTable = globe;
        return;
      }
      if (hcv_ant == 0) {
        found = true;
        number++;
      }
      if (number == 1) {
        this->globalBest = RAND_MAX;
      }
    }

    this->updateAntTrail(cycleb, cyclebest);
    // cout<<cyclebest.size() << " Size of cycle best "<<endl;
    if (cycleb < this->globalBest) {
      this->globalBest = cycleb;
      globe = cyclebest;
      // tempTable.writeTimetableToFileSolutionFormat("tempResult.sol", globe);
    }
    cout << "Global Best is: " << this->globalBest << endl;
  }

  cout << "Found Global Best is: " << this->globalBest << " in " << numCycles
       << " cycles " << endl;
  this->feasibleTable = globe;
}
