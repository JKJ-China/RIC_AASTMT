#include "naobehavior.h"

/*
  This function returns true if we are taking some action
  to avert or recover from fall; false otherwise.

  If fallState is 0, we are still safe, and a check is performed
  to detect fall along four directions: up, down, right, and left.
  If any of these indicates fall, the fallState becomes 1, and the robot
  stretches out its arms sideways. This action typically forces the robot
  into a up or down fallen position, i.e., not sideways. fallState 1 leads
  to fallState 2, when the fall direction is reevaluated.

  From fallenUp or fallenDown, fallState 3 leads to a sequence of fallStates
  that is ultimately reset to 0.
 */

bool NaoBehavior::checkingFall() {

    VecPosition accel = bodyModel->getAccelRates();
    if(fallState == 0 || beamablePlayMode()) {
        // Check if falling. Only one flag is to be set if falling.
        fallenUp = (accel.getX() < -6.5);
        fallenDown = !fallenUp && (accel.getX() > 7.5);
        fallenRight = !fallenUp && !fallenDown && (accel.getY() < -6.5);
        fallenLeft = !fallenUp && !fallenDown && !fallenRight && (accel.getY() >  6.5);

        /*
        if(fallenLeft){
          cout << "----------FALLEN LEFT-------------\n";
        }
        else if(fallenRight){
          cout << "----------FALLEN RIGHT-------------\n";
        }
        else if(fallenUp){
          cout << "----------FALLEN UP-------------\n";
        }
        else if(fallenDown){
          cout << "----------FALLEN DOWN-------------\n";
        }
        */

        if(fallenUp || fallenDown || fallenLeft || fallenRight) {
            fallState = 1;
            currentFallStateStartTime = -1.0;
            worldModel->setFallen(true);
            worldModel->setLocalized(false);
            if (beamablePlayMode()) {
                fallState = 0;
                return false;
            }
        }
        else {
            fallState = 0;
            worldModel->setFallen(false);
            return false;
        }
    }

    // Keep head forward
    bodyModel->setScale(EFF_H1, 0.5);
    bodyModel->setTargetAngle(EFF_H1, 0);

    if(fallState == 1) {

        if(currentFallStateStartTime < 0) {
            currentFallStateStartTime = worldModel->getTime();
        }

        bodyModel->setInitialArm(ARM_LEFT);
        bodyModel->setInitialArm(ARM_RIGHT);
        bodyModel->setInitialLeg(LEG_LEFT);
        bodyModel->setInitialLeg(LEG_RIGHT);

        bodyModel->setTargetAngle(EFF_LA2, 90);
        bodyModel->setTargetAngle(EFF_RA2, -90);

        bodyModel->setTargetAngle(EFF_LA4, 0);
        bodyModel->setTargetAngle(EFF_RA4, 0);

        if (fallTimeWait < 0) {
            fallTimeWait = worldModel->getTime();
        }

        if(worldModel->getTime() - currentFallStateStartTime > 0.2) { //TUNE

            fallState = 2;
            currentFallStateStartTime = -1.0;
            fallTimeStamp = worldModel->getTime();
            fallTimeWait = -1;
        }
    }
    else if(fallState == 2) {

        if(currentFallStateStartTime < 0) {
            currentFallStateStartTime = worldModel->getTime();
        }

        if(worldModel->getTime() > (fallTimeStamp + (fallenDown ? atof(namedParams.find("getup_parms_stateDownInitialWait")->second.c_str()) : atof(namedParams.find("getup_parms_stateUpInitialWait")->second.c_str())))) {
            fallTimeStamp = -1;
            fallState = 1;
            currentFallStateStartTime = -1.0;

            // Check once again. If still fallen right or left, act as though fallen up.
            fallenUp = (accel.getX() < -6.5);
            fallenDown = !fallenUp && (accel.getX() > 6.5);
            fallenRight = !fallenUp && !fallenDown && (accel.getY() < -6.5);
            fallenLeft = !fallenUp && !fallenDown && !fallenRight && (accel.getY() >  6.5);

            if(fallenUp || fallenRight || fallenLeft) {
                fallenUp = true;
                fallenRight = false;
                fallenLeft = false;
                fallState = 3;
                currentFallStateStartTime = -1.0;
            }
            else if(fallenDown) {
                fallState = 3;
                currentFallStateStartTime = -1.0;
            }
            else { //Magically, we are up!
                fallState = 0;
                currentFallStateStartTime = -1.0;
            }
        }
    }


    //Recovering
    if(fallenDown) {

        if(fallState == 3) {

            if(currentFallStateStartTime < 0) {
                currentFallStateStartTime = worldModel->getTime();
            }

            bodyModel->setTargetAngle(EFF_LA1, atof(namedParams.find("getup_parms_stateDown3A1")->second.c_str()));
            bodyModel->setTargetAngle(EFF_RA1, atof(namedParams.find("getup_parms_stateDown3A1")->second.c_str()));

            bodyModel->setTargetAngle(EFF_LA2, atof(namedParams.find("getup_parms_stateDown3A2")->second.c_str()));
            bodyModel->setTargetAngle(EFF_RA2, atof(namedParams.find("getup_parms_stateDown3A2")->second.c_str()));

            bodyModel->setTargetAngle(EFF_LL1, atof(namedParams.find("getup_parms_stateDown3L1")->second.c_str()));
            bodyModel->setTargetAngle(EFF_RL1, atof(namedParams.find("getup_parms_stateDown3L1")->second.c_str()));

            bodyModel->setTargetAngle(EFF_LL3, atof(namedParams.find("getup_parms_stateDown3L3")->second.c_str()));
            bodyModel->setTargetAngle(EFF_RL3, atof(namedParams.find("getup_parms_stateDown3L3")->second.c_str()));

            bodyModel->setTargetAngle(EFF_LL5, atof(namedParams.find("getup_parms_stateDown3L5")->second.c_str()));
            bodyModel->setTargetAngle(EFF_RL5, atof(namedParams.find("getup_parms_stateDown3L5")->second.c_str()));

            if (bodyModel->hasToe()) {
                bodyModel->setTargetAngle(EFF_LL7, atof(namedParams.find("getup_parms_stateDown3L7")->second.c_str()));
                bodyModel->setTargetAngle(EFF_RL7, atof(namedParams.find("getup_parms_stateDown3L7")->second.c_str()));
            }

            if (fallTimeWait < 0) {
                fallTimeWait = worldModel->getTime();
            }

            if(worldModel->getTime() - currentFallStateStartTime > atof(namedParams.find("getup_parms_stateDown3MinTime")->second.c_str())) {
                fallState = 4;
                currentFallStateStartTime = -1.0;
                fallTimeStamp = worldModel->getTime();
                fallTimeWait = -1;
            }
        }
        else if(fallState == 4) {

            if(currentFallStateStartTime < 0) {
                currentFallStateStartTime = worldModel->getTime();
            }

            bodyModel->setTargetAngle(EFF_LL1, atof(namedParams.find("getup_parms_stateDown4L1")->second.c_str()));
            bodyModel->setTargetAngle(EFF_RL1, atof(namedParams.find("getup_parms_stateDown4L1")->second.c_str()));

            bodyModel->setTargetAngle(EFF_LL3, atof(namedParams.find("getup_parms_stateDown4L3")->second.c_str()));
            bodyModel->setTargetAngle(EFF_RL3, atof(namedParams.find("getup_parms_stateDown4L3")->second.c_str()));

            if(worldModel->getTime() - currentFallStateStartTime > atof(namedParams.find("getup_parms_stateDown4MinTime")->second.c_str())) {
                 fallState = 5;
                 currentFallStateStartTime = -1.0;
                 fallTimeStamp = worldModel->getTime();
                 fallTimeWait = -1;
             }
        }
        else if(fallState == 5) {

            if(currentFallStateStartTime < 0) {
                currentFallStateStartTime = worldModel->getTime();
            }

            bodyModel->setTargetAngle(EFF_LL1, atof(namedParams.find("getup_parms_stateDown5L1")->second.c_str()));
            bodyModel->setTargetAngle(EFF_RL1, atof(namedParams.find("getup_parms_stateDown5L1")->second.c_str()));

            bodyModel->setTargetAngle(EFF_LL3, atof(namedParams.find("getup_parms_stateDown5L3")->second.c_str()));
            bodyModel->setTargetAngle(EFF_RL3, atof(namedParams.find("getup_parms_stateDown5L3")->second.c_str()));

            // take care of the toe 5 or 7 ??
            if (bodyModel->hasToe()) {
                bodyModel->setTargetAngle(EFF_LL7, atof(namedParams.find("getup_parms_stateDown5L7")->second.c_str()));
                bodyModel->setTargetAngle(EFF_RL7, atof(namedParams.find("getup_parms_stateDown5L7")->second.c_str()));
            }

            if (fallTimeWait < 0) {
                fallTimeWait = worldModel->getTime();
            }

            if(worldModel->getTime() - currentFallStateStartTime > atof(namedParams.find("getup_parms_stateDown5MinTime")->second.c_str())) {
                fallState = 6;
                currentFallStateStartTime = -1.0;
                fallTimeStamp = worldModel->getTime();
                fallTimeWait = -1;
            }
        }
        else if(fallState == 6) {

            if(currentFallStateStartTime < 0) {
                currentFallStateStartTime = worldModel->getTime();
            }

            bodyModel->setInitialArm(ARM_LEFT);
            bodyModel->setInitialArm(ARM_RIGHT);
            bodyModel->setInitialLeg(LEG_LEFT);
            bodyModel->setInitialLeg(LEG_RIGHT);

            if (fallTimeWait < 0) {
                fallTimeWait = worldModel->getTime();
            }

            if(worldModel->getTime() - currentFallStateStartTime > atof(namedParams.find("getup_parms_stateDown6MinTime")->second.c_str())) {
                fallState = 7;
                currentFallStateStartTime = -1.0;
                fallTimeStamp = worldModel->getTime();
                fallTimeWait = -1;
            }
        }
        else if(fallState == 7) {

            if(currentFallStateStartTime < 0) {
                currentFallStateStartTime = worldModel->getTime();
            }

            if (worldModel->getTime() < (fallTimeStamp + 1.0)) {
                VecPosition gyroRates = bodyModel->getGyroRates();
                // Check for stability and if gyro rates are high don't say that we have recovered.
                if (gyroRates.getX() > .5 || gyroRates.getY() > .5 || gyroRates.getZ() > .5) {
                    return true;
                }
            }

            if(worldModel->getTime() > (fallTimeStamp + 0.1)) {
                fallTimeStamp = -1;
                currentFallStateStartTime = -1.0;
                fallState = 0;
                fallenDown = false;
                lastGetupRecoveryTime = worldModel->getTime();
                // recursive ...
                return checkingFall();

            }
        }
    }

    if(fallenUp) {

        if(fallState == 3) {

            if(currentFallStateStartTime < 0) {
                currentFallStateStartTime = worldModel->getTime();
            }

            bodyModel->setTargetAngle(EFF_LA1, atof(namedParams.find("getup_parms_stateUp3A1")->second.c_str()));
            bodyModel->setTargetAngle(EFF_RA1, atof(namedParams.find("getup_parms_stateUp3A1")->second.c_str()));

            bodyModel->setTargetAngle(EFF_LA2, atof(namedParams.find("getup_parms_stateUp3A2")->second.c_str()));
            bodyModel->setTargetAngle(EFF_RA2, -atof(namedParams.find("getup_parms_stateUp3A2")->second.c_str()));

            bodyModel->setTargetAngle(EFF_LA4, atof(namedParams.find("getup_parms_stateUp3A4")->second.c_str()));
            bodyModel->setTargetAngle(EFF_RA4, -atof(namedParams.find("getup_parms_stateUp3A4")->second.c_str()));

            bodyModel->setTargetAngle(EFF_LL3, atof(namedParams.find("getup_parms_stateUp3L3")->second.c_str()));
            bodyModel->setTargetAngle(EFF_RL3, atof(namedParams.find("getup_parms_stateUp3L3")->second.c_str()));

            if (bodyModel->hasToe()) {
                bodyModel->setTargetAngle(EFF_LL7, atof(namedParams.find("getup_parms_stateUp3L7")->second.c_str()));
                bodyModel->setTargetAngle(EFF_RL7, atof(namedParams.find("getup_parms_stateUp3L7")->second.c_str()));
            }

            if (fallTimeWait < 0) {
                fallTimeWait = worldModel->getTime();
            }

            if(worldModel->getTime() - currentFallStateStartTime > atof(namedParams.find("getup_parms_stateUp3MinTime")->second.c_str())) {
                fallState = 4;
                currentFallStateStartTime = -1.0;
                fallTimeStamp = worldModel->getTime();
                fallTimeWait = -1;
            }
        }
        else if(fallState == 4) {

            if(currentFallStateStartTime < 0) {
                currentFallStateStartTime = worldModel->getTime();
            }
            bodyModel->setTargetAngle(EFF_LL3, atof(namedParams.find("getup_parms_stateUp4L3")->second.c_str()));
            bodyModel->setTargetAngle(EFF_RL3, atof(namedParams.find("getup_parms_stateUp4L3")->second.c_str()));

            bodyModel->setTargetAngle(EFF_LA1, 0);
            bodyModel->setTargetAngle(EFF_RA1, 0);

            bodyModel->setTargetAngle(EFF_LA2, 0);
            bodyModel->setTargetAngle(EFF_RA2, 0);

            bodyModel->setTargetAngle(EFF_LL1, atof(namedParams.find("getup_parms_stateUp4L1")->second.c_str()));
            bodyModel->setTargetAngle(EFF_RL1, atof(namedParams.find("getup_parms_stateUp4L1")->second.c_str()));

            if (bodyModel->hasToe()) {
                bodyModel->setTargetAngle(EFF_LL7, atof(namedParams.find("getup_parms_stateUp4L7")->second.c_str()));
                bodyModel->setTargetAngle(EFF_RL7, atof(namedParams.find("getup_parms_stateUp4L7")->second.c_str()));
            }

            if (fallTimeWait < 0) {
                fallTimeWait = worldModel->getTime();
            }

            if(worldModel->getTime() - currentFallStateStartTime > atof(namedParams.find("getup_parms_stateUp4MinTime")->second.c_str())) {
                fallState = 5;
                currentFallStateStartTime = -1.0;
                fallTimeStamp = worldModel->getTime();
                fallTimeWait = -1;
            }
        }
        else if(fallState == 5) {

            if(currentFallStateStartTime < 0) {
                currentFallStateStartTime = worldModel->getTime();
            }

            bodyModel->setTargetAngle(EFF_LA1, atof(namedParams.find("getup_parms_stateUp5A1")->second.c_str()));
            bodyModel->setTargetAngle(EFF_RA1, atof(namedParams.find("getup_parms_stateUp5A1")->second.c_str()));

            bodyModel->setTargetAngle(EFF_LL1, atof(namedParams.find("getup_parms_stateUp5L1")->second.c_str()));
            bodyModel->setTargetAngle(EFF_RL1, atof(namedParams.find("getup_parms_stateUp5L1")->second.c_str()));

            bodyModel->setTargetAngle(EFF_LL4, atof(namedParams.find("getup_parms_stateUp5L4")->second.c_str()));
            bodyModel->setTargetAngle(EFF_RL4, atof(namedParams.find("getup_parms_stateUp5L4")->second.c_str()));

            bodyModel->setTargetAngle(EFF_LL5, atof(namedParams.find("getup_parms_stateUp5L5")->second.c_str()));
            bodyModel->setTargetAngle(EFF_RL5, atof(namedParams.find("getup_parms_stateUp5L5")->second.c_str()));

            bodyModel->setTargetAngle(EFF_LL6, atof(namedParams.find("getup_parms_stateUp5L6")->second.c_str()));
            bodyModel->setTargetAngle(EFF_RL6, -atof(namedParams.find("getup_parms_stateUp5L6")->second.c_str()));

            if (bodyModel->hasToe()) {
                bodyModel->setTargetAngle(EFF_LL7, atof(namedParams.find("getup_parms_stateUp5L7")->second.c_str()));
                bodyModel->setTargetAngle(EFF_RL7, atof(namedParams.find("getup_parms_stateUp5L7")->second.c_str()));
            }

            if (fallTimeWait < 0) {
                fallTimeWait = worldModel->getTime();
            }

            if(worldModel->getTime() - currentFallStateStartTime > atof(namedParams.find("getup_parms_stateUp5MinTime")->second.c_str())) {
                fallState = 6;
                currentFallStateStartTime = -1.0;
                fallTimeStamp = worldModel->getTime();
                fallTimeWait = -1;
            }
        }
        else if(fallState == 6) {

            if(currentFallStateStartTime < 0) {
                currentFallStateStartTime = worldModel->getTime();
            }

            bodyModel->setTargetAngle(EFF_LA1, atof(namedParams.find("getup_parms_stateUp6A1")->second.c_str()));
            bodyModel->setTargetAngle(EFF_RA1, atof(namedParams.find("getup_parms_stateUp6A1")->second.c_str()));

            bodyModel->setTargetAngle(EFF_LL1, atof(namedParams.find("getup_parms_stateUp6L1")->second.c_str()));
            bodyModel->setTargetAngle(EFF_RL1, atof(namedParams.find("getup_parms_stateUp6L1")->second.c_str()));

            bodyModel->setTargetAngle(EFF_LL5, atof(namedParams.find("getup_parms_stateUp6L5")->second.c_str()));
            bodyModel->setTargetAngle(EFF_RL5, atof(namedParams.find("getup_parms_stateUp6L5")->second.c_str()));

            if (bodyModel->hasToe()) {
                bodyModel->setTargetAngle(EFF_LL7, atof(namedParams.find("getup_parms_stateUp6L7")->second.c_str()));
                bodyModel->setTargetAngle(EFF_RL7, atof(namedParams.find("getup_parms_stateUp6L7")->second.c_str()));
            }

            if (fallTimeWait < 0) {
                fallTimeWait = worldModel->getTime();
            }

            if(worldModel->getTime() - currentFallStateStartTime > atof(namedParams.find("getup_parms_stateUp6MinTime")->second.c_str())) {
                fallState = 7;
                currentFallStateStartTime = -1.0;
                fallTimeStamp = worldModel->getTime();
                fallTimeWait = -1;
            }
        }
        else if(fallState == 7) {

            if(currentFallStateStartTime < 0) {
                currentFallStateStartTime = worldModel->getTime();
            }

            bodyModel->setTargetAngle(EFF_LA1, atof(namedParams.find("getup_parms_stateUp7A1")->second.c_str()));
            bodyModel->setTargetAngle(EFF_RA1, atof(namedParams.find("getup_parms_stateUp7A1")->second.c_str()));

            bodyModel->setTargetAngle(EFF_LL1, atof(namedParams.find("getup_parms_stateUp7L1")->second.c_str()));
            bodyModel->setTargetAngle(EFF_RL1, atof(namedParams.find("getup_parms_stateUp7L1")->second.c_str()));

            bodyModel->setTargetAngle(EFF_LL3, atof(namedParams.find("getup_parms_stateUp7L3")->second.c_str()));
            bodyModel->setTargetAngle(EFF_RL3, atof(namedParams.find("getup_parms_stateUp7L3")->second.c_str()));

            bodyModel->setTargetAngle(EFF_LL4, atof(namedParams.find("getup_parms_stateUp7L4")->second.c_str()));
            bodyModel->setTargetAngle(EFF_RL4, atof(namedParams.find("getup_parms_stateUp7L4")->second.c_str()));

            bodyModel->setTargetAngle(EFF_LL5, atof(namedParams.find("getup_parms_stateUp7L5")->second.c_str()));
            bodyModel->setTargetAngle(EFF_RL5, atof(namedParams.find("getup_parms_stateUp7L5")->second.c_str()));

            if (bodyModel->hasToe()) {
                bodyModel->setTargetAngle(EFF_LL7, atof(namedParams.find("getup_parms_stateUp7L7")->second.c_str()));
                bodyModel->setTargetAngle(EFF_RL7, atof(namedParams.find("getup_parms_stateUp7L7")->second.c_str()));
            }

            if (fallTimeWait < 0) {
                fallTimeWait = worldModel->getTime();
            }

            //      if(bodyModel->targetsReached()){
            if(worldModel->getTime() - currentFallStateStartTime > atof(namedParams.find("getup_parms_stateUp7MinTime")->second.c_str())) {
                fallState = 8;
                currentFallStateStartTime = -1.0;
                fallTimeStamp = worldModel->getTime();
                fallTimeWait = -1;
            }
        }
        else if(fallState == 8) {

            if(currentFallStateStartTime < 0) {
                currentFallStateStartTime = worldModel->getTime();
            }

            bodyModel->setInitialArm(ARM_LEFT);
            bodyModel->setInitialArm(ARM_RIGHT);
            bodyModel->setInitialLeg(LEG_LEFT);
            bodyModel->setInitialLeg(LEG_RIGHT);

            if (fallTimeWait < 0) {
                fallTimeWait = worldModel->getTime();
            }

            if(worldModel->getTime() - currentFallStateStartTime > atof(namedParams.find("getup_parms_stateUp8MinTime")->second.c_str())) {
                fallState = 9;
                currentFallStateStartTime = -1.0;
                fallTimeStamp = worldModel->getTime();
                fallTimeWait = -1;
            }
        }
        else if(fallState == 9) {

            if(currentFallStateStartTime < 0) {
                currentFallStateStartTime = worldModel->getTime();
            }

            if (worldModel->getTime() < (fallTimeStamp + 1.0)) {
                VecPosition gyroRates = bodyModel->getGyroRates();
                // Check for stability and if gyro rates are high don't say that we have recovered.
                if (gyroRates.getX() > .5 || gyroRates.getY() > .5 || gyroRates.getZ() > .5) {
                    return true;
                }
            }

            if(worldModel->getTime() > (fallTimeStamp + 0.1)) {
                fallTimeStamp = -1;
                fallState = 0;
                currentFallStateStartTime = -1.0;
                fallenUp = false;
                lastGetupRecoveryTime = worldModel->getTime();
                // recursive ...
                return checkingFall();
            }
        }
    }

    return true;

}
