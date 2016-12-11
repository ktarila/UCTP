#include "data.h"
#include "ReadCRT.h"
#include "Schedule/smethods.h"
#include <string>
#include <sstream>
#include "Schedule/feasibletable.h"
#include "Schedule/improvetable.h"
#include "Schedule/enhancement.h"


#include "beagle/GA.hpp"
#include "Schedule/UCTPEvalOp.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <numeric>
#include <iostream>
#include <exception>

using namespace Beagle;
using namespace std;

int main(int argc, char** argv){
	const char* filename = "InputData/ITC-2007_ectt/comp12.ectt";
  const char* solfilename = "/home/patrick/work/UCTP-CPP/Results/comp12.sol";
  //const char* filename = "InputData/Test_ectt/toy.ectt";
  Data* data = new Data(filename);
  

  cout<<"\nName: " << data->getName()<<endl;
  cout<<"Courses: " << data->getNumCourses()<<endl;
  cout<<"Rooms: " << data->getNumRooms()<<endl;
  cout<<"Days: " << data->getNumDays()<<endl;
  cout<<"Periods_per_day: " << data->getNumPeriodsPerDay()<<endl;
  cout<<"Curricula: " << data->getNumCurricula()<<endl;
  cout<<"Min_Max_Daily_Lectures: " << data->getMinDaily() << " " <<data->getMaxDaily()<<endl;
  cout<<"UnavailabilityConstraints: " << data->getNumUnavailabilityConstraint()<<endl;
  cout<<"RoomConstraint (Feature): " << data->getNumRoomConstraints()<<endl;





  int maximumPeriod = (data->getNumDays() * data->getNumPeriodsPerDay()) - 1;
  SM::numPeriods = maximumPeriod + 1;

  FeasibleTable ft(data->getCourses(),  data->getRooms(), maximumPeriod,  data->getNumPeriodsPerDay(), data->getCurriculum());

  cout<<endl<<endl;

  cout<<"Construction ...."<<endl;
  //double wall0 =  SM::get_wall_time();
  //double cpu0  = SM::get_cpu_time();
  //ft.antColonyThread(8, 500);
  auto fromOld = readCRT(data->getCourses(), data->getRooms(), data->getNumPeriodsPerDay(), solfilename);
  ft.setFeasibleTable(fromOld);
  ImproveTable ite(ft.getVenueTime(), data->getRooms(), ft.getCurCodes(), ft.getCourse(), ft.getFeasibleTable(), ft.getMaxPeriod(), ft.getPeriodsInDay());

  

   //improve timetable static constant
  //auto timet = ite.runImprovement(2, 1, 1);
  //ft.setFeasibleTable(timet);
  cout<<" Timetable of size: "<< fromOld.size() <<" has "<<ft.NumberHCV()<<" number of hard constraint violations and "<<ite.NumberSCV(fromOld)<<" soft constraint violations "<<endl;
  


  //Enhancement en(ft); //constructor
  //auto timet = en.runEnhancement(10, 4, 1);
  //ft.setFeasibleTable(timet);
  //auto fSchedule = ft.getFeasibleTable();
  //cout<<" Timetable has "<<ft.NumberHCV()<<" number of hard constraint violations and SCV: "<<ite.NumberSCV(timet) <<endl;

   //feasible timetable constraints static constant
  //SM::iT = ft;
  //SM::iiT = ite;




  cout<<endl<<endl<<"Improvement Phase..."<<endl;
  for (int j =0; j < 20; j++)
  {
  try {
   //ImproveTable ite(ft.getVenueTime(), data->getRooms(), ft.getCurCodes(), ft.getCourse(), ft.getFeasibleTable(), ft.getMaxPeriod(), ft.getPeriodsInDay());

   //improve timetable static constant
  //auto timet = ite.runImprovement(10, 1, 1);
  //Enhancement en(ft); //constructor
  //auto timet = en.runEnhancement(6, 2, 1);
  //ft.setFeasibleTable(timet);
    //cout<<" Timetable has "<<ft.NumberHCV()<<" number of hard constraint violations"<<endl;
  //auto fSchedule = ft.getFeasibleTable();

   //feasible timetable constraints static constant
  SM::iT = ft;
  SM::iiT = ite;
    // 1. Build the system.
    System::Handle lSystem = new System;
    // 2. Build evaluation operator.
    UCTPEvalOp::Handle lEvalOp = new UCTPEvalOp;
    // 3. Instanciate the evolver and the vivarium for float vectors GA population.
    GA::IntegerVector::Alloc::Handle lFVAlloc = new GA::IntegerVector::Alloc;
    FitnessSimpleMin::Alloc::Handle lFitnessAlloc = new FitnessSimpleMin::Alloc;
    Vivarium::Handle lVivarium = new Vivarium(lFVAlloc, lFitnessAlloc);
    // 4. Set representation, float vectors of 5 values.
    const unsigned int lVectorSize=100;  
    // 5. Initialize the evolver and evolve the vivarium.
    GA::EvolverIntegerVector::Handle lEvolver = new GA::EvolverIntegerVector(lEvalOp, lVectorSize);

    
    lEvolver->initialize(lSystem, argc, argv);

    lEvolver->evolve(lVivarium);


    //get Best --- It is located in the hall of fame
    Beagle::HallOfFame hof = lVivarium->getHallOfFame();
    //hofName = hof[0].mIndividual->serialize();  //print individual --- this is in xml

    Individual x = *hof[0].mIndividual; //Get individual object, pointer to individual allocator, could be more than one
                                        // sort individuals before getting best

    Beagle::GA::IntegerVector::Handle vecto = castHandleT<GA::IntegerVector>(x[0]); //get vector representation of best individual
    std::vector<int> chrome; 
    for(unsigned int i=0; i<lVectorSize; ++i) 
    {
      //int lXi = (*vecto)[i];
      chrome.push_back((*vecto)[i]);
      //std::cout<<lXi<<std::endl;
    }

    UCTPEvalOp ev;
    auto finalSche = ev.applyChromosome(chrome,  SM::iT.getFeasibleTable());
    std::stringstream ss;
    ss << "Solutions/genKempe-"<<j<<"-" << data->getName();
    ite.writeTimetableToFileSolutionFormat(ss.str(), finalSche);


    int roomStabilityViolations =   0;
    int courseWorkingDayViolations =   0;
    int consecutiveLectureViolations =   0;
    int sizeViolations = 0;

    cout<<endl<<endl<<"***********************Improved Genetic Timetable2*****************"<<endl;
    cout<<endl<<endl<<" New Timetable has "<<ft.NumberHCV(finalSche)<<" number of hard constraint violations"<<endl;
    cout<<" New Timetable has "<<ite.NumberSCV(finalSche,&roomStabilityViolations, &courseWorkingDayViolations, &consecutiveLectureViolations, &sizeViolations)<<" number of soft constraint violations"<<endl;
    cout<<"Room Size Violations: "<<sizeViolations<<" Room Stability: " << roomStabilityViolations << " Course Working Day Violations: " << courseWorkingDayViolations << " Consecutive Lecture Violations: " << consecutiveLectureViolations<<endl;
    ft.setFeasibleTable(finalSche);
    //SM::iT = ft;
    
  }
  catch(Exception& inException) {
    inException.terminate(std::cerr);
  }
  catch(std::exception& inException) {
    std::cerr << "Standard exception catched:" << std::endl << std::flush;
    std::cerr << inException.what() << std::endl << std::flush;
    return 1;
  }
}
  //std::cout<<hofName<<std::endl;
  return 0;
}
