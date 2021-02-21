#pragma once
#include "..\CSC8503Common\GameObject.h"
#include "../CSC8503Common/NavigationPath.h"
#include "../CSC8503Common/GameWorld.h"

namespace NCL {
	namespace CSC8503 {
		class StateMachine;
		//class TutorialGame;
		class PlayerCharacter : public GameObject {
		public:
			PlayerCharacter(GameWorld* w, string name = "");
			~PlayerCharacter();

			virtual void Update(float dt);
			void OnCollisionBegin(GameObject* otherObject);
			void OnCollisionEnd(GameObject* otherObject);

			//Vector3 destination;

		protected:
			void MoveLeft(float dt);
			void MoveRight(float dt);

			void NewDestination();

			bool checkpoint;
			GameWorld* world;
			Vector3 direction;
			float counter;
		};
	}
}