/*
 * curricula.h
 *
 *  Created on: 21 Jun 2015
 *      Author: patrick
 */

#ifndef CURRICULA_H_
#define CURRICULA_H_
#include <iostream>
#include <stdio.h>
#include <string>
#include <vector>

using namespace std;

/*
typedef struct Course {
	int value;
	struct Node* left;
	struct Node* right;
	struct Node* parent;

	Course(int value) {
		this->value = value;
		right = NULL;
		left = NULL;
		parent = NULL;
	}
} COURSE;

*/
class Curricula {
private:
	string code;
	vector<string> courseCodes;  //list of courses on curricula


public:
	Curricula(string, vector<string>);
	virtual ~Curricula();
	bool friend operator== (const Curricula &c1, const Curricula &c2);
	bool friend operator!= (const Curricula &c1, const Curricula &c2);
	string getCode() const;
	void setCode(string code);
	vector<string> getCourseCodes() const;
	void setCourseCodes(vector<string> courseCodes);

};



#endif /* CURRICULA_H_ */
