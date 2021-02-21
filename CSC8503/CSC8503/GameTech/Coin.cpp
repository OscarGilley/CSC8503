#include "Coin.h"
#include "../CSC8503Common//StateTransition.h"
#include "../CSC8503Common/StateMachine.h"
#include "../CSC8503Common//State.h"
//#include "../GameTech/TutorialGame.h"

using namespace NCL;
using namespace CSC8503;

Coin::Coin(float* s, bool die, string n ) {
	//game = g;
	score = s;
	name = n;
	canDie = die;
}

void Coin::OnCollisionBegin(GameObject* otherObject) {
	
}

void Coin::OnCollisionEnd(GameObject* otherObject) {
	if (otherObject->GetName() == "player") {
		dead = true;
		*score += 100;
	}

	if (otherObject->GetName() == "enemy") {
		dead = true;
		//*score += 100;
	}
}

void Coin::Update(float dt) {
	counter += dt;

	/*
	if (counter > 5.0f) {
		for (int i = 0; i < 2; i++) {
			if (i == 0) {
				this->GetRenderObject()->GetTransform()->SetScale(Vector3(0, 0, 0));
			}
			if(i == 1) {
				this->GetRenderObject()->GetTransform()->SetScale(Vector3(0.25, 0.25, 0.25));
			}
		}
	}
	*/
	if (counter > 10.0f && canDie) {
		dead = true;
	}
}