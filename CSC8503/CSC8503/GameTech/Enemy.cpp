#include "Enemy.h"
#include "../CSC8503Common//StateTransition.h"
#include "../CSC8503Common/StateMachine.h"
#include "../CSC8503Common//State.h"
#include "../GameTech/TutorialGame.h"
#include "../CSC8503Common/NavigationPath.h"

using namespace NCL;
using namespace CSC8503;

Enemy::Enemy(NavigationPath p, bool hard, float* s, string n ) {
	//game = g;
	score = s;
	hardMode = hard;
	path = p;
	name = n;
	counter = 0.0f;
	stateMachine = new StateMachine();

	//move to goal
	State* stateA = new State([&](float dt)->void
		{
			//this->MoveLeft(dt);
			NewDestination();
			//if(path.PopWaypoint()
			//path.;
		}
	);

	State* stateB = new State([&](float dt)->void
		{
			//this->MoveRight(dt);
			currentLocation = this->GetTransform().GetPosition();
			direction = (destination - currentLocation).Normalised();
		
			this->GetPhysicsObject()->AddForce(direction * Vector3(75, 5, 75));
		}
	);

	stateMachine->AddState(stateA);
	stateMachine->AddState(stateB);

	stateMachine->AddTransition(new StateTransition(stateA, stateB, [&]()->bool
		{
			//return this->counter > 2.0f;
			return this->GetTransform().GetPosition() != destination;
		}
	));

	stateMachine->AddTransition(new StateTransition(stateB, stateA, [&]()->bool
		{
			if (abs(this->GetTransform().GetPosition().x - destination.x) < 3 && abs(this->GetTransform().GetPosition().y - destination.y) < 10 && abs(this->GetTransform().GetPosition().z - destination.z) < 3){
				return true;
			}

			return false;
				
			//return this->GetTransform().GetPosition() == destination;
		}
	));
}

Enemy::~Enemy() {
	delete stateMachine;
}

void Enemy::Update(float dt) {
	stateMachine->Update(dt);
	if (currentLocation.y < -50) {
		this->GetTransform().SetPosition(previousLocation);
	}
	//is your destination behind you?
	if (currentLocation.z < destination.z - 20) {
		NewDestination();
	}
}

void Enemy::OnCollisionBegin(GameObject* otherObject) {
	if (otherObject->GetName() != "player") {
		this->GetPhysicsObject()->SetFriction(otherObject->GetPhysicsObject()->GetFriction());
	}
	if (otherObject->GetName() == "goal" && !reachedGoal) {
		if (hardMode) {
			*score = 0;
		}

		else {
			*score -= 100;
		}
		reachedGoal = true;
	}
}

void Enemy::OnCollisionEnd(GameObject* otherObject) {
	this->GetPhysicsObject()->SetFriction(0.98f);
}

void Enemy::MoveLeft(float dt) {
	GetPhysicsObject()->AddForce({ -10, 0, 0 });
	counter += dt;
}

void Enemy::MoveRight(float dt) {
	GetPhysicsObject()->AddForce({ 10, 0, 0 });
	counter -= dt;
}

void Enemy::NewDestination() {
	GetPhysicsObject()->ClearForces();

	previousLocation = GetTransform().GetPosition();

	if (path.PopWaypoint(destination)) {
		direction = (destination - currentLocation).Normalised();
	}

	else {
		direction = Vector3(0, 0, 0);
	}
}