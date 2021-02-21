#pragma once
#include "..\CSC8503Common\GameObject.h"
#include "../CSC8503Common/BehaviourNode.h"
#include "../CSC8503Common/BehaviourAction.h"
#include "../CSC8503Common/BehaviourSequence.h"
#include "../CSC8503Common/BehaviourSelector.h"
#include "../CSC8503Common/NavigationPath.h"
#include "../CSC8503Common/GameWorld.h"

namespace NCL {
	namespace CSC8503 {
		class StateMachine;
		//class GameWorld;
		//class TutorialGame;
		class ChasingEnemy : public GameObject {
		public:
			ChasingEnemy(GameWorld* w, string name = "");
			~ChasingEnemy();

			virtual void Update(float dt);

			Vector3 destination;

		protected:
			void MoveLeft(float dt);
			void MoveRight(float dt);

			//void NewDestination();

			Vector3* playerPosPoint;
			GameObject* player;
			Vector3 playerPos;
			vector<Vector3> cards = { Vector3(1, 0, 0), Vector3(0, 0, 1), Vector3(-1, 0, 0), Vector3(0, 0, -1) };
			GameWorld* world;
			BehaviourSelector* root;
			bool first = true;

			StateMachine* stateMachine;
			NavigationPath path;
			Vector3 currentPosition;
			Vector3 previousLocation;
			Vector3 direction;
			float counter;
		};
	}
}
