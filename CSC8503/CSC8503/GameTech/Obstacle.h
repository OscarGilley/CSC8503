#pragma once
#include "..\CSC8503Common\GameObject.h"
namespace NCL {
	namespace CSC8503 {
		class StateMachine;
		//class TutorialGame;
		class Obstacle : public GameObject {
		public:
			Obstacle();
			~Obstacle();

			virtual void Update(float dt);

		protected:
			void MoveLeft(float dt);
			void MoveRight(float dt);

			//StateMachine* stateMachine;
			float counter;
			Vector3 constForce;
			bool firstTime = true;

		};
	}
}
