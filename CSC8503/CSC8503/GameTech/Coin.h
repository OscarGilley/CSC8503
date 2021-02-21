#pragma once
#include "..\CSC8503Common\GameObject.h"
namespace NCL {
	namespace CSC8503 {
		class StateMachine;
		//class TutorialGame;

		class Coin : public GameObject {
		public:
			Coin(float* score, bool die, string name = "");
			void OnCollisionBegin(GameObject* otherObject);
			void OnCollisionEnd(GameObject* otherObject);
			~Coin();

			virtual void Update(float dt);

		protected:
			//void MoveLeft(float dt);
			//void MoveRight(float dt);

			//StateMachine* stateMachine;
			float counter;
			Vector3 constForce;
			bool firstTime = true;
			float* score;
			bool canDie;

		};
	}
}
