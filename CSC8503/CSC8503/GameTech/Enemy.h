#pragma once
#include "..\CSC8503Common\GameObject.h"
#include "../CSC8503Common/NavigationPath.h"

namespace NCL {
	namespace CSC8503 {
		class StateMachine;
		//class TutorialGame;
		class Enemy : public GameObject {
		public:
			Enemy(NavigationPath path, bool hardMode, float* score, string name = "" );
			~Enemy();

			virtual void Update(float dt);

			void SetNavigationPath(NavigationPath p) {
				path = p;
			}

			NavigationPath GetPath() {
				return path;
			}

			void OnCollisionBegin(GameObject* otherObject);
			void OnCollisionEnd(GameObject* otherObject);

			Vector3 destination;

		protected:
			void MoveLeft(float dt);
			void MoveRight(float dt);

			void NewDestination();

			bool hardMode;
			bool reachedGoal;
			float* score;
			StateMachine* stateMachine;
			NavigationPath path;
			Vector3 currentLocation;
			Vector3 previousLocation;
			Vector3 direction;
			float counter;
		};
	}
}