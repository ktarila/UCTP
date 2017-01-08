
#include <mpi/mpi.h> /* Important include mpi library befor extern C so mpi.h is not included again in gaul.h which causes
                        a conflict as it shouldn't be in extern "C" */

#include "ReadCRT.h"
#include "data.h"
#include "Schedule/smethods.h"
#include "Schedule/feasibletable.h"
#include "Schedule/enhancement.h"
#include "Schedule/improvetable.h"
#include "Schedule/path.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <iostream>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <time.h> 

extern "C" {
#include "gaul.h"
}


using namespace std;

long evaluation_count=0;  /* Number of fitness evaluations performed. */
int scale = 1000000; /* Linear scale fitness value */
boolean accept; 
vector<CourseRoomTime> applyAllele(const int &allele,
                                   const vector<CourseRoomTime> &oldSchedule) {
  int fSize = SM::iT.getFeasibleTable().size();
  int pSize = SM::iiT.getMaxPeriod() + 1;
  int oldSCV = SM::iiT.NumberSCV(oldSchedule);

  int total = fSize + pSize;
  // int total = fSize;

  int firstCRT = allele / total;
  int secondCRT = allele % total;

  // cout<<fSize<<" - Table Size "<< vtSize <<" - VenueTime Size"<<endl;
  // cout<<"allele: " << allele << ": first: "<<firstCRT <<" Second: "
  // <<secondCRT <<" Total: " << total <<endl;
  if (secondCRT < fSize) {
    // simple swap first and second swapRTs(firstCRT, secondCRT)
    // cout<<"\t\tSwapping CRT index "<<firstCRT << " CRT index "<< secondCRT
    // <<endl;
    auto swapped = SM::iiT.singleSwap(firstCRT, secondCRT, oldSchedule);
    int currentSCV = SM::iiT.NumberSCV(swapped);
    if (currentSCV <= oldSCV) return swapped;
  } else if (secondCRT >= fSize) {
    int row = firstCRT * total;
    int period = allele - row - fSize;

    // cout<<"\t\tMoving CRTIndex "<<firstCRT << " to period "<< period <<endl;
    auto moved = SM::iiT.singleMove(firstCRT, period, oldSchedule);
    int currentSCV = SM::iiT.NumberSCV(moved);
    if (currentSCV <= oldSCV) return moved;
  }
  // cout<<" nothing "<<endl;
  return oldSchedule;
}

/**********************************************************************
  explode()
  synopsis: Explode a string.
  parameters:
  return:
  updated:  20 Sep 2004
 **********************************************************************/
vector<string> explode(const string& str, const char& ch) {
    string next;
    vector<string> result;

    // For each character in the string
    for (string::const_iterator it = str.begin(); it != str.end(); it++) {
        // If we've hit the terminal character
        if (*it == ch) {
            // If we have some characters accumulated
            if (!next.empty()) {
                // Add them to the result vector
                result.push_back(next);
                next.clear();
            }
        } else {
            // Accumulate the next character into the sequence
            next += *it;
        }
    }
    if (!next.empty())
         result.push_back(next);
    return result;
}

/**********************************************************************
  applyChromosome()
  synopsis: The resulting timetabling schedule when chromosome is applied.
  parameters:
  return:
  updated:  20 Sep 2004
 **********************************************************************/
static vector<CourseRoomTime> applyChromosome(std::vector<string> bestChromosome) {
  vector<CourseRoomTime> tSchedule = SM::iT.getFeasibleTable();
  std::vector<int> sequence;

  for (size_t i = 0; i < bestChromosome.size(); i++) {
    //std::cout << "\"" << bestChromosome[i] << "\"" << endl;
    sequence.push_back(std::stoi( bestChromosome[i] ));
  }
  auto tempSchedule  = SM::enhance.applyEnhancementSequence(sequence, tSchedule, accept);
  return tempSchedule;
}


/**********************************************************************
  softconstraint_score()
  synopsis: Evaluation / fitness function to minimize soft constraints.
  parameters:
  return:
  updated:  20 Sep 2004
 **********************************************************************/
static boolean softconstraint_score(population *pop, entity *entity) {
  int k;

  entity->fitness = 0.0;

  //double lSum = 10.0;
  vector<CourseRoomTime> tSchedule = SM::iT.getFeasibleTable();
  vector<CourseRoomTime> tempSchedule;
  std::vector<int> sequence;

  /* Loop over alleles in chromosome. */
  for (k = 0; k < pop->len_chromosomes; k++)
  {
    int allele = ((int *)entity->chromosome[0])[k];
    sequence.push_back(allele);
    //entity->fitness += (5-allele)*(5-allele);
    //tempSchedule = applyAllele(allele, tSchedule);
    //tempSchedule = SM::iiT.bestNeighbour(allele, tSchedule);
    //tSchedule = tempSchedule;
  }

  tempSchedule  = SM::enhance.applyEnhancementSequence(sequence, tSchedule, accept);

  auto quality = (double) SM::iiT.NumberSCV(tempSchedule);
  entity->fitness = quality;

/* Normalize fitness so smaller positive values are better. */
  entity->fitness = 1.0/(1.0+entity->fitness);

  //linear scale fitness
  entity->fitness = scale * entity->fitness;

  evaluation_count++;
  printf("Evaluation number = %ld is concluded with quality %f \n", evaluation_count, quality);
  return TRUE;

  
  /*for (k = 0; k < pop->len_chromosomes; k++) {
    int allele = ((int *)entity->chromosome[0])[k];
    //cout<<"I am here evaluation allele is: "<< allele <<"k is:"<<k<<endl;
    //tempSchedule = applyAllele(allele, tSchedule);
    // tempSchedule = SM::iiT.bestNeighbour(allele, tSchedule);
    //tSchedule = tempSchedule;
  }

  //lSum = SM::iiT.NumberSCV(tSchedule);

  lSum = lSum *-1;  // convert to negative value to minimize

  entity->fitness = lSum;

  evaluation_count++;
  cout<<"evolution: "<<evaluation_count<<" fitness: "<<lSum<<endl;
  return TRUE;*/
}

/**********************************************************************
  struggle_generation_hook()
  synopsis: This function is called by the main GA routine at the
    beginning of every generation.
  parameters:
  return:
  updated:  07/07/01
 **********************************************************************/

static boolean struggle_generation_hook(int generation, population *pop)
  {
  static double sum_best_fitnesses=0.0; /* Sum of best fitness score at each generation. */
  double  average, stddev;  /* Simple stats. */

  sum_best_fitnesses += ga_get_entity_from_rank(pop,0)->fitness;

/*
 * Display statistics every 20th generation.
 */
  if (generation%1 == 0)
    {
    printf("Generation = %d\n", generation);
    printf("Number of evaluations = %ld\n", evaluation_count);
    double eval = (scale - ga_get_entity_from_rank(pop,0)->fitness)/ga_get_entity_from_rank(pop,0)->fitness;
    printf("Best fitness = %f\n", ga_get_entity_from_rank(pop,0)->fitness);
    printf("Best Evaluation = %f\n", eval);
    ga_fitness_mean_stddev(pop, &average, &stddev);
    printf("Mean fitness = %f, with standard deviation = %f\n", average, stddev);
    if (generation>0)
      printf("Average best fitness for entire run = %f\n", sum_best_fitnesses/generation);
    }

/*
 * Stop if we have the exact solution.
 */
  if (ga_get_entity_from_rank(pop,0)->fitness == 0)
    {
    printf("Exact solution has been found!\n");
    return FALSE;
    }

  return TRUE;  /* TRUE indicates that evolution should continue. */
  }



/**********************************************************************
  main()
  synopsis:
  parameters:
  return:
  updated:  17 Feb 2005
 **********************************************************************/

int main(int argc, char **argv) {
  const char *filename = "InputData/ITC-2007_ectt/comp12.ectt";
  const char *solfilename = "/home/patrick/work/UCTP-CPP/Results/comp12.sol";
  // const char* filename = "InputData/Test_ectt/toy.ectt";
  Data *data = new Data(filename);

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

  cout << endl << endl;

  cout << "Construction ...." << endl;
  double wall0 =  SM::get_wall_time();
  double cpu0  = SM::get_cpu_time();
  ft.antColonyThread(1, 1);
  auto fromOld = readCRT(data->getCourses(), data->getRooms(),
                         data->getNumPeriodsPerDay(), solfilename);
  ft.setFeasibleTable(fromOld);
  double wall6 = SM::get_wall_time();
  double cpu6 = SM::get_cpu_time();
  std::cout << endl
            << "Full Timetable construction " << wall6 - wall0
            << " wall seconds and " << cpu6 - cpu0 << " CPU seconds" << endl;
  ImproveTable ite(ft.getVenueTime(), data->getRooms(), ft.getCurCodes(),
                   ft.getCourse(), ft.getFeasibleTable(), ft.getMaxPeriod(),
                   ft.getPeriodsInDay());

  // improve timetable static constant
  //auto timet = ite.runImprovement(2, 1, 1);
  //ft.setFeasibleTable(timet);
  //auto fromOld = ft.getFeasibleTable();
  cout << " Timetable of size: " << fromOld.size() << " has " << ft.NumberHCV()
       << " number of hard constraint violations and " << ite.NumberSCV(fromOld)
       << " soft constraint violations " << endl;

  //Enhancement en(ft); //constructor
  //auto timet = en.runEnhancement(4, 3, 1);
  //ft.setFeasibleTable(timet);
  // auto fSchedule = ft.getFeasibleTable();
  //cout<<" Timetable has "<<ft.NumberHCV()<<" number of hard constraint violations and SCV: "<<ite.NumberSCV(timet) <<endl;

  // feasible timetable constraints static constant
   SM::iT = ft;
   SM::iiT = ite;

   Enhancement en(ft); //constructor
   SM::enhance = en;


  /*
  *  Improve on soft constraints using genetic algorithms
  */
  
  for (int i = 0; i < 20; i++) {
    cout << endl << endl << "Improvement Phase: Iteration:  " << i << endl;
    if (i%2 == 0){
       accept = true;
    }else {
      accept = false;
    }
    population* pop = NULL;  /* Population of solutions. */
    char* beststring = NULL; /* Human readable form of best solution. */
    size_t beststrlen = 0;   /* Length of beststring. */

    int randSeed = time(NULL);
    random_seed(randSeed);
    // int max = (SM::iT.getFeasibleTable().size() *
    //           (SM::iT.getFeasibleTable().size() + SM::iiT.getMaxPeriod() +
    //           1)) -
    //          1;

    int max = fromOld.size() - 1;
    //int max = 3;
    // int chromLength = fromOld.size();
    cout << "Maximum: " << max << endl;

    // int max = 10;
    pop = ga_genesis_integer(
      5,                       /* const int              population_size */
      1,                        /* const int              num_chromo */
      max,                      /* const int              len_chromo */
      struggle_generation_hook, /* GAgeneration_hook      generation_hook */
      NULL,                     /* GAiteration_hook       iteration_hook */
      NULL,                     /* GAdata_destructor      data_destructor */
      NULL,                   /* GAdata_ref_incrementor data_ref_incrementor */
      softconstraint_score,   /* GAevaluate             evaluate */
      ga_seed_integer_random, /* GAseed                 seed */
      NULL,                   /* GAadapt                adapt */
      ga_select_one_roulette,      /* GAselect_one           select_one */
      ga_select_two_roulette,      /* GAselect_two           select_two */
      ga_mutate_integer_singlepoint_drift, /* GAmutate               mutate */
      ga_crossover_integer_singlepoints, /* GAcrossover            crossover */
      NULL,                              /* GAreplace   replace */
      NULL                               /* vpointer    User data */
      );

    ga_population_set_allele_min_integer(pop, 0);
    ga_population_set_allele_max_integer(pop, max);

    ga_population_set_parameters(
      pop,                        /* population      *pop */
      GA_SCHEME_DARWIN,           /* const ga_scheme_type  scheme */
      GA_ELITISM_PARENTS_SURVIVE, /* const ga_elitism_type   elitism */
      0.8,                        /* double    crossover */
      0.05,                       /* double    mutation */
      0.0                         /* double    migration */
      );

    ga_evolution(pop, /* population              *pop */
                 0   /* const int               max_generations */
                 );

    /* Display final solution. */
    //printf("The final solution was:\n");
    beststring = ga_chromosome_integer_to_string(
      pop, ga_get_entity_from_rank(pop, 0), beststring, &beststrlen);
    /*printf("%s\n", beststring);
    printf("With score = %f\n", ga_get_entity_from_rank(pop, 0)->fitness);
    double eval = (1 - ga_get_entity_from_rank(pop, 0)->fitness) /
                  ga_get_entity_from_rank(pop, 0)->fitness;
    printf("And evaluation quality = %f\n", eval / scale);*/

    std::vector<std::string> result = explode(beststring, ' ');
    auto currentSchedule = applyChromosome(result);
    SM::iT.setFeasibleTable(currentSchedule);
    SM::enhance = SM::iT;

    cout << " Timetable of size: " << currentSchedule.size() << " has " << SM::iT.NumberHCV()
       << " number of hard constraint violations and " << ite.NumberSCV(SM::iT.getFeasibleTable())
       << " soft constraint violations " << endl;

    /* Free memory. */
    ga_extinction(pop);
    s_free(beststring);
  }
  exit(EXIT_SUCCESS);
}
