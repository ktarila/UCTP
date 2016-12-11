
#include "beagle/GA.hpp"
#include "UCTPEvalOp.hpp"
#include "smethods.h"
#include "path.h"

#include <cmath>
#include <algorithm>

using namespace Beagle;
using namespace std;

/*!
 *  \brief Construct the individual evaluation operator for maximizing the function.
 */
UCTPEvalOp::UCTPEvalOp() :
  EvaluationOp("UCTPEvalOp")
{ 

}


/*!
 *  \brief Evaluate the fitness of the given individual.
 *  \param inIndividual Current individual to evaluate.
 *  \param ioContext Evolutionary context.
 *  \return Handle to the fitness value of the individual.
 */
Fitness::Handle UCTPEvalOp::evaluate(Individual& inIndividual, Context& ioContext)
{
  Beagle_AssertM(inIndividual.size() == 1);
  GA::IntegerVector::Handle lIntegerVector = castHandleT<GA::IntegerVector>(inIndividual[0]);

  //Beagle_AssertM(lIntegerVector->size() == 100);

  double lSum = 0.0;
  vector<CourseRoomTime> tSchedule = SM::iT.getFeasibleTable();
  vector<CourseRoomTime> tempSchedule;
  
  for(unsigned int i=0; i<lIntegerVector->size(); ++i) 
  {
    int allele = (*lIntegerVector)[i];
    tempSchedule = this->applyAllele(allele, tSchedule);
    //tempSchedule = SM::iiT.bestNeighbour(allele, tSchedule);
    tSchedule = tempSchedule;
  }
  lSum = SM::iiT.NumberSCV(tSchedule);
  return new FitnessSimpleMin(lSum);
}

vector<CourseRoomTime> UCTPEvalOp::applyAllele(const int& allele, const vector<CourseRoomTime>& oldSchedule)
{

  int fSize = SM::iT.getFeasibleTable().size();
  int pSize = SM::iiT.getMaxPeriod() + 1;
  int oldSCV = SM::iiT.NumberSCV(oldSchedule);


  int total = fSize + pSize;
  //int total = fSize;

  int firstCRT = allele / total;
  int secondCRT = allele % total;

  //cout<<fSize<<" - Table Size "<< vtSize <<" - VenueTime Size"<<endl;
  //cout<<"allele: " << allele << ": first: "<<firstCRT <<" Second: " <<secondCRT <<" Total: " << total <<endl;
  if ( secondCRT < fSize)
  {
    //simple swap first and second swapRTs(firstCRT, secondCRT)
    //cout<<"\t\tSwapping CRT index "<<firstCRT << " CRT index "<< secondCRT <<endl;
    auto swapped = SM::iiT.singleSwap(firstCRT, secondCRT,  oldSchedule);
    int currentSCV = SM::iiT.NumberSCV(swapped);
    if (currentSCV <= oldSCV)
      return swapped;
  }
  else if (secondCRT >= fSize)
  {
    int row = firstCRT * total;
    int period = allele - row - fSize;

    //cout<<"\t\tMoving CRTIndex "<<firstCRT << " to period "<< period <<endl;
    auto moved = SM::iiT.singleMove(firstCRT, period,  oldSchedule);
    int currentSCV = SM::iiT.NumberSCV(moved);
    if (currentSCV <= oldSCV)
      return moved;
  }
  //cout<<" nothing "<<endl;
  return oldSchedule;
}

vector<CourseRoomTime> UCTPEvalOp::applyChromosome(vector<int> chromosome,  vector<CourseRoomTime> timetable)
{
  vector<CourseRoomTime> tSchedule = timetable;
  vector<CourseRoomTime> tempSchedule;
  for (auto &allele:chromosome)
  {
    tempSchedule = this->applyAllele(allele, tSchedule);
    //tempSchedule = SM::iiT.bestNeighbour(allele, tSchedule);
    tSchedule = tempSchedule;
  }
  //cout <<change <<" Number of Improvements"<<endl;
  return tSchedule;
}

/*!
 *  \brief Initialize the TSP evaluation operator.
 *  \param ioSystem Evolutionary system.
 */
void UCTPEvalOp::initialize(Beagle::System& ioSystem)
{
  Beagle::EvaluationOp::initialize(ioSystem);

  
  int max = (SM::iT.getFeasibleTable().size() * (SM::iT.getFeasibleTable().size() + SM::iiT.getMaxPeriod() + 1 )) - 1;
  //int max = (SM::iT.getFeasibleTable().size() * (SM::iT.getFeasibleTable().size() )) - 1;
  cout<<max<<endl;
  if(ioSystem.getRegister().isRegistered("ec.term.maxgen")) {
    mIntMaxGen = castHandleT<UInt>(ioSystem.getRegister()["ec.term.maxgen"]);
  } else {
    mIntMaxGen = new UInt(10);
    Register::Description lDescription(
      "Maximum Number of generations",
      "UInt",
      "111",
      "Maximum number of generations."
    );
    ioSystem.getRegister().addEntry("ec.term.maxgen", mIntMaxGen, lDescription);
  }

/*
  if(ioSystem.getRegister().isRegistered("ga.int.maxvalue")) {
    mIntMaxValue = castHandleT<UInt>(ioSystem.getRegister()["ga.int.maxvalue"]);
  } else {
    mIntMaxValue = new UInt(5);
    Register::Description lDescription(
      "Maximum Allele Value",
      "UInt",
      "5",
      "Maximum Allele Value."
    );
    ioSystem.getRegister().addEntry("ga.int.maxvalue", mIntMaxValue, lDescription);
  }
  */
}

/*CourseRoomTime UCTPEvalOp::moveToBest(int crtIndex, vector<CourseRoomTime>* timetable, bool accept )
{
  CourseRoomTime c;
  auto fullSchedule = SM::iT.getFeasibleTable();
  CourseRoomTime crt = fullSchedule[crtIndex];
  auto tempSchedule = *timetable;
  unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
  double number = rand_r(&seed)/(double)RAND_MAX;
  //accept = false;

  std::vector<CourseRoomTime>::iterator findCRT = std::find(tempSchedule.begin(), tempSchedule.end(), crt);
  int indexInTS = std::distance(tempSchedule.begin(), findCRT);
  if (findCRT != tempSchedule.end())
  {
    int min = SM::iiT.NumberSCV(tempSchedule);
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
        SM::periodViolation(newTemp, SM::iiT.getMaxPeriod(), period1, &v1);
        SM::periodViolation(newTemp, SM::iiT.getMaxPeriod(), period2, &v2);
        Course c1 = crt.getCourse();
        Course c2 = swapCRT.getCourse();
        bool l = true;
        number = rand_r(&seed)/(double)RAND_MAX;
        if (v1 == 0 && v2 == 0 &&  l)
        {
          int nSCurrent = SM::iiT.NumberSCV(newTemp);
          if (nSCurrent < min)
          {
            //cout<<" best is swap "<<endl;
            min = nSCurrent;
            bestSchedule = newTemp;
          }*/
/*          if (nSCurrent == min && number > 0.5)
          {
            //cout<<" best is swap "<<endl;
            min = nSCurrent;
            bestSchedule = newTemp;
          }*/


        /*}

        //swap room
        newTemp = tempSchedule;
        RoomTime newcrtRoomTime = crt.getVenueTime();
        newcrtRoomTime.setRoom(swapCRT.getVenueTime().getRoom());
        RoomTime newswapRoomTime = swapCRT.getVenueTime();
        newcrtRoomTime.setRoom(crt.getVenueTime().getRoom());
        newTemp[indexInTS].setVenueTime(newcrtRoomTime);
        newTemp[swapIndex].setVenueTime(newswapRoomTime);

        SM::periodViolation(newTemp, SM::iiT.getMaxPeriod(), period1, &v1);
        SM::periodViolation(newTemp, SM::iiT.getMaxPeriod(), period2, &v2);
        if (v1 == 0 && v2 == 0)
        {
          int nSCurrent = SM::iiT.NumberSCV(newTemp);
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
        SM::periodViolation(newTemp, SM::iiT.getMaxPeriod(), period1, &v1);
        SM::periodViolation(newTemp, SM::iiT.getMaxPeriod(), period2, &v2);
        if (v1 == 0 && v2 == 0)
        {
          int nSCurrent = SM::iiT.NumberSCV(newTemp);
          if (nSCurrent < min)
          {
            //cout<<" best is swap "<<endl;
            min = nSCurrent;
            bestSchedule = newTemp;
          }

        }

      }
    }*/


    //bool moved = false;
    //move to period
/*    for (int i =0; i < SM::iiT.getMaxPeriod(); i++)
    {
      newTemp = tempSchedule;
      RoomTime roomtime_periodMove = crt.getVenueTime();
      roomtime_periodMove.setPeriod(i);
      newTemp[indexInTS].setVenueTime(roomtime_periodMove);
      int v1 = 0;
      SM::periodViolation(newTemp, SM::iiT.getMaxPeriod(), i, &v1);
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
      SM::periodViolation(newTemp, SM::iiT.getMaxPeriod(), roomtime_RoomMove.getPeriod(), &v1);
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
    /*auto venueTime = SM::iT.getVenueTime();
    for (std::size_t i =0; i < venueTime.size(); i++)
    {
      auto eventP = venueTime[i];
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
        SM::periodViolation(newTemp, SM::iiT.getMaxPeriod(), eventP.getPeriod(), &violations);
        if (violations == 0)
        {
          int nSCurrent = SM::iiT.NumberSCV(newTemp);
          if (nSCurrent <= min)
          {
            //moved = true;
            min = nSCurrent;
            bestSchedule = newTemp;
          }
          //number = rand_r(&seed)/(double)RAND_MAX;
          //if (nSCurrent == min && number > 0.5)
          //{
            //cout<<" best is swap "<<endl;
            //min = nSCurrent;
            //bestSchedule = newTemp;
          //}

        }
        else if ( violations > 0 && accept == true)
        {

          //cout<<"in violations more than zero"<<endl;
          //to do ---- check if it's a single CRT Violation and return it for next move
          auto clashList = this->getClashList(newTemp, newTemp[indexInTS]);
          //check if single clash
          */
/*          cout<<clashList.size()<<endl;
          for (auto &clasL:clashList)
          {
            cout<<clasL.toString()<<" ";
          }
          cout<<endl;
          */
         /* if (clashList.size() == 1)
          {
            //cout<<"clash size equal to 1"<<endl;
            auto copyNewTemp = newTemp;
            auto findCourse = std::find(copyNewTemp.begin(), copyNewTemp.end(), clashList[0]);
            auto cIndex = std::distance(copyNewTemp.begin(), findCourse);
            auto newC = this->moveToBest2(cIndex, &copyNewTemp);
            if (newC.empty == true)
            {
              //cout<<" Safely swapped "<<endl;
              int nSCurrent = SM::iiT.NumberSCV(copyNewTemp);
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
    }*/
    /*
    if (!moved)
      cout<<" Best is swap"<<endl;
    else
      cout<<" Best is move"<<endl;
    */
/*    *timetable = bestSchedule;
  }
  return c;
}
*/
vector<CourseRoomTime> UCTPEvalOp::getClashList(const vector<CourseRoomTime>& timetable, const CourseRoomTime& crt)
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

/*CourseRoomTime UCTPEvalOp::moveToBest2(int crtIndex, vector<CourseRoomTime>* timetable )
{
  //cout<<" in move to best 2"<<endl;
  CourseRoomTime c;
  auto t = *timetable;
  auto venueTime = SM::iT.getVenueTime();
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
    for (std::size_t i =0; i < venueTime.size(); i++)
    {
      auto eventP = venueTime[i];
      if (eventP.canAssignCourse(crt.getCourse()))
      {
        newTemp = tempSchedule;
        newTemp[indexInTS].setVenueTime(eventP);
        //check number of hard constraint violations in newTimetable CRT
        int violations = 0;
        SM::periodViolation(newTemp, SM::iiT.getMaxPeriod(), eventP.getPeriod(), &violations);
        if (violations == 0)
        {
          int nSCurrent = SM::iiT.NumberSCV(newTemp);
          if (nSCurrent <= min)
          {
            min = nSCurrent;
            bestSchedule = newTemp;
          }
          number = rand_r(&seed)/(double)RAND_MAX;*/
          /*if (nSCurrent == min && number > 0.5)
          {
            //cout<<" best is swap "<<endl;
            min = nSCurrent;
            bestSchedule = newTemp;
          }*/
/*          found++;

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
}*/


/**
 * Gets the subset of given timetable that occur on the specified period
 * @param period
 * @param timetable
 * @return ArrayList of CourseRoomTime objects from the timetable in the Specified period
 */

vector<CourseRoomTime> UCTPEvalOp::getCRTsinPeriod(const int& period, const vector<CourseRoomTime>& timetable)
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
