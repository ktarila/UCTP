#compiler in use
CXX = g++

#Define any compiler flags
CXXFLAGS=-O3 -std=c++11 -pthread -D_DEBUG -g3 -Wall

#Library paths TO LINK WITH EXECUTABLE
LIBS = /user/local/lib/*.so


default: antSchedule geneticScheduleGAUL

antSchedule:  ./Venues/venue.o  ./Schedule/courseroomtime.o ./Schedule/feasibletable.o ./Schedule/roompath.o ./Schedule/improvetable.o  ./Schedule/path.o ./Schedule/smethods.o ./Schedule/venueTime.o  ./Curricula/curricula.o  ./Courses/course.o ./Schedule/enhancement.o  Data.o Timetable.o
	$(CXX)  $(CXXFLAGS) -o antSchedule ./Venues/venue.o  ./Schedule/courseroomtime.o ./Schedule/feasibletable.o ./Schedule/improvetable.o ./Schedule/roompath.o ./Schedule/path.o ./Schedule/smethods.o ./Schedule/venueTime.o  ./Curricula/curricula.o  ./Courses/course.o ./Schedule/enhancement.o Data.o Timetable.o

#geneticScheduleGAUL: geneticAssignment.cpp 
#	g++  -o geneticAssignment geneticAssignment.cpp -O2 -Wall   -L/usr/lib -lm -lmpi -lgaul -lgaul_util -fopenmp -lgaul -lgaul_util -lm -lmpi -lgaul -lgaul_util

geneticScheduleGAUL: ./Venues/venue.o  ./Schedule/courseroomtime.o ./Schedule/feasibletable.o \
	./Schedule/improvetable.o ./Schedule/enhancement.o ./Schedule/roompath.o ./Schedule/path.o \
	./Schedule/smethods.o ./Schedule/venueTime.o  ./Curricula/curricula.o  ./Courses/course.o  \
	Data.o  geneticAssignment.o
	$(CXX)  $(CXXFLAGS) -o geneticSchedule ./Venues/venue.o  ./Schedule/courseroomtime.o \
	./Schedule/feasibletable.o ./Schedule/improvetable.o ./Schedule/enhancement.o ./Schedule/roompath.o \
	./Schedule/path.o ./Schedule/smethods.o ./Schedule/venueTime.o  ./Curricula/curricula.o  \
	./Courses/course.o  Data.o  geneticAssignment.o -L/usr/lib -lm -lmpi -lgaul -lgaul_util -fopenmp -lgaul -lgaul_util -lm -lmpi -lgaul -lgaul_util

Timetable.o:  Timetable.cpp ReadCRT.h data.h ./Schedule/smethods.h ./Schedule/feasibletable.h ./Schedule/improvetable.h
	$(CXX) $(CXXFLAGS) -c Timetable.cpp

geneticAssignment.o:  geneticAssignment.cpp ReadCRT.h data.h ./Schedule/smethods.h ./Schedule/feasibletable.h ./Schedule/improvetable.h ./Schedule/enhancement.h ./Schedule/path.h
	$(CXX) $(CXXFLAGS) -c geneticAssignment.cpp -L/usr/lib -lm -lmpi -lgaul -lgaul_util -fopenmp -lgaul -lgaul_util -lm -lmpi -lgaul -lgaul_util

# To create the object file course:
course.o:  ./Courses/course.cpp ./Courses/course.h
	$(CXX) $(CXXFLAGS) -c ./Courses/course.cpp

# To create the object file curricula:
curricula.o:  ./Curricula/curricula.cpp ./Curricula/curricula.h
	$(CXX) $(CXXFLAGS) -c ./Curricula/curricula.cpp

# To create the object file VenueTime:
venuetime.o:  ./Schedule/venueTime.cpp ./Schedule/venueTime.h ./Venues/venues.h
	$(CXX) $(CXXFLAGS) -c ./Schedule/venueTime.cpp

# To create the object file courseroomtime:
courseroomtime.o:  ./Schedule/courseroomtime.cpp ./Schedule/venueTime.h ./Schedule/courseroomtime.h
	$(CXX) $(CXXFLAGS) -c ./Schedule/courseroomtime.cpp

# To create the object file feasibletable:
feasibletable.o:  ./Schedule/feasibletable.cpp ./Schedule/feasibletable.h  ./Curricula/curricula.h ./Schedule/venueTime.h ./Schedule/courseroomtime.h ./Schedule/improvetable.h
	$(CXX) $(CXXFLAGS) -c  ./Schedule/feasibletable.cpp

# To create the object file path:
path.o:  ./Schedule/path.cpp ./Schedule/path.h ./Schedule/courseroomtime.h
	$(CXX) $(CXXFLAGS) -c ./Schedule/path.cpp

# To create the object file path:
roompath.o:  ./Schedule/roompath.cpp ./Schedule/roompath.h ./Schedule/courseroomtime.h ./Schedule/smethods.h
	$(CXX) $(CXXFLAGS) -c ./Schedule/roompath.cpp

# To create the object file path:
smethods.o:  ./Schedule/smethods.cpp ./Schedule/smethods.h ./Schedule/courseroomtime.h ./Schedule/feasibletable.h  ./Schedule/improvetable.h ./Schedule/enhancement.h
	$(CXX) $(CXXFLAGS) -c ./Schedule/smethods.cpp

# To create the object file improvetable:
improvetable.o:  ./Schedule/improvetable.cpp ./Schedule/improvetable.h ./Schedule/smethods.h ./Schedule/path.h
	$(CC) $(CFLAGS) -c ./Schedule/improvetable.cpp

# To create the object file improvetable:
UCTPEvalOP.o:  ./Schedule/UCTPEvalOP.cpp ./Schedule/UCTPEvalOP.hpp ./Schedule/smethods.h ./Schedule/path.h
	$(CC) $(CFLAGS) -c ./Schedule/UCTPEvalOP.cpp

# To create the object file improvetable:
enhancement.o:  ./Schedule/enhancement.cpp ./Schedule/enhancement.h ./Schedule/improvetable.h ./Schedule/smethods.h ./Schedule/path.h ./Schedule/roompath.h
	$(CC) $(CFLAGS) -c ./Schedule/enhancement.cpp

# To create the object file venue:
venue.o:  ./Venues/venue.cpp ./Venues/venues.h
	$(CXX) $(CXXFLAGS) -c ./Venues/venue.cpp

# To create the object file data:
Data.o:  Data.cpp data.h ./Courses/course.h ./Venues/venue.h ./Curricula/curricula.h
	$(CXX) $(CXXFLAGS) -c Data.cpp

clean:
	$(RM) *.o *~  geneticSchedule antSchedule ./Schedule/UCTPEvalOP.o ./Venues/venue.o \
	./Schedule/enhancement.o ./Schedule/courseroomtime.o ./Schedule/feasibletable.o \
	./Schedule/feasibletable.o ./Schedule/improvetable.o ./Schedule/path.o \
	./Schedule/roompath.o ./Schedule/smethods.o ./Schedule/venueTime.o  \
	./Curricula/curricula.o  ./Courses/course.o  Data.o Timetable.o geneticAssignment geneticAssignment.o
