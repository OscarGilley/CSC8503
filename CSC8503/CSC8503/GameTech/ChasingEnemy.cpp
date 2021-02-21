#include "ChasingEnemy.h"
#include "../CSC8503Common//StateTransition.h"
#include "../CSC8503Common/StateMachine.h"
#include "../CSC8503Common//State.h"
#include "../GameTech/TutorialGame.h"
#include "../CSC8503Common/NavigationPath.h"
#include "../CSC8503Common/GameWorld.h"

using namespace NCL;
using namespace CSC8503;


ChasingEnemy::ChasingEnemy(GameWorld* w, string n) {
	world = w;
	name = n;
	counter = 0.0f;
	//cards
	
	//float behaviourTimer;
	//float distanceToTarget;
	BehaviourAction* findPlayer = new BehaviourAction("Find Player", [&](float dt, BehaviourState state)->BehaviourState {
		if (state == Initialise) {
			std::cout << "Looking for player!\n";
			counter = 0.0f;
			//behaviourTimer = rand() % 100;
			state = Ongoing;
		}
		else if (state == Ongoing) {
			counter += dt;
			for (int i = 0; i < 4; i++) {
				int randX = rand() % 64 + 1;
				int randZ = rand() % 64 + 1;
				int randY = rand() % 2 - 1;
				Vector3 edge = this->GetTransform().GetPosition() + (Vector3(1 / randX, 0, 1 / randZ) + Vector3(2.1f, randY, 2.1f));
				//Vector3 edge = this->GetTransform().GetPosition() + (cards[i] * 2.1f);
				Ray r = Ray(edge, cards[i]);
				//std::cout << this->GetTransform().GetPosition();

				RayCollision closestCollision;

				world->Raycast(r, closestCollision, true);
				Debug::DrawLine(this->GetTransform().GetPosition(), closestCollision.collidedAt, Vector4(1, 1, 1, 1), 10.0f);
				//Debug::DrawLine(this->GetTransform().GetPosition(), edge, Vector4(1, 1, 1, 1), 10.0f);
				//Debug::DrawLine(closestCollision.collidedAt, this->GetTransform().GetPosition(), Vector4(1, 1, 1, 1), 10.0f);
				GameObject* castedObject = (GameObject*)closestCollision.node;

				if (castedObject) {
					if (castedObject->GetName() == "player") {
						std::cout << "Located.";
						playerPosPoint = &(castedObject->GetTransform().GetPosition());
						playerPos = castedObject->GetTransform().GetPosition();
						player = castedObject;

						return Success;
					}
					//if (castedObject->GetName() == "coin") {
					//	return Failure;
					//}
				}
			}

			if (counter > 10.0f) {
				std::cout << "Can't find them :(\n";
				return Failure;
			}
			
			//if (behaviourTimer <= 0.0f) {
			//	std::cout << "Found a key!\n";
			//	return Success;
			//}
		}
		return state;
		}
	);

	BehaviourAction* goToPlayer = new BehaviourAction("Chase Player", [&](float dt, BehaviourState state)->BehaviourState {
		if (state == Initialise) {
			std::cout << "Going to chase them!\n";
			state = Ongoing;
		}
		else if (state == Ongoing) {
			//distanceToTarget -= dt;
			direction = (player->GetTransform().GetPosition() - this->GetTransform().GetPosition()).Normalised();

			this->GetPhysicsObject()->AddForce((direction * 45));

			if ((playerPos - this->GetTransform().GetPosition()).Length() > 50) {
				std::cout << "Lost Player!\n";
				return Failure;
			}
		}
		return state; //will be 'ongoing' until success
		}
	);

	BehaviourAction* lookForCoin = new BehaviourAction("Looking for Coin", [&](float dt, BehaviourState state)->BehaviourState {
		if (state == Initialise) {
			std::cout << "Looking for coin!\n";
			counter = 0.0f;
			//behaviourTimer = rand() % 100;
			state = Ongoing;
		}
		else if (state == Ongoing) {
			counter += dt;
			for (int i = 0; i < 4; i++) {
				int randX = rand() % 64 + 1;
				int randZ = rand() % 64 + 1;
				int randY = rand() % 2 - 1;
				Vector3 edge = this->GetTransform().GetPosition() + (Vector3(1 / randX, 0, 1 / randZ) + Vector3(2.1f, randY, 2.1f));
				Ray r = Ray(edge, cards[i]);

				//std::cout << r.GetDirection();
				RayCollision closestCollision;

				world->Raycast(r, closestCollision, true);
				//Debug::DrawLine(closestCollision.collidedAt, r.GetDirection(), Vector4(1, 1, 1, 1), 10.0f);
				GameObject* castedObject = (GameObject*)closestCollision.node;

				if (castedObject) {
					if (castedObject->GetName() == "coin") {
						std::cout << "Money.\n";
						destination = castedObject->GetTransform().GetPosition();
						return Success;
					}
					if (castedObject->GetName() == "player" && closestCollision.rayDistance < 50) {
						return Failure;
					}
				}
			}


			if (counter > 10.0f) {
				std::cout << "Can't find money :(\n";
				return Failure;
			}

			//if (behaviourTimer <= 0.0f) {
			//	std::cout << "Found a key!\n";
			//	return Success;
			//}
		}
		return state;
		}
	);

	BehaviourAction* getCoin = new BehaviourAction("Get Coin", [&](float dt, BehaviourState state)->BehaviourState {
		if (state == Initialise) {
			std::cout << "Going to get rich!\n";
			state = Ongoing;
		}
		else if (state == Ongoing) {
			//distanceToTarget -= dt;
			direction = (destination - this->GetTransform().GetPosition()).Normalised();

			this->GetPhysicsObject()->AddForce((direction * 45));

			if ((destination - this->GetTransform().GetPosition()).Length() < 2) {
				std::cout << "Got the cash.\n";
				return Success;
			}
		}
		return state; //will be 'ongoing' until success
		}
	);

	BehaviourSequence* sequence = new BehaviourSequence("Player Selector");
	sequence->AddChild(findPlayer);
	sequence->AddChild(goToPlayer);
	//sequence->AddChild(openDoor);

	BehaviourSequence* selection = new BehaviourSequence("Coin Sequence");
	selection->AddChild(lookForCoin);
	selection->AddChild(getCoin);

	root = new BehaviourSelector("Root Sequence");
	root->AddChild(sequence);
	root->AddChild(selection);

	
}

ChasingEnemy::~ChasingEnemy() {
	delete stateMachine;
}

void ChasingEnemy::Update(float dt) {
	//stateMachine->Update(dt);

	if (first) {
		root->Reset();
		first = false;
	}
	
	BehaviourState state = Ongoing;
	if (state == Ongoing) {
		state = root->Execute(dt);
	}

	if (state == Success || state == Failure) {
		root->Reset();
	}
	
}

void ChasingEnemy::MoveLeft(float dt) {
	GetPhysicsObject()->AddForce({ -10, 0, 0 });
	counter += dt;
}

void ChasingEnemy::MoveRight(float dt) {
	GetPhysicsObject()->AddForce({ 10, 0, 0 });
	counter -= dt;
}