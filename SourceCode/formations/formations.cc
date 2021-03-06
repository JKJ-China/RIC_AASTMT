/*
 * formations.cc
 *
 *  Created on: Feb 7, 2017
 *      Author: aastteam
 */
#include "formation.h"
#include "../behaviors/naobehavior.h"
#include <fstream>
#include <iostream>
using namespace std;
formationLoader::formationLoader() {
	state = ATTACKING;

}
formationLoader::~formationLoader() {

}
void formationLoader::setStrategy(string strategy) {
	this->strategy = strategy;
	loadFormations();
}
void formationLoader::loadFormations() {

	beamings.clear();
	attacking.clear();
	defending.clear();
	string bfile = "./formations/" + strategy + "/beaming.io";
	fstream of(bfile.c_str(), ios_base::in);
	double x, y, ang,len,wid;
	while (of >> x >> y >> ang) {
		beamings.push_back(*(new VecPosition(x, y, ang)));

	}
	of.close();
	bfile = "./formations/" + strategy + "/attacking.io";
	fstream of2(bfile.c_str(), ios_base::in);
	//double x, y,ang;
	while (of2 >> x >> y >> ang >>len>>wid){
		attacking.push_back(*(new VecPosition(x, y, ang)));
		attackingLengths.push_back(len);
		attackingWidths.push_back(wid);
	}
	of2.close();
	bfile = "./formations/" + strategy + "/defending.io";
	fstream of3(bfile.c_str(), ios_base::in);
	//double x, y,ang;
	while (of3 >> x >> y >> ang >> len>>wid)
		{defending.push_back(*(new VecPosition(x, y, ang)));
		defendingLengths.push_back(len);
		defendingWidths.push_back(wid);

		}
		if(defending.size() < 11 || attacking.size() < 11 || beamings.size() < 11)
			NaoBehavior::debug->fail(0,"couldn't load files");
	of3.close();

}

