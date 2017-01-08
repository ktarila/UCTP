/*
 * improvetable.h
 *
 *  Created on: 10 Jul 2015
 *      Author: patrick
 *      Improve a timetable using VOIN (Vote Out Invite In Heuristic)
 */

#ifndef IMPROVETABLE_H_
#define IMPROVETABLE_H_

#include "feasibletable.h"
#include "path.h"
#include "roompath.h"
#include <chrono>
#include <mutex>

class ImproveTable
{
private:
  vector<vector<double>> adjacentMatrix;
  vector<vector<double>> roomMatrix;
  int periods_in_day;
  int maxPeriod;
  int bestSoFar;

  vector<vector<int>> possible;
  // std::mutex _lock;
  vector<Course> course;
  vector<string> curCodes;
  vector<Venue> rooms;
  vector<RoomTime> venueTime;

  // vector<CourseRoomTime> global;
  int globalBest;

  // int index = 1;

  // Ant colony optimizaton parameters
  constexpr static double t_min = 0.01; // minumum pheromone value
  constexpr static double t_max = 10;   // maximum pheromone value
  constexpr static double alpha = 8;    // heuristic weight -- high Ants less
                                        // sensitive to pheromone trail more
                                        // sensitive to heuristics
  constexpr static double beta = 2;  // trail weight     -- high means ants are
                                     // less sensitive to heuristic more
                                     // senstive to pheromone trail
  constexpr static double rho = 0.2; // rate of evapouration -- high means fast
                                     // evapouration rate (range between 0 and
                                     // 1);

  int getDay(int period);
  vector<CourseRoomTime> getCRTsinDay(const int& day,
                                      const vector<CourseRoomTime>& timetable);
  vector<string> curriculumInTimetable(const vector<CourseRoomTime>& timetable);
  // vector<string>[] getCourseRoom(const vector<CourseRoomTime>& timetable);
  vector<Course> coursesInTimetable(const vector<CourseRoomTime>& timetable);
  int numberCurriculumLastTimeSlot(const vector<CourseRoomTime>& timetable);
  vector<CourseRoomTime> getCRTsinPeriod(
    const int& period, const vector<CourseRoomTime>& timetable);
  vector<CourseRoomTime> getCRTsinGivenPeriods(
    const int& p1, const int& p2, const vector<CourseRoomTime>& timetable,
    vector<CourseRoomTime>* periodOne, vector<CourseRoomTime>* periodTwo);
  vector<CourseRoomTime> removeEmptyCourses(
    const vector<CourseRoomTime>& timetable);
  void initializeMatrix();
  vector<CourseRoomTime> getClashList(const vector<CourseRoomTime>&,
                                      const CourseRoomTime&);
  CourseRoomTime moveToBest2(int, vector<CourseRoomTime>*);
  vector<vector<int>> getBarpatiteMatrix(vector<CourseRoomTime> merged,
                                         vector<CourseRoomTime> p1,
                                         vector<CourseRoomTime> p2);
  bool hasCommon(Course c1, Course c2);
  vector<CourseRoomTime> getKempeChain(const vector<CourseRoomTime>& timetable,
                                       const vector<CourseRoomTime>& swapList,
                                       const int& periodOne,
                                       const int& periodTwo);
  vector<Venue> getTakenRooms(int period, vector<CourseRoomTime> timetable);
  vector<Venue> getAvailableRooms(int period, vector<CourseRoomTime> timetable,
                                  vector<CourseRoomTime> kempeChain);
  vector<CourseRoomTime> singleKempeSwap(vector<Venue>* availableRooms,
                                         CourseRoomTime crt, int newPeriod,
                                         std::vector<CourseRoomTime> timetable);
  std::vector<CourseRoomTime> kempeChainSwap(vector<CourseRoomTime> kempeChain,
                                             vector<CourseRoomTime> timetable,
                                             int p1, int p2);

public:
  ImproveTable(vector<RoomTime> vt, vector<Venue> rooms,
               vector<string> curCodes, vector<Course> course,
               vector<CourseRoomTime> timetable, int maxPeriod,
               int periods_in_day); // constructor
  ImproveTable();
  ~ImproveTable();
  int NumberSCV(const vector<CourseRoomTime>& timetable,
                int* roomStabilityViolations, int* courseWorkingDayViolations,
                int* consecutiveLectureViolations, int* sizeViolations);
  int NumberSCV();
  int NumberSCV(const vector<CourseRoomTime>& timetable);
  void writeTimetableToFile(string name);
  void writeTimetableToFile2();
  vector<CourseRoomTime> getCoursePeriods(const Course&);
  int getIndex(CourseRoomTime crt, vector<CourseRoomTime> timetable);
  vector<CourseRoomTime> removeCRTinPeriod(const int& period,
                                           vector<CourseRoomTime> timetable);
  void softWalk(const vector<CourseRoomTime>& timetable, const int& antIndex,
                vector<CourseRoomTime>* newScedule, int* num_scv,
                vector<int>* newPath, bool accept);
  int selectNextCRT(vector<CourseRoomTime> timetable, const int& rowIndex,
                    const int& antIndex, vector<CourseRoomTime>*);
  CourseRoomTime selectNeighbourProbability(
    vector<double> probablility, const vector<CourseRoomTime>& functions,
    const int& antIndex, int* returnIndex);
  void evapourate(int period);
  void updateAntTrail(const int& numSCV_cycleBest, const vector<int>& path);
  void initPeriod(const int& day);
  vector<CourseRoomTime> antColonySoftThread(const int& numberAnts,
                                             const int& numberCycles,
                                             const int& num_ber, bool accept);
  vector<CourseRoomTime> antColonySoft(const int& numberAnts,
                                       const int& numberCycles,
                                       const int& num_ber, bool accept);
  vector<CourseRoomTime> fullSchedule;
  void setFullSchedule(vector<CourseRoomTime>);
  void writeTimetableToFileSolutionFormat(string solution,
                                          vector<CourseRoomTime> timetable);
  vector<CourseRoomTime> runImprovement(int numIterations, int numAnts,
                                        int numCycles);
  int getMaxPeriod();
  int getPeriodValue(int period, int day);
  vector<CourseRoomTime> singleSwap(int crtOneIndex, int crtTwoIndex,
                                    vector<CourseRoomTime> timetable);
  vector<CourseRoomTime> singleMove(int crtOneIndex, int newPeriod,
                                    vector<CourseRoomTime> schedule);
  vector<CourseRoomTime> bestNeighbour(int crtOneIndex,
                                       vector<CourseRoomTime> schedule);
  vector<CourseRoomTime> applyImprovementSequence(vector<int> &seq, vector<CourseRoomTime>& timetable);
};

#endif /* IMPROVETABLE_H_ */
