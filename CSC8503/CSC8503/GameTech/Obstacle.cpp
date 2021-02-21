#include "Obstacle.h"
#include "../CSC8503Common//StateTransition.h"
#include "../CSC8503Common/StateMachine.h"
#include "../CSC8503Common//State.h"
//#include "../GameTech/TutorialGame.h"

using namespace NCL;
using namespace CSC8503;

Obstacle::Obstacle() {
	float x = rand() % 20 - 10;
	float z = rand() % 5 + 1;

	float forceX = x / 20.0f;

	constForce = Vector3(forceX, -0.98, 2);
}

Obstacle::~Obstacle() {
	//delete stateMachine;
}

void Obstacle::Update(float dt) {
	//stateMachine->Update(dt);
	if (firstTime) {
		this->GetPhysicsObject()->GetInverseMass();
		GetPhysicsObject()->AddTorque(constForce + Vector3(0, 600 / this->GetPhysicsObject()->GetInverseMass(), 0));
		GetPhysicsObject()->AddForce(constForce + Vector3(0, 6000 / this->GetPhysicsObject()->GetInverseMass(), 0));
		firstTime = false;
	}
	GetPhysicsObject()->AddForce(constForce * 50 / this->GetPhysicsObject()->GetInverseMass());
	GetPhysicsObject()->AddTorque(Vector3(10, 0, 0) * 20 / this->GetPhysicsObject()->GetInverseMass());
}

void Obstacle::MoveLeft(float dt) {
	GetPhysicsObject()->AddForce({ -10, 0, 0 });
	counter += dt;
}

void Obstacle::MoveRight(float dt) {
	GetPhysicsObject()->AddForce({ 10, 0, 0 });
	counter -= dt;
}