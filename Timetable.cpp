// reading a text file
#include "ReadCRT.h"
#include "Schedule/enhancement.h"
#include "Schedule/feasibletable.h"
#include "Schedule/improvetable.h"
#include "Schedule/smethods.h"
#include "data.h"
#include <cstdlib>
#include <sstream>
#include <string>

using namespace std;

int main(int argc, char *argv[]) {
  srand(time(NULL));
  int numAnts = 8;
  int numGens = 1000;

  if (argc != 4) { // argc should be 4 for correct execution
    // We print argv[0] assuming it is the program name
    cout << "usage: " << argv[0]
         << " <comp01> <number_of_ants> <number_of_iterations>\n";
    return 0;
  }
  numAnts = atoi(argv[2]);
  numGens = atoi(argv[3]);

  std::stringstream filename;
  filename << "InputData/ITC-2007_ectt/" << argv[1] << ".ectt";
  // const char* filename = "/home/patrick/work/ndutoectt/timetable.ectt";
  std::stringstream solfilenamestream;
  solfilenamestream << "/home/patrick/work/UCTP-CPP/Results/" << argv[1]
                    << ".sol";
  // const char* filename = "InputData/Test_ectt/toy.ectt";
  char *inputfilepath = new char[filename.str().length() + 1];
  filename >> inputfilepath;

  char *solfilename = new char[solfilenamestream.str().length() + 1];
  solfilenamestream >> solfilename;
  Data *data = new Data(inputfilepath);

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
  // double wall0 = SM::get_wall_time();
  // double cpu0 = SM::get_cpu_time();
  ft.antColonyThread(numAnts, numGens);

  // ft.antColony(1, 1);

  // auto fromOld = readCRT(data->getCourses(), data->getRooms(),
  //                       data->getNumPeriodsPerDay(), solfilename);
  // ft.setFeasibleTable(fromOld);
  // cout << " Timetable has " << ft.NumberHCV()
  //      << " number of hard constraint violations" << endl;

  // cout << endl << endl << "Improvement Phase..." << endl;

  // int max = ft.getFeasibleTable().size() - 1;
  // int min = 0;
  // int crtOneIndex = min + (rand() % (int)(max - min + 1));  //generate random
  // number in range [min max]
  // int crtTwoIndex = min + (rand() % (int)(max - min + 1));  //generate random
  // number in range [min max]
  // cout<<"Index One: "<<crtOneIndex<<" Index Two: "<<crtTwoIndex<<endl;
  // auto timet = impTable.singleMove(10, 180, ft.getFeasibleTable());
  // auto timet= fromOld;
  // auto fromOld = ft.getFeasibleTable();
  // ImproveTable impTable(ft.getVenueTime(), data->getRooms(), ft.getCurCodes(),
  //                       ft.getCourse(), ft.getFeasibleTable(),
  //                       ft.getMaxPeriod(), ft.getPeriodsInDay());
  //   // for (size_t i = 0; i < ft.getFeasibleTable().size(); i++) {
    //   cout << "Iteration " << i << ": ";
    //   auto timet = impTable.bestNeighbour((int)i, ft.getFeasibleTable());
    //   auto currentSCV = impTable.NumberSCV(timet);
    //   std::cout << currentSCV << " is the current SCV" << endl;
    //   if (currentSCV < impTable.NumberSCV(fromOld)) {
    //     // cout<<" I am here "<<impTable.NumberSCV(fromOld)<<endl;
    //     fromOld = timet;
    //     ft.setFeasibleTable(fromOld);
    //     // timet = fromOld;
    //   }
    // }
    // cout << "number of violations in iteration is "
    //      << impTable.NumberSCV(ft.getFeasibleTable());

  // vector<int> chromosome;
  // auto timet = impTable.applyChomosome(chromosome, ft.getFeasibleTable());
  // auto timet = impTable.bestNeighbour(10, ft.getFeasibleTable());
  // auto newT = timet;

  //
  // enhance table by moves
  // Enhancement en(ft); //constructor
  // auto newT = en.runEnhancement(4, 3, 1);
  // ft.setFeasibleTable(newT);

  // final improvement with kempe swaps

  // impTable.antColonySoft(8, 3, 3, true);
  // auto newT = impTable.runImprovement(10, 3, 3);
  // auto newT = impTable.antColonySoftThread(8, 50, const int &num_ber, bool
  // accept) runImprovement(10, 3, 3);
  // ft.setFeasibleTable(timet);
  // auto newT = ft.getFeasibleTable();
  // std::stringstream ss;
  // ss << "Solutions/" << data->getName();
  // ss << "/home/patrick/work/ndutoectt/" << data->getName();
  cout << "Improvement phase "<< endl;
  ImproveTable finalTable(ft.getVenueTime(), data->getRooms(), ft.getCurCodes(),
                        ft.getCourse(), ft.getFeasibleTable(),
                        ft.getMaxPeriod(), ft.getPeriodsInDay());
  auto newT = finalTable.runImprovement(1, 3, 1);
  // auto newT = finalTable.antColonySoftThread(3, 30);
  finalTable.writeTimetableToFileSolutionFormat(solfilename, newT);

  // double wall6 = SM::get_wall_time();
  // double cpu6 = SM::get_cpu_time();
  // std::cout << endl
  //           << "Full Timetable construction " << wall6 - wall0
  //           << " wall seconds and " << cpu6 - cpu0 << " CPU seconds" << endl;

  int roomStabilityViolations = 0;
  int courseWorkingDayViolations = 0;
  int consecutiveLectureViolations = 0;
  int sizeViolations = 0;

  cout << endl
       << endl
       << "***********************Improved Timetable2*****************" <<
       endl;
  cout << endl
       << endl
       << " New Timetable has " << ft.NumberHCV(newT)
       << " number of hard constraint violations" << endl;
  cout << " New Timetable has "
       << finalTable.NumberSCV(newT, &roomStabilityViolations,
                             &courseWorkingDayViolations,
                             &consecutiveLectureViolations, &sizeViolations)
       << " number of soft constraint violations" << endl;
  cout << "Room Size Violations: " << sizeViolations
       << " Room Stability: " << roomStabilityViolations
       << " Course Working Day Violations: " << courseWorkingDayViolations
       << " Consecutive Lecture Violations: " << consecutiveLectureViolations
       << endl;

  cout << endl << "**************Old Timetable**********************" <<
  endl; cout << " Timetable has " << ft.NumberHCV()
       << " number of hard constraint violations" << endl;
  cout << " Old Timetable has "
       << finalTable.NumberSCV(ft.getFeasibleTable(), &roomStabilityViolations,
                             &courseWorkingDayViolations,
                             &consecutiveLectureViolations, &sizeViolations)
       << " number of soft constraint violations" << endl;
  cout << "Room Size Violations: " << sizeViolations
       << " Room Stability: " << roomStabilityViolations
       << " Course Working Day Violations: " << courseWorkingDayViolations
       << " Consecutive Lecture Violations: " << consecutiveLectureViolations
       << endl;

  // cout << "Threaded HCV: " << SM::ThreadNumberHCV(newT, (maximumPeriod + 1));
  // impTable.writeTimetableToFile("ImproveTab.csv");

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

  // impTable.writeTimetableToFile2();

  delete data;
  // cout<<"Maximum period is: "<<maximumPeriod;

  return 0;
}
