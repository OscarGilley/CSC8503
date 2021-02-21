#pragma once
#include "GameTechRenderer.h"
#include "../CSC8503Common/PhysicsSystem.h"
#include "StateGameObject.h"
#include "Enemy.h"
#include "Obstacle.h"
#include "Coin.h"
#include "Goal.h"
#include "ChasingEnemy.h"
#include "PlayerCharacter.h"
#include "../CSC8503Common/NavigationPath.h"
#include "../CSC8503Common/NavigationGrid.h"
//#include "../CSC8503Common/PlayerCharacter.h"

namespace NCL {
	namespace CSC8503 {
		class TutorialGame		{
		public:
			TutorialGame();
			~TutorialGame();

			virtual void UpdateGame(float dt);
			void FlipPause();
			float score = 1000;
			int coinCount = 0;
			bool won = false;
			bool complete = false;
			void InitWorld();
			void InitWorld2();
			void InitCourse();

			int level = 0;
			bool hard = false;
			bool debug = false;
			int noEnemies;

			//static void IncreaseScore(float num) {
			//	score += num;
			//}



		protected:
			void InitialiseAssets();

			void InitCamera();
			void UpdateKeys();

			

			void InitGameExamples();

			void InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius);
			void InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing);
			void InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims);
			void InitDefaultFloor();
			void InitMaze();
			void BridgeConstraintTest();
			void BridgeConstraint();
	
			bool SelectObject();
			void MoveSelectedObject();
			void DebugObjectMovement();
			void LockedObjectMovement();
			void DisplayResults(bool won);

			GameObject* AddFloorToWorld(const Vector3& position, Vector3 size, Vector3 colour, float e = 0.5);
			GameObject* AddCheckpointToWorld(const Vector3& position, Vector3 size, Vector3 colour, float e = 0.5);
			GameObject* AddIceToWorld(const Vector3& position, Vector3 size, Vector3 colour, float e = 0.5);
			GameObject* AddSwampToWorld(const Vector3& position, Vector3 size, Vector3 colour, float e = 0.5);
			GameObject* AddOBBFloorToWorld(const Vector3& position, Vector3 size, Vector3 rotation, float angle, Vector3 colour, float e = 0.5);
			GameObject* AddWallToWorld(const Vector3& position, Vector3 size, Vector3 rotation, float angle, Vector3 colour, float e = 0.5);
			GameObject* AddObstaclesToWorld(const Vector3& position);
			GameObject* AddSphereToWorld(const Vector3& position, float radius, float inverseMass = 10.0f);
			GameObject* AddObstacleToWorld(const Vector3& position, float radius, float inverseMass = 10.0f);
			GameObject* AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);
			
			GameObject* AddCapsuleToWorld(const Vector3& position, float halfHeight, float radius, Vector3 force, float inverseMass = 10.0f);

			GameObject* AddPlayerToWorld(const Vector3& position, bool sphere = false);
			GameObject* AddEnemyToWorld(const Vector3& position, NavigationPath path = {});
			GameObject* AddBehaviourEnemyToWorld(const Vector3& position);
			GameObject* AddBonusToWorld(const Vector3& position, bool die);
			GameObject* AddGoalToWorld(const Vector3& position, Vector3 size, Vector3 colour, float e = 0.5);

			StateGameObject* AddStateObjectToWorld(const Vector3& position);
			StateGameObject* testStateObject = nullptr;

			//vector<Enemy*> enemies;

			GameTechRenderer*	renderer;
			PhysicsSystem*		physics;
			GameWorld*			world;

			vector<Vector3> mazeCoins = {Vector3(10, 5, 100), Vector3(30, 5, 20), Vector3(130, 5, 90), Vector3(30, 5, 100), Vector3(30, 5, 40), Vector3(10, 5, 133)};
			vector<Vector3> mazeStarts = { Vector3(130, 5, 50), Vector3(10, 5, 80), Vector3(130, 5, 120) };
			bool useGravity;
			bool inSelectionMode;
			bool lockedObjectAirborne = false;
			bool paused = true;

			float		forceMagnitude;
			float		capsuleTimer = 0;
			float		scoreTimer = 0;
			//float		score = 1000;

			GameObject* selectionObject = nullptr;

			OGLMesh*	capsuleMesh = nullptr;
			OGLMesh*	cubeMesh	= nullptr;
			OGLMesh*	sphereMesh	= nullptr;
			OGLTexture* basicTex	= nullptr;
			OGLShader*	basicShader = nullptr;

			//Coursework Meshes
			OGLMesh*	charMeshA	= nullptr;
			OGLMesh*	charMeshB	= nullptr;
			OGLMesh*	enemyMesh	= nullptr;
			OGLMesh*	bonusMesh	= nullptr;

			//Coursework Additional functionality	
			GameObject* lockedObject	= nullptr;
			Vector3 lockedOffset		= Vector3(0, 14, 20);
			void LockCameraToObject(GameObject* o) {
				lockedObject = o;
			}

		};
	}
}

