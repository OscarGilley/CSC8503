#include "StateGameObject.h"
#include "../CSC8503Common//StateTransition.h"
#include "../CSC8503Common/StateMachine.h"
#include "../CSC8503Common//State.h"
#include <random>
//#include "../GameTech/TutorialGame.h"


using namespace NCL;
using namespace CSC8503;

StateGameObject::StateGameObject() {
	counter = 0.0f;
	stateMachine = new StateMachine();

	State* stateA = new State([&](float dt)->void
		{
			this->MoveLeft(dt);
		}
	);

	State* stateB = new State([&](float dt)->void
		{
			this->MoveRight(dt);
		}
	);

	stateMachine->AddState(stateA);
	stateMachine->AddState(stateB);

	stateMachine->AddTransition(new StateTransition(stateA, stateB, [&]()->bool
		{
			//return this->counter > 0.5f;
			return GetRenderObject()->GetTransform()->GetPosition().x < -15;
		}
	));

	stateMachine->AddTransition(new StateTransition(stateB, stateA, [&]()->bool
		{
			if (GetRenderObject()->GetTransform()->GetPosition().z > 15) {

			}
			//return this->counter < -0.5f;
			return GetRenderObject()->GetTransform()->GetPosition().x > 15;
		}
	));


}

StateGameObject::~StateGameObject() {
	delete stateMachine;
}

void StateGameObject::Update(float dt) {
	stateMachine->Update(dt);
}

void StateGameObject::MoveLeft(float dt) {
	GetPhysicsObject()->AddForce({ -400, 0, 0 });
	counter += dt;
}

void StateGameObject::MoveRight(float dt) {
	GetPhysicsObject()->AddForce({ 400, 0, 0 });
	counter -= dt;
}