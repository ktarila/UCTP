/*
 * curricula.cpp
 *
 *  Created on: 21 Jun 2015
 *      Author: patrick
 */

#include "curricula.h"

Curricula::Curricula(string code, vector<string> courseCodes){
	this->code = code;
	this->courseCodes = courseCodes;
}

Curricula::~Curricula(){

}

bool operator== (const Curricula &c1, const Curricula &c2)
{
	if (c1.code == c2.code)
		return true;
	else
		return false;
}

string Curricula::getCode() const {
	return code;
}

void Curricula::setCode(string code) {
	this->code = code;
}

vector<string> Curricula::getCourseCodes() const {
	return courseCodes;
}

void Curricula::setCourseCodes(vector<string> courseCodes) {
	this->courseCodes = courseCodes;
}

bool operator!= (const Curricula &c1, const Curricula &c2)
{
	return  !(c1 == c2);
}



