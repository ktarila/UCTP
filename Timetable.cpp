// reading a text file
#include <sstream>
#include <string>
#include "ReadCRT.h"
#include "Schedule/enhancement.h"
#include "Schedule/feasibletable.h"
#include "Schedule/improvetable.h"
#include "Schedule/smethods.h"
#include "data.h"

using namespace std;

int main() {
  srand(time(NULL));
  const char* filename = "InputData/ITC-2007_ectt/comp11.ectt";
  //const char* solfilename = "/home/patrick/work/UCTP-CPP/Results/comp11.sol";
  // const char* filename = "InputData/Test_ectt/toy.ectt";
  Data* data = new Data(filename);

  cout << "\nName: " << data->getName() << endl;
  cout << "Courses: " << data->getNumCourses() << endl;
  cout << "Rooms: " << data->getNumRooms() << endl;
  cout << "Days: " << data->getNumDays() << endl;
  cout << "Periods_per_day: " << data->getNumPeriodsPerDay() << endl;
  cout << "Curricula: " << data->getNumCurricula() << endl;
  cout << "Min_Max_Daily_Lectures: " << data->getMinDaily() << " "
       << data->getMaxDaily() << endl;
  cout << "UnavailabilityConstraints: "
       << data->getNumUnavailabilityConstraint() << endl;
  cout << "RoomConstraint (Feature): " << data->getNumRoomConstraints() << endl;

  int maximumPeriod = (data->getNumDays() * data->getNumPeriodsPerDay()) - 1;
  SM::numPeriods = maximumPeriod + 1;

  FeasibleTable ft(data->getCourses(), data->getRooms(), maximumPeriod,
                   data->getNumPeriodsPerDay(), data->getCurriculum());
  // std::clock_t t1,t2;
  // ft.printAdjMat();

  cout << endl << endl;

  cout << "Construction ...." << endl;
  double wall0 = SM::get_wall_time();
  double cpu0 = SM::get_cpu_time();
  ft.antColonyThread(8, 100);

  //auto fromOld = readCRT(data->getCourses(), data->getRooms(),
  //                       data->getNumPeriodsPerDay(), solfilename);
  // ft.setFeasibleTable(fromOld);
  cout << " Timetable has " << ft.NumberHCV()
       << " number of hard constraint violations" << endl;

  cout << endl << endl << "Improvement Phase..." << endl;

  ImproveTable impTable(ft.getVenueTime(), data->getRooms(), ft.getCurCodes(),
                        ft.getCourse(), ft.getFeasibleTable(),
                        ft.getMaxPeriod(), ft.getPeriodsInDay());
  // int max = ft.getFeasibleTable().size() - 1;
  // int min = 0;
  // int crtOneIndex = min + (rand() % (int)(max - min + 1));  //generate random
  // number in range [min max]
  // int crtTwoIndex = min + (rand() % (int)(max - min + 1));  //generate random
  // number in range [min max]
  // cout<<"Index One: "<<crtOneIndex<<" Index Two: "<<crtTwoIndex<<endl;
  // auto timet = impTable.singleMove(10, 180, ft.getFeasibleTable());
  //auto timet= fromOld;
  auto fromOld = ft.getFeasibleTable();
  for (int j =0; j < 10; j++)
  {
      for (size_t i =0; i < ft.getFeasibleTable().size(); i++)
      {
              cout<<"Iteration "<<i<<": ";
              auto timet = impTable.bestNeighbour((int)i, ft.getFeasibleTable());
              auto currentSCV = impTable.NumberSCV(timet);
              std::cout<< currentSCV<<" is the current SCV"<<endl;
              if (currentSCV < impTable.NumberSCV(fromOld))
              {
                      //cout<<" I am here "<<impTable.NumberSCV(fromOld)<<endl;
                      fromOld = timet;
                      ft.setFeasibleTable(fromOld);
                      //timet = fromOld;
              }
      }
      cout<<"number of violations in iteration "<<j << " is "<<
             impTable.NumberSCV(ft.getFeasibleTable());
  }

  // vector<int> chromosome;
  // auto timet = impTable.applyChomosome(chromosome, ft.getFeasibleTable());
  // auto timet = impTable.bestNeighbour(10, ft.getFeasibleTable());
  // auto newT = timet;

  //auto newT = impTable.runImprovement(10, 3, 1);
  auto newT = ft.getFeasibleTable();
  std::stringstream ss;
  ss << "Solutions/" << data->getName();
  impTable.writeTimetableToFileSolutionFormat(ss.str(), newT);

  double wall6 = SM::get_wall_time();
  double cpu6 = SM::get_cpu_time();
  std::cout << endl
            << "Full Timetable construction " << wall6 - wall0
            << " wall seconds and " << cpu6 - cpu0 << " CPU seconds" << endl;

  int roomStabilityViolations = 0;
  int courseWorkingDayViolations = 0;
  int consecutiveLectureViolations = 0;
  int sizeViolations = 0;

  cout << endl
       << endl
       << "***********************Improved Timetable2*****************" << endl;
  cout << endl
       << endl
       << " New Timetable has " << ft.NumberHCV(newT)
       << " number of hard constraint violations" << endl;
  cout << " New Timetable has "
       << impTable.NumberSCV(newT, &roomStabilityViolations,
                             &courseWorkingDayViolations,
                             &consecutiveLectureViolations, &sizeViolations)
       << " number of soft constraint violations" << endl;
  cout << "Room Size Violations: " << sizeViolations
       << " Room Stability: " << roomStabilityViolations
       << " Course Working Day Violations: " << courseWorkingDayViolations
       << " Consecutive Lecture Violations: " << consecutiveLectureViolations
       << endl;

  cout << endl << "**************Old Timetable**********************" << endl;
  cout << " Timetable has " << ft.NumberHCV()
       << " number of hard constraint violations" << endl;
  cout << " Old Timetable has "
       << impTable.NumberSCV(ft.getFeasibleTable(), &roomStabilityViolations,
                             &courseWorkingDayViolations,
                             &consecutiveLectureViolations, &sizeViolations)
       << " number of soft constraint violations" << endl;
  cout << "Room Size Violations: " << sizeViolations
       << " Room Stability: " << roomStabilityViolations
       << " Course Working Day Violations: " << courseWorkingDayViolations
       << " Consecutive Lecture Violations: " << consecutiveLectureViolations
       << endl;

  cout << "Threaded HCV: " << SM::ThreadNumberHCV(newT, (maximumPeriod + 1));
  impTable.writeTimetableToFile("ImproveTab.csv");

  /*
  CourseRoomTime crt = impTable.voteOut(0, ft.getFeasibleTable());
  auto invitations = impTable.invites(crt, ft.getFeasibleTable());
  auto invited = impTable.inviteMinIn(invitations, ft.getFeasibleTable());
  cout<<endl<<endl<<"Voted out "<<crt.toString()<<" best invite is
  "<<invited<<endl;
  //impTable.writeTimetableToFile2();
  auto newT = impTable.applyInvite(invited, crt, ft.getFeasibleTable());
  cout<<endl<<endl<<" New Timetable has "<<ft.NumberHCV(newT)<<" number of hard
  constraint violations"<<endl;
  cout<<" New Timetable has "<<impTable.NumberSCV(newT)<<" number of hard
  constraint violations"<<endl;
  */

  impTable.writeTimetableToFile2();

  delete data;
  // cout<<"Maximum period is: "<<maximumPeriod;

  return 0;
}
