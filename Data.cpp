

#include "data.h"
//#include "Courses/course.h"
//#include "Venues/venue.h"
#include <iterator>
#include <sstream>
#include <vector>
#include <algorithm>

/**
*Reading the ectt file
*
*/

Data::Data(const char* filename){
	string line, location = "header";
	ifstream myfile (filename);
	int count = 0;
 	if (myfile.is_open())
	{
		while ( getline (myfile,line) )
		{
			istringstream ss(line);
			istream_iterator<string> begin(ss), end;

			//putting all the tokens in the vector
			vector<std::string> arrayTokens(begin, end);

			if (&arrayTokens[0] != NULL)
			{
				string* lineArray = &arrayTokens[0];  //pointer to vector so can access as array

				if ( *lineArray == "COURSES:")
				{
					//cout<<"location is courses\n";
					location = *lineArray;
				}

				if (*lineArray == "ROOMS:")
				{
					/*
					if (courses.size() == this->num_courses)
						cout<<"\t Courses are equal\n";
					else
						cout<<courses.size()<< " " <<this->num_courses<<endl;
					*/
					location= *lineArray;
					//cout<<"location is rooms\n";
				}
				if (*lineArray == "CURRICULA:")
				{
					/*
					if (rooms.size() == this->num_rooms )
						cout<<"\t Rooms are equal\n";
					*/
					location= *lineArray;
					//cout<<"location is curricular\n";
				}
				if (*lineArray == "UNAVAILABILITY_CONSTRAINTS:")
				{
					/*
					if (curriculum.size() == this->num_curricula )
						cout<<"\t Curricula are equal\n";
					else
						cout<<"\t"<<curriculum.size() << " " << this->num_curricula << endl;
					*/
					location= *lineArray;
					//cout<<"location is unavailability constraints\n";
					count =0;
				}
				if (*lineArray == "ROOM_CONSTRAINTS:")
				{
					location= *lineArray;

					/*
					if (count != this->num_unavailabilityConstraint)
						cout<<"\tNumber of Unavailable Constraints do not match\n";
					else
						cout<<"\tAll Unavailable Constraints Added\n";
					*/
					//cout<< "location  is room constraints\n";
					count =0;
				}
				if (*lineArray == "END.")
				{
					location= *lineArray;
					this->addRoomSizeConstraints();  //update constraints

					/*
					if (count != this->num_roomConstraints)
						cout<<"\tNumber of Room Constraints do not match\n";
					else
						cout<<"\n\tAll Room Constraints Added\n";
					*/
					cout<<"\n Finished Reading Input File\n";
				}

				/**
				 * Saving Header
				 * Contains number of items in each location
				 */
				if (location == "header")
				{
					//cout<<"Adding header"<<*lineArray<<endl;
					//if ((&arrayTokens[1] != NULL || &arrayTokens[2] != NULL)  && &arrayTokens[3] == NULL )
					if (&arrayTokens[1] != NULL)
					{
						if (*lineArray == "Name:")
						{
							string *token1 = &arrayTokens[1];
							this->name = *token1;
						}
						if (*lineArray == "Courses:")
						{
							string *token1 = &arrayTokens[1];
							istringstream(*token1) >> this->num_courses;
							//cout<<"num courses is "<<this->num_courses<<endl;
						}

						if (*lineArray == "Rooms:")
						{
							string *token1 = &arrayTokens[1];
							istringstream(*token1) >> this->num_rooms;
							//cout<<"num rooms is "<<this->num_rooms<<endl;

						}

						if (*lineArray  == "Days:")
						{
							string *token1 = &arrayTokens[1];
							istringstream(*token1) >> this->num_days;
							//cout<<"num days is "<<this->num_days<<endl;

						}

						if (*lineArray  == "Periods_per_day:")
						{
							string *token1 = &arrayTokens[1];
							istringstream(*token1) >> this->num_periods_per_day;
							//cout<<"num periods per day is "<<this->num_periods_per_day<<endl;

						}

						if (*lineArray  == "Curricula:")
						{
							string *token1 = &arrayTokens[1];
							//cout<<"curricula is "<<*token1<<endl;
							istringstream(*token1) >> this->num_curricula;

						}

						if (*lineArray  == "UnavailabilityConstraints:")
						{
							string *token1 = &arrayTokens[1];
							istringstream(*token1) >> this->num_unavailabilityConstraint;

						}

						if (*lineArray  == "RoomConstraints:")
						{
							string *token1 = &arrayTokens[1];
							istringstream(*token1) >> this->num_roomConstraints;

						}

						if (*lineArray  == "Min_Max_Daily_Lectures:")
						{
							string *token1 = &arrayTokens[1];
							string *token2 = &arrayTokens[2];
							istringstream(*token1) >> this->minDaily;
							istringstream(*token2) >> this->maxDaily;
						}
					}
				}
				if (location == "COURSES:")
				{
					if (*lineArray == "COURSES:")
					{
						//cout<<"\tAdding " << location << "\n";
					}
					else if (&arrayTokens[1] != NULL)
					{
						//System.out.println(line);
						string *token0 = &arrayTokens[0];
						string *token1 = &arrayTokens[1];
						string *token2 = &arrayTokens[2];
						string *token3 = &arrayTokens[3];
						string *token4 = &arrayTokens[4];
						string *token5 = &arrayTokens[5];
						int wLectures;
						int minWD;
						int numStud;
						int doubleLect;

						istringstream(*token2) >> wLectures;
						istringstream(*token3) >> minWD;
						istringstream(*token4) >> numStud;
						istringstream(*token5) >> doubleLect;
						Course c(*token0, *token1, wLectures, minWD, numStud, doubleLect);
						courses.push_back(c);
					}

				}


				if (location == "ROOMS:")
				{
					if (*lineArray == "ROOMS:")
					{
						//cout<< "\tAdding " << location << "\n";
					}
					else if (&arrayTokens[1] != NULL)
					{
						//System.out.println(line);
						string *token0 = &arrayTokens[0];
						string *token1 = &arrayTokens[1];
						string *token2 = &arrayTokens[2];

						int cap;
						int siz;

						istringstream(*token1) >> cap;
						istringstream(*token2) >> siz;
						Venue v(*token0, cap, siz);
						rooms.push_back(v);
					}

				}


				if (location == "CURRICULA:")
				{
					if (*lineArray == "CURRICULA:")
					{
						//cout << "\tAdding " << location << endl;
					}
					else if (&arrayTokens[3] != NULL)
					{
						//System.out.println(line);
						vector<string> cur; //save courses in curriculum as a list
						int numItems;
						string *token0 = &arrayTokens[0];
						string *token1 = &arrayTokens[1];

						istringstream(*token1) >> numItems;
						//cout<<numItems<<endl;

						for (int i = 2; i <= numItems + 1; i++)
						{
							string *tok = &arrayTokens[i];
							Course temp(*tok);
							std::vector<Course>::iterator findCourse = std::find(courses.begin(), courses.end(), temp);
							if (findCourse != courses.end() )  //If course code exists add course to curriculum
							{
								cur.push_back(*tok);
								Course c = *findCourse;
								c.addCurriculaCodes(*token0); //update curriculum codes;
								//courses.remove(*findCourse);  //delete previous course from list
								*findCourse = c;        //update course
								//courses.push_back(c);         //add updated course;
							}
						}
						//if (numItems !=  cur.size())
							//cout<<"Courses added not equal to number courses in currculum !!!!"<<endl;
						Curricula curricula(*token0, cur);
						curriculum.push_back(curricula);

					}
				}

				if (location == "UNAVAILABILITY_CONSTRAINTS:")
				{
					if (*lineArray == "UNAVAILABILITY_CONSTRAINTS:")
					{
						//cout<<"\tAdding " << location<<endl;
					}
					else if ( &arrayTokens[1] != NULL)
					{
						string cCode = *(&arrayTokens[0]);
						int day;
						istringstream(*(&arrayTokens[1])) >> day;
						int day_period;
						istringstream(*(&arrayTokens[2])) >> day_period;
						int period = (this->num_periods_per_day * day) + day_period;
						Course temp(cCode);
						std::vector<Course>::iterator findCourse = std::find(courses.begin(), courses.end(), temp);
						if (findCourse != courses.end())  //If course code exists add course to curriculum
						{
							Course c = *findCourse;      //get course
							c.addUnavailableConstraint(period);  //update unavailable constraints
							//courses.remove(*findCourse); //remove old course
							*findCourse = c;     //update course
							//courses.push_back(c);    //add updated course

							count++;  //increment number of unavailable constraints added
						}
						else
						{
							cout<<cCode << " is not in the course list";
						}
					}
				}

				if (location == "ROOM_CONSTRAINTS:")
				{
					if (*lineArray == "ROOM_CONSTRAINTS:")
					{
						//cout<<"\tAdding " + location;
					}
					else if ( &arrayTokens[1] != NULL)
					{
						string cCode = *(&arrayTokens[0]);
						string rCode = *(&arrayTokens[1]);
						Course temp(cCode);
						Venue temp1(rCode);


						std::vector<Course>::iterator findCourse = std::find(courses.begin(), courses.end(), temp);
						std::vector<Venue>::iterator findRoom = std::find(rooms.begin(), rooms.end(), temp1);
						if ( findCourse != courses.end() && findRoom != rooms.end())  //If course and room exists
						{
							// add room constraint -- only those rooms that fit
							// comment out for ITC2007 experiment
							// leave for NDU generation
							Venue room = *findRoom;
							Course c = *findCourse;
							if (room.getCapacity() >= c.getNumStudents())
							{
								c.addRoomConstraint(rCode); //update room constraints
							    //courses.std::remove(*findCourse); //delete old course;
							    *findCourse = c; //update course
							    //courses.push_back(c);  //add updated course
							}
							count++; //increment number of room constraints added
						}
						else
						{
							cout<<cCode << " or " << rCode << " is not in the course list\n";
						}
					}

				}
				//cout<< *lineArray <<"\n";

			}

		}
		myfile.close();
	}
	else cout << "Unable to open file"; 
}

vector<Course> Data::getCourses() const {
	return courses;
}

vector<Curricula> Data::getCurriculum() const {
	return curriculum;
}

int Data::getMaxDaily() const {
	return maxDaily;
}

int Data::getMinDaily() const {
	return minDaily;
}

string Data::getName() const {
	return name;
}

int Data::getNumCourses() const {
	return num_courses;
}

int Data::getNumCurricula() const {
	return num_curricula;
}

int Data::getNumDays() const {
	return num_days;
}

int Data::getNumPeriodsPerDay() const {
	return num_periods_per_day;
}

int Data::getNumRoomConstraints() const {
	return num_roomConstraints;
}

int Data::getNumRooms() const {
	return num_rooms;
}

int Data::getNumUnavailabilityConstraint() const {
	return num_unavailabilityConstraint;
}

vector<Venue> Data::getRooms() const {
	return rooms;
}

Data::~Data()
{

}

/**
 * Update the room constraints to include
 * those rooms that students in the course can fit into.
 *
 * Only courses with empty constraints are updated
 *
 */
void Data::addRoomSizeConstraints()
{
	vector<Course>::iterator courseToken;

	courseToken = courses.begin();
	while (courseToken != courses.end())
	{
		Course c = *courseToken;
		if (c.getRoomConstraint().size() == 0)   //add room size constraint to only those courses without room constraints already
		{
			int courseSize = c.getNumStudents();
			vector<Venue>::iterator venueToken;
			venueToken = rooms.begin();
			while (venueToken != rooms.end())
			{
				Venue v = *venueToken;
				//c.addRoomConstraint(v.getName());
				int roomCapacity = v.getCapacity() + 0;
				//int roomCapacity = courseSize;
				if (roomCapacity >= courseSize)
					c.addRoomConstraint(v.getName());  //add those rooms that can fit student
				venueToken++;
			}
			*courseToken = c;   //update course
		}
		courseToken++;
	}

}

