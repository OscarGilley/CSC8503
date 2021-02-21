#include "Goal.h"
#include "../CSC8503Common//StateTransition.h"
#include "../CSC8503Common/StateMachine.h"
#include "../CSC8503Common//State.h"
//#include "../GameTech/TutorialGame.h"

using namespace NCL;
using namespace CSC8503;

Goal::Goal(string n) {
	//game = g;
	name = n;
}

void Goal::OnCollisionBegin(GameObject* otherObject) {
	//game->score += 100;
	//game->GoalCount++;
	if (otherObject->GetName() == "player") {
		dead = true;
	}
}

void Goal::OnCollisionEnd(GameObject* otherObject) {

	if (otherObject->GetName() == "enemy") {

	}
}