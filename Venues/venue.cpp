/*
 * venue.cpp
 *
 *  Created on: 21 Jun 2015
 *      Author: patrick
 */

#include "venue.h"
#include <sstream>

Venue::Venue(string name, int capacity, int site){
	this->name = name;
	this->capacity= capacity;
	this->site = site;
}

int Venue::getCapacity() const {
	return capacity;
}

int Venue::getSite() const {
	return site;
}

Venue::Venue(string name)
{
	this->name = name;
}

Venue::Venue()
{

}

Venue::~Venue(){

}

bool operator== (const Venue &c1, const Venue &c2)
{
	if (c1.name == c2.name)
		return true;
	else
		return false;

}


bool operator!= (const Venue &c1, const Venue &c2)
{
	return  !(c1 == c2);
}


string Venue::toString()
{
	std::stringstream sstm;
	sstm<<"Name: " << this->name << " Capacity: "<< this->capacity << " Site: " << this->site;
	return sstm.str();
}

string Venue::getName()
{
	return this->name;
}

void Venue::setName(string name)
{
	this->name = name;
}






