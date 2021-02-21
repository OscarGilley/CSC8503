#include "PlayerCharacter.h"
#include "../CSC8503Common//StateTransition.h"
#include "../CSC8503Common/StateMachine.h"
#include "../CSC8503Common//State.h"
#include "../CSC8503Common/GameWorld.h"
#include "../CSC8503Common/NavigationPath.h"

using namespace NCL;
using namespace CSC8503;

PlayerCharacter::PlayerCharacter(GameWorld* w, string n) {
	//game = g;
	world = w;
	name = n;
	counter = 0.0f;
}

PlayerCharacter::~PlayerCharacter() {
	//delete stateMachine;
}

void PlayerCharacter::Update(float dt) {
	if (this->GetTransform().GetPosition().y < -50) {
		if (checkpoint) {
			GetTransform().SetPosition(Vector3(0, 79, -382));
			//this-SetPosition()
		}
		else {
			GetTransform().SetPosition(Vector3(0, 20, 0));
		}
	}

	//stateMachine->Update(dt);
	//if (currentLocation.y < -50) {
	//	this->GetTransform().SetPosition(previousLocation);
	//}

	//is your destination behind you?
	//if (currentLocation.z < destination.z - 20) {
	//	NewDestination();
	//}
}

void PlayerCharacter::OnCollisionBegin(GameObject* otherObject) {
	if (otherObject->GetName() != "enemy") {
		this->GetPhysicsObject()->SetFriction(otherObject->GetPhysicsObject()->GetFriction());
	}
	if (otherObject->GetName() == "checkpoint") {
		checkpoint = true;
	}
}

void PlayerCharacter::OnCollisionEnd(GameObject* otherObject) {
	this->GetPhysicsObject()->SetFriction(0.98f);
}

void PlayerCharacter::MoveLeft(float dt) {
	GetPhysicsObject()->AddForce({ -10, 0, 0 });
	counter += dt;
}

void PlayerCharacter::MoveRight(float dt) {
	GetPhysicsObject()->AddForce({ 10, 0, 0 });
	counter -= dt;
}