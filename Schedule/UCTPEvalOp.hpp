
 
#ifndef UCTPEvalOpEvalOp_hpp
#define UCTPEvalOpEvalOp_hpp

#include "beagle/GA.hpp"
#include <stdexcept>
#include <iostream>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>
#include "feasibletable.h"
#include "path.h"
#include "roompath.h"
#include <mutex>
#include <chrono>


/*!
 *  \class UCTPEvalOpEvalOp UCTPEvalOpEvalOp.hpp "UCTPEvalOpEvalOp.hpp"
 *  \brief The individual evaluation class operator for the problem of real-valued GA
 *    function maximisation.
 *  \ingroup UCTPEvalOp
 */
class UCTPEvalOp : public Beagle::EvaluationOp {

private:
  vector<CourseRoomTime> applyAllele(const int& allele, const vector<CourseRoomTime>& oldSchedule);
  //CourseRoomTime moveToBest(int crtIndex, vector<CourseRoomTime>* timetable, bool accept );
  vector<CourseRoomTime> getClashList(const vector<CourseRoomTime>& timetable, const CourseRoomTime& crt);
  //CourseRoomTime moveToBest2(int crtIndex, vector<CourseRoomTime>* timetable );
  vector<CourseRoomTime> getCRTsinPeriod(const int& period, const vector<CourseRoomTime>& timetable);
  

public:

  //! UCTPEvalOpEvalOp allocator type.
  typedef Beagle::AllocatorT<UCTPEvalOp,Beagle::EvaluationOp::Alloc>
          Alloc;
  //!< UCTPEvalOpEvalOp handle type.
  typedef Beagle::PointerT<UCTPEvalOp,Beagle::EvaluationOp::Handle>
          Handle;
  //!< UCTPEvalOpEvalOp bag type.
  typedef Beagle::ContainerT<UCTPEvalOp,Beagle::EvaluationOp::Bag>
          Bag;

  explicit UCTPEvalOp();

  virtual Beagle::Fitness::Handle evaluate(Beagle::Individual& inIndividual,
                                           Beagle::Context& ioContext);

  virtual void initialize(Beagle::System& ioSystem);
  vector<CourseRoomTime> applyChromosome(vector<int> chromosome,  vector<CourseRoomTime> timetable);

protected:

  Beagle::UInt::Handle   mIntMaxGen;    //!< Individual integer vectors size.
  Beagle::IntArray::Handle   mIntMaxValue;    //!< Individual integer vectors size.
  Beagle::IntArray::Handle  mMaxInitValue;   //!< Maximum value used to initialize integers.
  Beagle::IntArray::Handle  mMinInitValue;   //!< Minimum value used to initialize integers.

};

#endif // UCTPEvalOp_hpp
