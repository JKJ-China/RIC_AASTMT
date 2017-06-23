#include "naobehavior.h"
#include "../rvdraw/rvdraw.h"

#include "../chain_Acion/analyzer.h"

#include <iostream>
#include <vector>
#include<cassert>
#include <fstream>
using namespace std;

vector<VecPosition> sttaics;
bool start = 0;
bool printed = 0;
void add(double x, double y, double z = 0, vector<VecPosition> &v = sttaics) {
	v.push_back(*(new VecPosition(x, y, z)));
}

void load(string filename) {

	sttaics.clear();
	fstream of(filename.c_str(), ios_base::in);
	double x, y;
	while (of >> x >> y)
		add(x, y);
	of.close();
	/*add(-1,0);
	 add(0, 4);
	 add(6, -4);
	 add(6, 4);
	 add(4, 0);*/
}
/*
 * Real game beaming.
 * Filling params x y angle
 */
void NaoBehavior::beam(double& beamX, double& beamY, double& beamAngle) {
	//cout<<" side is "<<worldModel->getSide();
	//if (worldModel->getSide() == SIDE_LEFT)
	loader->setStrategy("second");
	VecPosition beamer = loader->getBeamingPosition(this->agentUNum);
	beamX = beamer.getX();
	beamY = beamer.getY();
	beamAngle = beamer.getZ();
//beamX = -15+agentUNum;
//beamY = 0;
//beamAngle = 0;

}

SkillType NaoBehavior::selectSkill() {

	if (worldModel->getPlayMode() != PM_PLAY_ON) {
		return getPlayModeSkill();
	}
	if (loader->getTeamState() == ATTACKING) {
		return getAttackSkill();

	}
	if (loader->getTeamState() == DEFENDING) {
			return getDefensiveSkill();

		}

	if ((((worldModel->getSide() == SIDE_LEFT)
			&& (worldModel->getPlayMode() == PM_KICK_OFF_LEFT))
			|| ((worldModel->getSide() == SIDE_RIGHT)
					&& (worldModel->getPlayMode() == PM_KICK_OFF_RIGHT)))
			&& (worldModel->getTeammateClosestTo(worldModel->getBall())
					== worldModel->getUNum())) {
		return kickBall(KICK_FORWARD,
				((worldModel->getOppLeftGoalPost()
						+ worldModel->getOppRightGoalPost()) / 2));
	}
	if (analyzer->getTopSkill().getType() == SKILL_PASS
			&& worldModel->getBall().getDistanceTo(worldModel->getMyPosition())
					> 1) {
		analyzer->resetCandidates();

	}
	if (analyzer->getTopSkill().getType() == SKILL_DRIBBLE
			&& worldModel->getBall().getDistanceTo(worldModel->getMyPosition())
					> 0.8) {
		analyzer->resetCandidates();

	}
	analyzer->generateCanditates();
	skilldesc skilltarg = analyzer->getTopSkill();
	SkillType ret;

	if (skilltarg.getType() == SKILL_PASS) {
		//cout<<"SKILL = SKILL_PASS"<<endl;
		if (worldModel->getBall().getDistanceTo(skilltarg.getTarget()) > 5)
			ret = kickBall(KICK_FORWARD, skilltarg.getTarget());
		else
			ret = kickBall(KICK_IK, skilltarg.getTarget());
		//if (ret != SKILL_STAND && ret != SKILL_WALK_OMNI)
		//analyzer->resetCandidates();
		return ret;
	} else if (skilltarg.getType() == SKILL_WALK_OMNI
			|| skilltarg.getType() == SKILL_INTERCEPT) {
		//cout<<"SKILL = SKILL_WALK_OMNI"<<endl;
		analyzer->resetCandidates();
		/*if(worldModel->getRole(worldModel->getUNum()) < 2) {
		 if(skilltarg.getTarget().getDistanceTo(loader->getDuePosition(worldModel->getUNum())) > 5
		 && skilltarg.getTarget().getX() > loader->getDuePosition(worldModel->getUNum()).getX()+3
		 )
		 return SKILL_STAND;
		 }*/
		double distance, angle, targ;
		getTargetDistanceAndAngle(skilltarg.getTarget(), distance, angle);
		/*	if (abs(angle) > 10) {
		 return goToTargetRelative(VecPosition(), angle);
		 } else */

		if (distance > 0.2) {
			return goToTarget(skilltarg.getTarget());

		} else if (skilltarg.getType() == SKILL_INTERCEPT
				&& (worldModel->getMyPosition().getX() > -12
						|| fabs(worldModel->getMyAngDeg()) < 90)) {
			/*VecPosition target = (worldModel->getBall() - worldModel->getMyPosition())*1.5;
			 target += worldModel->getMyPosition();
			 ret = kickBall(KICK_DRIBBLE, skilltarg.getTarget());

			 analyzer->resetCandidates();
			 return ret;*/
			return intercept();

		} else if (worldModel->getMyPosition().getDistanceTo(
				loader->getDuePosition(worldModel->getUNum())) < 0.5) {
			double angle;
			VecPosition ball = worldModel->getBall();
			double dis;
			getTargetDistanceAndAngle(ball, dis, angle);
			return goToTargetRelative(VecPosition(), angle);
		}
		return SKILL_STAND;

	} else if (skilltarg.getType() == SKILL_SHOOT) {
		//cout<<"SKILL = SKILL_KICK_LEFT_LEG"<<endl;
		ret = kickBall(KICK_FORWARD, skilltarg.getTarget());
		analyzer->resetCandidates();
		return ret;
	} else if (skilltarg.getType() == SKILL_DRIBBLE) {
		ret = kickBall(KICK_DRIBBLE, skilltarg.getTarget());

		//analyzer->resetCandidates();
		return ret;
	} else
		analyzer->resetCandidates();

	return SKILL_STAND;
}

/*
 * Demo behavior where players form a rotating circle and kick the ball
 * back and forth
 */
SkillType NaoBehavior::demoKickingCircle() {
	// Parameters for circle
	VecPosition center = VecPosition(-HALF_FIELD_X / 2.0, 0, 0);
	double circleRadius = 5.0;
	double rotateRate = 2.5;

	// Find closest player to ball
	int playerClosestToBall = -1;
	double closestDistanceToBall = 10000;
	for (int i = WO_TEAMMATE1; i < WO_TEAMMATE1 + NUM_AGENTS; ++i) {
		VecPosition temp;
		int playerNum = i - WO_TEAMMATE1 + 1;
		if (worldModel->getUNum() == playerNum) {
			// This is us
			temp = worldModel->getMyPosition();
		} else {
			WorldObject* teammate = worldModel->getWorldObject(i);
			if (teammate->validPosition) {
				temp = teammate->pos;
			} else {
				continue;
			}
		}
		temp.setZ(0);

		double distanceToBall = temp.getDistanceTo(ball);
		if (distanceToBall < closestDistanceToBall) {
			playerClosestToBall = playerNum;
			closestDistanceToBall = distanceToBall;
		}
	}

	if (playerClosestToBall == worldModel->getUNum()) {
		// Have closest player kick the ball toward the center
		analyzer->generateCanditates();
		skilldesc skilltarg = analyzer->getTopSkill();
		if (skilltarg.getType() == SKILL_PASS) {
			//cout<<"shooting";
			return kickBall(KICK_IK, skilltarg.getTarget());
		} else if (skilltarg.getType() == SKILL_WALK_OMNI) {
			return goToTarget(skilltarg.getTarget());
		} else
			//return goToTargetRelative(VecPosition(1,0,0), 0);
			return SKILL_STAND;
		return kickBall(KICK_FORWARD, center);
	} else {
		// Move to circle position around center and face the center
		VecPosition localCenter = worldModel->g2l(center);
		SIM::AngDeg localCenterAngle = atan2Deg(localCenter.getY(),
				localCenter.getX());

		// Our desired target position on the circle
		// Compute target based on uniform number, rotate rate, and time
		VecPosition target = center
				+ VecPosition(circleRadius, 0, 0).rotateAboutZ(
						360.0 / (NUM_AGENTS - 1)
								* (worldModel->getUNum()
										- (worldModel->getUNum()
												> playerClosestToBall ? 1 : 0))
								+ worldModel->getTime() * rotateRate);

		// Adjust target to not be too close to teammates or the ball
		target = collisionAvoidance(true /*teammate*/, false/*opponent*/,
				true/*ball*/, 1/*proximity thresh*/, .5/*collision thresh*/,
				target, true/*keepDistance*/);

		if (me.getDistanceTo(target) < .25 && abs(localCenterAngle) <= 10) {
			// Close enough to desired position and orientation so just stand
			return SKILL_STAND;
		} else if (me.getDistanceTo(target) < .5) {
			// Close to desired position so start turning to face center
			return goToTargetRelative(worldModel->g2l(target), localCenterAngle);
		} else {
			// Move toward target location
			return goToTarget(target);
		}
	}
}
