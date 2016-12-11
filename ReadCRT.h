/*
 * ReadCRT.h
 *
 *  Created on: 8 Aug 2015
 *      Author: patrick
 */

#ifndef READCRT_H_
#define READCRT_H_

#include "Schedule/courseroomtime.h"
#include <iostream>
#include <iterator>
#include <sstream>
#include <vector>
#include <algorithm>

vector<CourseRoomTime> readCRT(vector<Course> courses, vector<Venue> rooms, int periods_per_day, const char* filename)
{

	vector<CourseRoomTime> jCRT;
	string line;
	ifstream myfile (filename);
 	if (myfile.is_open())
	{
		while ( getline (myfile,line) )
		{
			istringstream ss(line);
			istream_iterator<string> begin(ss), end;

			//putting all the tokens in the vector
			vector<std::string> arrayTokens(begin, end);
			//string* lineArray = &arrayTokens[0];  //pointer to vector so can access as array

			if (&arrayTokens[0] != NULL)
			{
				int period = 0;
				int day = 0;
				string cCode, rCode;
				string *token0 = &arrayTokens[0];
				string *token1 = &arrayTokens[1];
				string *token2 = &arrayTokens[2];
				string *token3 = &arrayTokens[3];

				istringstream(*token2) >> day;
				istringstream(*token3) >> period;
				rCode = *token1;
				Course temp(*token0);
				std::vector<Course>::iterator findCourse = std::find(courses.begin(), courses.end(), temp);

				Venue temp1(*token1);
				std::vector<Venue>::iterator findRoom = std::find(rooms.begin(), rooms.end(), temp1);
				if (findCourse != courses.end() && findRoom != rooms.end())
				{
					int periodValue = (day * periods_per_day) + period;
					Course c = *findCourse;
					Venue r = *findRoom;
					CourseRoomTime crt(c, RoomTime(periodValue, r));
					jCRT.push_back(crt);
				}
			}
		}
		myfile.close();
	}
	else std::cout << "Unable to open file";
 	return jCRT;


}



#endif /* READCRT_H_ */
