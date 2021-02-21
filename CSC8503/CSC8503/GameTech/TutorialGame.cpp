#include "TutorialGame.h"
#include "../CSC8503Common/GameWorld.h"
#include "../../Plugins/OpenGLRendering/OGLMesh.h"
#include "../../Plugins/OpenGLRendering/OGLShader.h"
#include "../../Plugins/OpenGLRendering/OGLTexture.h"
#include "../../Common/TextureLoader.h"
#include "../CSC8503Common/PositionConstraint.h"

//TODO:
//Do polish! Maybe work on behaviour tree AI!

using namespace NCL;
using namespace CSC8503;

TutorialGame::TutorialGame()	{
	world		= new GameWorld();
	renderer	= new GameTechRenderer(*world);
	physics		= new PhysicsSystem(*world);
	noEnemies = 0;

	forceMagnitude	= 10.0f;
	useGravity		= true;
	physics->UseGravity(useGravity);
	inSelectionMode = false;

	Debug::SetRenderer(renderer);

	InitialiseAssets();
}

/*

Each of the little demo scenarios used in the game uses the same 2 meshes, 
and the same texture and shader. There's no need to ever load in anything else
for this module, even in the coursework, but you can add it if you like!

*/
void TutorialGame::InitialiseAssets() {
	auto loadFunc = [](const string& name, OGLMesh** into) {
		*into = new OGLMesh(name);
		(*into)->SetPrimitiveType(GeometryPrimitive::Triangles);
		(*into)->UploadToGPU();
	};

	loadFunc("cube.msh"		 , &cubeMesh);
	loadFunc("sphere.msh"	 , &sphereMesh);
	loadFunc("Male1.msh"	 , &charMeshA);
	loadFunc("courier.msh"	 , &charMeshB);
	loadFunc("security.msh"	 , &enemyMesh);
	loadFunc("coin.msh"		 , &bonusMesh);
	loadFunc("capsule.msh"	 , &capsuleMesh);

	basicTex	= (OGLTexture*)TextureLoader::LoadAPITexture("checkerboard.png");
	basicShader = new OGLShader("GameTechVert.glsl", "GameTechFrag.glsl");

	InitCamera();
	InitWorld();
}

TutorialGame::~TutorialGame()	{
	delete cubeMesh;
	delete sphereMesh;
	delete charMeshA;
	delete charMeshB;
	delete enemyMesh;
	delete bonusMesh;

	delete basicTex;
	delete basicShader;

	delete physics;
	delete renderer;
	delete world;
}

void TutorialGame::FlipPause() {
	if (paused) {
		paused = false;
	}
	else {
		paused = true;
	}
}

void TutorialGame::UpdateGame(float dt) {
	if (!paused) {
		if (!inSelectionMode) {
			world->GetMainCamera()->UpdateCamera(dt);
		}

		if (score <= 0) {
			FlipPause();
			won = false;
			complete = true;
			
			//DisplayResults(false);
		}

		std::vector<GameObject*>::const_iterator first;
		std::vector<GameObject*>::const_iterator last;
		world->GetObjectIterators(first, last);
		std::vector<GameObject*> deadObjects;

		for (auto i = first; i != last; ++i) {
			(*i)->Update(dt);

			if ((*i)->IsDead() == true) {
				if ((*i)->GetName() == "goal") {
					FlipPause();
					won = true;
					complete = true;
					break;
				}

				deadObjects.push_back(*i);
			}
			else {
				if ((*i)->GetTransform().GetPosition().y < -60) {
					if ((*i)->GetName() != "player") {
						deadObjects.push_back(*i);

						//(*i)->GetTransform().SetPosition(Vector3(0, 20, 0));
					}
					else {
						deadObjects.push_back(*i);

					}

				}
			}
		}

		if (deadObjects.size() > 0) {
			for (int j = deadObjects.size(); j > 0; j--) {
				world->RemoveGameObject(deadObjects.at(j - 1));
				//delete deadObjects.at(j);
			}
		}


		UpdateKeys();

		if (debug) {
			if (useGravity) {
				Debug::Print("(G)ravity on", Vector2(5, 95));
			}
			else {
				Debug::Print("(G)ravity off", Vector2(5, 95));
			}
		}
		if (debug) {
			Debug::Print("Debug Mode On! Toggle with Z!", Vector2(5, 15));
			Debug::Print("Use the F5, F6 and F7 keys to switch between levels!", Vector2(5, 20));
		}
		else {
			Debug::Print("Debug Mode Off! Toggle with Z!", Vector2(5, 15));
		}

		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::Z)) {
			debug = !debug;
		}

		string scoreStr = std::to_string(score);

		Debug::Print("Score: " + scoreStr, Vector2(5, 90));

		if (capsuleTimer <= 0 && level == 1) {
			float ballSize = rand() % 4 + 2;
			float randX = rand() % 20 - 10;
			float randZ = rand() % 50 - 350;
			//std::cout << ballSize;
			AddObstacleToWorld(Vector3(randX, 75, -400), ballSize, 5 / ballSize);
			AddBonusToWorld(Vector3(randX, 70, randZ), true);

			capsuleTimer += 2;
		}

		capsuleTimer -= dt;

		if (scoreTimer <= 0 && level != 3 && !hard) {
			score -= 10;
			scoreTimer += 1;
		}

		if (scoreTimer <= 0 && level != 3 && hard) {
			score -= 20;
			scoreTimer += 1;
		}
		
		if (capsuleTimer <= 0 && level == 3) {
			float randX = rand() % 100 - 50;
			float randZ = rand() % 100 - 50;
			AddBonusToWorld(Vector3(randX, 2, randZ), false);

			capsuleTimer += 2;
		}

		scoreTimer -= dt;

		//score -= 1;

		SelectObject();
		MoveSelectedObject();
		physics->Update(dt);

		if (lockedObject != nullptr) {
			Vector3 objPos = lockedObject->GetTransform().GetPosition();
			Vector3 camPos = objPos + lockedOffset;

			if (debug) {
				Debug::Print("Name: " + lockedObject->GetName(), Vector2(5, 75));
			}

			Matrix4 temp = Matrix4::BuildViewMatrix(camPos, objPos, Vector3(0, 1, 0));

			Matrix4 modelMat = temp.Inverse();

			Quaternion q(modelMat);
			Vector3 angles = q.ToEuler(); //nearly there now!

			world->GetMainCamera()->SetPosition(camPos);
			world->GetMainCamera()->SetPitch(angles.x);
			world->GetMainCamera()->SetYaw(angles.y);

			if (level == 2) {
				world->GetMainCamera()->SetPosition(camPos - Vector3(0, -20, 20));
				world->GetMainCamera()->SetPitch(-90.0f);
				world->GetMainCamera()->SetYaw(360.0f);
			}

			//Debug::DrawLine(camPos, objPos, Vector4(1,1,1,1), 120);
			//Debug::DrawLine(objPos, Vector3(10,10,10));

			//Debug::DrawLine(objPos, camPos);

			//Debug::DrawAxisLines(lockedObject->GetTransform().GetMatrix(), 2.0f);
		}
		
	}

	world->UpdateWorld(dt);
	renderer->Update(dt);

	Debug::FlushRenderables(dt);
	renderer->Render();
}

void TutorialGame::DisplayResults(bool win) {

	//paused = true;
}

void TutorialGame::UpdateKeys() {
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F7)) {
		InitWorld(); //We can reset the simulation at any time with F5
		selectionObject = nullptr;
		lockedObject	= nullptr;
		
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F6)) {
		InitWorld2();
		selectionObject = nullptr;
		lockedObject = nullptr;
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F5)) {
		InitCourse();
		selectionObject = nullptr;
		lockedObject = nullptr;
	}

	//if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::NUM0)) {
	//	InitWorld();
	//}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F2)) {
		InitCamera(); //F2 will reset the camera to a specific default place
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::G)) {
		useGravity = !useGravity; //Toggle gravity!
		physics->UseGravity(useGravity);
	}
	//Running certain physics updates in a consistent order might cause some
	//bias in the calculations - the same objects might keep 'winning' the constraint
	//allowing the other one to stretch too much etc. Shuffling the order so that it
	//is random every frame can help reduce such bias.
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F9)) {
		world->ShuffleConstraints(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F10)) {
		world->ShuffleConstraints(false);
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F7)) {
		world->ShuffleObjects(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F8)) {
		world->ShuffleObjects(false);
	}

	if (lockedObject) {
		//Debug::DrawLine(world->GetMainCamera()->GetPosition(), lockedObject->GetTransform().GetPosition());
		LockedObjectMovement();
	}
	else {
		DebugObjectMovement();
	}
}

void TutorialGame::LockedObjectMovement() {

	Matrix4 view		= world->GetMainCamera()->BuildViewMatrix();
	Matrix4 camWorld	= view.Inverse();

	Vector3 rightAxis = Vector3(camWorld.GetColumn(0)); //view is inverse of model!

	//forward is more tricky -  camera forward is 'into' the screen...
	//so we can take a guess, and use the cross of straight up, and
	//the right axis, to hopefully get a vector that's good enough!

	Vector3 fwdAxis = Vector3::Cross(Vector3(0, 1, 0), rightAxis);
	fwdAxis.y = 0.0f;
	fwdAxis.Normalise();

	Vector3 charForward  = lockedObject->GetTransform().GetOrientation() * Vector3(0, 0, 1);
	Vector3 charForward2 = lockedObject->GetTransform().GetOrientation() * Vector3(0, 0, 1);

	float force = 75.0f;

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::LEFT)) {
		lockedObject->GetPhysicsObject()->AddForce(-rightAxis * force);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
		//Vector3 worldPos = selectionObject->GetTransform().GetPosition();
		lockedObject->GetPhysicsObject()->AddForce(rightAxis * force);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP)) {
		lockedObject->GetPhysicsObject()->AddForce(fwdAxis * force);
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::SPACE) && !lockedObjectAirborne) {
		lockedObject->GetPhysicsObject()->AddForce(Vector3(0, 2000, 0));
		lockedObjectAirborne = true;
	}

	if (lockedObjectAirborne) {
		RayCollision floorDetection;
		Ray ray = Ray(lockedObject->GetTransform().GetPosition(), Vector3(0, -1, 0));

		Vector3 halfDirs = ((AABBVolume&)*lockedObject->GetBoundingVolume()).GetHalfDimensions();

		if (world->Raycast(ray, floorDetection, true)) {
			if (floorDetection.rayDistance <= halfDirs.y) {
				lockedObjectAirborne = false;
			}
		}
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN)) {
		lockedObject->GetPhysicsObject()->AddForce(-fwdAxis * force);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::Q)) {
		lockedObject->GetPhysicsObject()->AddTorque(rightAxis * force);
	}
	
	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::E)) {
		lockedObject->GetPhysicsObject()->AddTorque(-rightAxis * force);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM2)) {
		lockedObject->GetPhysicsObject()->AddTorque(-fwdAxis * force);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM3)) {
		lockedObject->GetPhysicsObject()->AddTorque(fwdAxis * force);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NEXT)) {
		lockedObject->GetPhysicsObject()->AddForce(Vector3(0,-10,0));
	}
}

void TutorialGame::DebugObjectMovement() {
//If we've selected an object, we can manipulate it with some key presses
	if (inSelectionMode && selectionObject) {
		//Twist the selected object!
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::LEFT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(-10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM7)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, 10, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM8)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, -10, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, -10));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, 10));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM5)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, -10, 0));
		}
	}

}

void TutorialGame::InitCamera() {
	world->GetMainCamera()->SetNearPlane(0.1f);
	world->GetMainCamera()->SetFarPlane(500.0f);
	world->GetMainCamera()->SetPitch(-15.0f);
	world->GetMainCamera()->SetYaw(315.0f);
	world->GetMainCamera()->SetPosition(Vector3(-60, 40, 60));
	lockedObject = nullptr;
}

void TutorialGame::InitWorld() {
	world->ClearAndErase();
	physics->Clear();
	testStateObject = AddStateObjectToWorld(Vector3(0, 10, 0));
	level = 3;
	capsuleTimer = 0;
	//enemies.emplace_back(AddEnemyToWorld(Vector3(10, 0, 10)));
	AddGoalToWorld(Vector3(75, -1.9, 80), Vector3(10, 2, 10), Vector4(0, 1, 0, 1), 0.5);


	//InitMixedGridWorld(5, 5, 3.5f, 3.5f);
	InitGameExamples();
	InitDefaultFloor();
	//BridgeConstraint();
	//InitMaze();
}

void TutorialGame::InitWorld2() {
	world->ClearAndErase();
	physics->Clear();
	//testStateObject = AddStateObjectToWorld(Vector3(0, 10, 0));
	level = 2;
	//enemies.emplace_back(AddEnemyToWorld(Vector3(10, 0, 10)));
	int randStart = rand() % 3;

	AddPlayerToWorld(mazeStarts[randStart]);
	

	//InitMixedGridWorld(10, 10, 3.5f, 3.5f);
	//InitGameExamples();
	AddFloorToWorld(Vector3(75, -2, 75), Vector3(100, 2, 100), Vector4(1, 0.271, 0, 1), 0.5);

	InitMaze();
}

void TutorialGame::InitMaze() {
	//world->ClearAndErase();
	//physics->Clear();
	//testStateObject = AddStateObjectToWorld(Vector3(0, 10, 0));

	//outer walls
	AddFloorToWorld(Vector3(140, 10, 75), Vector3(5, 20, 75), Vector4(0, 0, 1, 1), 0.5);//right
	AddFloorToWorld(Vector3(0, 10, 75), Vector3(5, 20, 75), Vector4(0, 0, 1, 1), 0.5);//left
	AddFloorToWorld(Vector3(75, 10, 140), Vector3(75, 20, 5), Vector4(0, 0, 1, 1), 0.5);//bottom
	AddFloorToWorld(Vector3(75, 10, 0), Vector3(75, 20, 5), Vector4(0, 0, 1, 1), 0.5);//top

	//starting from the top
	AddFloorToWorld(Vector3(20, 10, 25), Vector3(5, 20, 25), Vector4(0, 0, 1, 1), 0.5);//vertical
	AddFloorToWorld(Vector3(30, 10, 30), Vector3(15, 20, 5), Vector4(0, 0, 1, 1), 0.5);//horizontal
	AddFloorToWorld(Vector3(25, 10, 50), Vector3(10, 20, 5), Vector4(0, 0, 1, 1), 0.5);//horizontal
	AddFloorToWorld(Vector3(50, 10, 65), Vector3(5, 20, 20), Vector4(0, 0, 1, 1), 0.5);//vertical
	AddFloorToWorld(Vector3(60, 10, 70), Vector3(5, 20, 15), Vector4(0, 0, 1, 1), 0.5);//vertical
	AddFloorToWorld(Vector3(10, 10, 70), Vector3(5, 20, 5), Vector4(0, 0, 1, 1), 0.5);//vertical
	//AddFloorToWorld(Vector3(40, 10, 70), Vector3(5, 20, 5), Vector4(0, 0, 1, 1), 0.5);//vertical
	AddFloorToWorld(Vector3(30, 10, 60), Vector3(5, 20, 15), Vector4(0, 0, 1, 1), 0.5);//vertical
	AddFloorToWorld(Vector3(20, 10, 100), Vector3(5, 20, 15), Vector4(0, 0, 1, 1), 0.5);//vertical
	AddFloorToWorld(Vector3(30, 10, 110), Vector3(15, 20, 5), Vector4(0, 0, 1, 1), 0.5);//horizontal
	AddFloorToWorld(Vector3(45, 10, 130), Vector3(30, 20, 5), Vector4(0, 0, 1, 1), 0.5);//horizontal
	AddFloorToWorld(Vector3(60, 10, 115), Vector3(5, 20, 10), Vector4(0, 0, 1, 1), 0.5);//vertical
	AddFloorToWorld(Vector3(70, 10, 115), Vector3(5, 20, 10), Vector4(0, 0, 1, 1), 0.5);//vertical
	AddFloorToWorld(Vector3(70, 10, 40), Vector3(5, 20, 5), Vector4(0, 0, 1, 1), 0.5);//vertical

	AddFloorToWorld(Vector3(90, 10, 115), Vector3(5, 20, 10), Vector4(0, 0, 1, 1), 0.5);//vertical
	AddFloorToWorld(Vector3(120, 10, 110), Vector3(30, 20, 5), Vector4(0, 0, 1, 1), 0.5);//horizontal
	AddFloorToWorld(Vector3(115, 10, 130), Vector3(10, 20, 5), Vector4(0, 0, 1, 1), 0.5);//horizontal

	//AddFloorToWorld(Vector3(110, 10, 30), Vector3(5, 20, 15), Vector4(0, 0, 1, 1), 0.5);//horizontal
	AddFloorToWorld(Vector3(80, 10, 60), Vector3(5, 20, 25), Vector4(0, 0, 1, 1), 0.5);//vertical


	AddFloorToWorld(Vector3(40, 10, 25), Vector3(5, 20, 10), Vector4(0, 0, 1, 1), 0.5);//vertical
	AddFloorToWorld(Vector3(80, 10, 20), Vector3(25, 20, 5), Vector4(0, 0, 1, 1), 0.5);//horizontal
	AddFloorToWorld(Vector3(120, 10, 10), Vector3(5, 20, 10), Vector4(0, 0, 1, 1), 0.5);//vertical
	AddFloorToWorld(Vector3(100, 10, 30), Vector3(5, 20, 15), Vector4(0, 0, 1, 1), 0.5);//vertical
	AddFloorToWorld(Vector3(120, 10, 40), Vector3(25, 20, 5), Vector4(0, 0, 1, 1), 0.5);//horizontal
	AddFloorToWorld(Vector3(110, 10, 55), Vector3(5, 20, 10), Vector4(0, 0, 1, 1), 0.5);//vertical
	AddFloorToWorld(Vector3(120, 10, 55), Vector3(5, 20, 10), Vector4(0, 0, 1, 1), 0.5);//vertical

	AddFloorToWorld(Vector3(90, 10, 85), Vector3(5, 20, 10), Vector4(0, 0, 1, 1), 0.5);//vertical
	AddFloorToWorld(Vector3(50, 10, 90), Vector3(45, 20, 5), Vector4(0, 0, 1, 1), 0.5);//horizontal

	AddFloorToWorld(Vector3(110, 10, 85), Vector3(5, 20, 10), Vector4(0, 0, 1, 1), 0.5);//vertical
	AddFloorToWorld(Vector3(120, 10, 85), Vector3(5, 20, 10), Vector4(0, 0, 1, 1), 0.5);//vertical

	AddGoalToWorld(Vector3(75, -1.9, 80), Vector3(10, 2, 10), Vector4(0, 1, 0, 1), 0.5);

	for (int i = 0; i < 3; i++) {
		int randCoin = rand() % 6;
		AddBonusToWorld(mazeCoins[randCoin], false);
	}

	NavigationGrid grid("TestGrid3.txt");

	NavigationPath topLeft;
	NavigationPath topRight;
	NavigationPath bottomLeft;
	NavigationPath bottomRight;

	//Vector3 startPos(10, 0, 10);
	Vector3 endPos(70, 0, 70);

	grid.FindPath(Vector3(10,0,10), endPos, topLeft);
	grid.FindPath(Vector3(130, 0, 10), endPos, topRight);
	grid.FindPath(Vector3(10, 0, 130), endPos, bottomLeft);
	grid.FindPath(Vector3(130, 0, 130), endPos, bottomRight);

	/*
	vector<Vector3> starts;
	starts.push_back(Vector3(10, 0, 10));
	starts.push_back(Vector3(130, 0, 10));
	starts.push_back(Vector3(10, 0, 130));
	starts.push_back(Vector3(130, 0, 130));

	vector<NavigationPath> paths;
	paths.push_back(topLeft);
	paths.push_back(topRight);
	paths.push_back(bottomLeft);
	paths.push_back(bottomRight);

	//for (int i = 0; i < noEnemies; i++) {
	//	AddEnemyToWorld(starts[i], paths[i]);
	//}
	*/

	AddEnemyToWorld(Vector3(10, 2, 10), topLeft);
	AddEnemyToWorld(Vector3(130, 2, 10), topRight);
	AddEnemyToWorld(Vector3(10, 2, 130), bottomLeft);
	AddEnemyToWorld(Vector3(130, 2, 130), bottomRight);
	
	//AddFloorToWorld(Vector3(0, 2, -100), Vector3(200, 2, 10), Vector3(1, 0, 0), 90, Vector4(1, 1, 1, 1), 0.5);
	//AddFloorToWorld(Vector3(0, 2, 100), Vector3(200, 2, 10), Vector3(1, 0, 0), 90, Vector4(1, 1, 1, 1), 0.5);
	//AddWallToWorld(Vector3(0, 0, 0), Vector3(20, 2, 60), Vector3(0, 0, 1), 90, Vector4(1, 1, 1, 1), 0.5);
	//AddFloorToWorld(Vector3(-100, 2, 0), Vector3(10, 2, 200), Vector3(0, 0, 1), 90, Vector4(1, 1, 1, 1), 0.5);
	//AddToWorld(Vector3(100, 2, 0), Vector3(10, 2, 200), Vector3(0, 0, 1), 90, Vector4(1, 1, 1, 1), 0.5);
	//AddWallToWorld(Vector3(0, 0, 0), Vector3(20, 2, 60), Vector3(0, 1, 0), 90, Vector4(1, 1, 1, 1), 0.5);
	//AddOBBFloorToWorld(Vector3(0, 31.75, -391), Vector3(20, 2, 60), Vector3(1, 0, 0), 30, Vector4(1, 1, 1, 1), 0.5);
}

void TutorialGame::InitCourse() {
	world->ClearAndErase();
	physics->Clear();
	//testStateObject = AddStateObjectToWorld(Vector3(0, 10, 0));
	level = 1;
	capsuleTimer = 0;
	AddPlayerToWorld(Vector3(0, 10, 0));

	NavigationPath leftPath = NavigationPath();
	NavigationPath rightPath = NavigationPath();
	NavigationPath middlePath = NavigationPath();

	leftPath.PushWaypoint(Vector3(-60, 59, -552));
	leftPath.PushWaypoint(Vector3(-60, 59, -402));
	leftPath.PushWaypoint(Vector3(-20, 59, -382));
	leftPath.PushWaypoint(Vector3(-20, 2, -250));
	//leftPath.PushWaypoint(Vector3(-60, 20, -150));
	leftPath.PushWaypoint(Vector3(-60, 2, -140));
	leftPath.PushWaypoint(Vector3(-60, 20, -83.5));
	leftPath.PushWaypoint(Vector3(-60, 2, -0));

	rightPath.PushWaypoint(Vector3(60, 59, -552));
	rightPath.PushWaypoint(Vector3(60, 59, -402));
	rightPath.PushWaypoint(Vector3(20, 59, -382));
	rightPath.PushWaypoint(Vector3(20, 2, -250));
	//rightPath.PushWaypoint(Vector3(60, 20, -150));
	rightPath.PushWaypoint(Vector3(60, 2, -140));
	rightPath.PushWaypoint(Vector3(60, 20, -83.5));
	rightPath.PushWaypoint(Vector3(60, 2, -0));

	middlePath.PushWaypoint(Vector3(0, 59, -552));
	middlePath.PushWaypoint(Vector3(0, 59, -382));
	middlePath.PushWaypoint(Vector3(0, 2, -250));
	//middlePath.PushWaypoint(Vector3(0, 20, -150));
	middlePath.PushWaypoint(Vector3(0, 2, -140));
	middlePath.PushWaypoint(Vector3(0, 2, -70));
	middlePath.PushWaypoint(Vector3(0, 2, -10));

	int x1 = 0;
	int x2 = 0;
	for (int i = 0; i < noEnemies; i++) {
		
		if (i % 3 == 0) {
			AddEnemyToWorld(Vector3(x1 - 10, 5, 0), leftPath);
			x1 -= 10;
		}

		if (i % 3 == 1) {
			AddEnemyToWorld(Vector3(0, 5, 0), middlePath);
		}

		if (i % 3 == 2) {
			AddEnemyToWorld(Vector3(x2 + 10, 5, 0), rightPath);
			x2 += 10;
		}
	}
	//AddEnemyToWorld(Vector3(-20, 5, 0), leftPath);
	//AddEnemyToWorld(Vector3(20, 5, 0), rightPath);
	//AddEnemyToWorld(Vector3(-10, 5, 0), rightPath);
	//AddEnemyToWorld(Vector3(0, 5, 10), middlePath);



	AddStateObjectToWorld(Vector3(0, 2, -30));
	AddStateObjectToWorld(Vector3(0, 2, -60));
	AddStateObjectToWorld(Vector3(0, 2, -90));
	AddStateObjectToWorld(Vector3(0, 2, -120));

	BridgeConstraintTest();

	AddFloorToWorld(Vector3(0, 0, 0), Vector3(80, 2, 20), Vector4(0.6, 0, 0.6, 1), 0.5);
	AddFloorToWorld(Vector3(-60, 0, -20), Vector3(20, 1.9, 20), Vector4(1, 1, 1, 1), 0.5);
	AddFloorToWorld(Vector3(0, 0, -77.5), Vector3(2, 1.9, 55), Vector4(1, 1, 1, 1), 0.3);
	AddFloorToWorld(Vector3(60, 0, -20), Vector3(20, 1.9, 20), Vector4(1, 1, 1, 1), 0.5);
	AddOBBFloorToWorld(Vector3(-60, 7, -60), Vector3(20, 1.9, 30), Vector3(1, 0, 0), 20, Vector4(1, 1, 1, 1), 0.5);
	AddOBBFloorToWorld(Vector3(60, 6.99, -60), Vector3(20, 1.9, 30), Vector3(1, 0, 0), 20, Vector4(1, 1, 1, 1), 0.5);
	AddOBBFloorToWorld(Vector3(-60, 6.99, -115), Vector3(20, 1.9, 30), Vector3(1, 0, 0), -20, Vector4(1, 1, 1, 1), 0.5);
	AddOBBFloorToWorld(Vector3(60, 7, -115), Vector3(20, 1.9, 30), Vector3(1, 0, 0), -20, Vector4(1, 1, 1, 1), 0.5);
	//AddFloorToWorld(Vector3(-60, 17, -91), Vector3(20, 1.9, 5), Vector4(1, 0, 0, 1), 0.8);
	//AddFloorToWorld(Vector3(60, 17, -91), Vector3(20, 1.9, 5), Vector4(1, 0, 0, 1), 0.8);
	AddFloorToWorld(Vector3(0, 0, -165), Vector3(80, 1.8, 30), Vector4(1, 0.75, 0.8, 1), 3.0);

	AddFloorToWorld(Vector3(0, 0, -240), Vector3(80, 1.8, 20), Vector4(1,1,1,1), 0.5);
	AddOBBFloorToWorld(Vector3(0, 28.85, -311), Vector3(40, 2, 60), Vector3(1, 0, 0), 30, Vector4(1, 1, 1, 1), 0.5);
	AddCheckpointToWorld(Vector3(0, 59, -382), Vector3(80, 1.8, 20), Vector4(0, 0, 0, 1), 0.5);
	AddIceToWorld(Vector3(0, 59, -422), Vector3(80, 1.8, 20), Vector4(1, 1, 1, 1), 0.5);
	AddFloorToWorld(Vector3(60, 59, -472), Vector3(20, 1.9, 30), Vector4(1, 1, 1, 1), 0.5);
	AddFloorToWorld(Vector3(-60, 59, -472), Vector3(20, 1.9, 30), Vector4(1, 1, 1, 1), 0.5);
	AddSwampToWorld(Vector3(0, 59, -472), Vector3(20, 2, 30), Vector4(1, 1, 1, 1), 0.5);
	AddBonusToWorld(Vector3(0, 69, -472), false);
	AddBonusToWorld(Vector3(10, 69, -477), false);
	AddBonusToWorld(Vector3(-10, 69, -477), false);

	AddFloorToWorld(Vector3(-0, 59, -522), Vector3(80, 1.9, 20), Vector4(1, 1, 1, 1), 0.5);

	AddGoalToWorld(Vector3(0, 59, -552), Vector3(80, 1.8, 20), Vector4(0, 1, 0, 1), 0.5);
}

void TutorialGame::BridgeConstraintTest() {
	Vector3 cubeSize = Vector3(3, 1, 6);

	float invCubeMass = 5; //how heavy middle pieces art
	int numLinks = 10;
	float maxDistance = 15; //constraint distance
	float cubeDistance = 12; //distance between links

	Vector3 startPos = Vector3(-70, 0, -210);
	GameObject* start = AddCubeToWorld(startPos + Vector3(0, 0, 0), cubeSize, 0);
	GameObject* end = AddCubeToWorld(startPos + Vector3((numLinks + 2) * cubeDistance, 0, 0), cubeSize, 0);

	GameObject* previous = start;

	for (int i = 0; i < numLinks; ++i) {
		GameObject* block = AddCubeToWorld(startPos + Vector3((i + 1) * cubeDistance, 0, 0), cubeSize, invCubeMass);
		PositionConstraint* constraint = new PositionConstraint(previous, block, maxDistance);
		world->AddConstraint(constraint);
		previous = block;
	}
	PositionConstraint* constraint = new PositionConstraint(previous, end, maxDistance);
	world->AddConstraint(constraint);

}

void TutorialGame::BridgeConstraint() {
	Vector3 cubeSize = Vector3(5, 1, 2);

	float invCubeMass = 5; //how heavy middle pieces art
	int numLinks = 8;
	float maxDistance = 10; //constraint distance
	float cubeDistance = 15; //distance between links

	Vector3 startPos = Vector3(0, 0, -115);
	GameObject* start = AddCubeToWorld(startPos + Vector3(0, 0, 0), cubeSize, 0);
	GameObject* end = AddCubeToWorld(startPos + Vector3(0, 0, (numLinks + 2) * cubeDistance), cubeSize, 0);

	GameObject* previous = start;

	for (int i = 0; i < numLinks; ++i) {
		GameObject* block = AddCubeToWorld(startPos + Vector3(0, 0, (i + 1) * cubeDistance), cubeSize, invCubeMass);
		PositionConstraint* constraint = new PositionConstraint(previous, block, maxDistance);
		world->AddConstraint(constraint);
		previous = block;
	}
	PositionConstraint* constraint = new PositionConstraint(previous, end, maxDistance);
	world->AddConstraint(constraint);

}

/*

A single function to add a large immoveable cube to the bottom of our world

*/
GameObject* TutorialGame::AddFloorToWorld(const Vector3& position, Vector3 size, Vector3 colour, float e) {
	GameObject* floor = new GameObject();
	floor->SetRestitution(e);
	//floor->GetPhysicsObject()->SetFriction(f);

	Vector3 floorSize = size;
	AABBVolume* volume	= new AABBVolume(floorSize);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform()
		.SetScale(floorSize * 2)
		.SetPosition(position);

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, basicTex, basicShader));
	floor->GetRenderObject()->SetColour(colour);
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(floor);

	return floor;
}

GameObject* TutorialGame::AddCheckpointToWorld(const Vector3& position, Vector3 size, Vector3 colour, float e) {
	GameObject* floor = new GameObject("checkpoint");
	floor->SetRestitution(e);
	
	//floor->GetPhysicsObject()->SetFriction(f);

	Vector3 floorSize = size;
	AABBVolume* volume = new AABBVolume(floorSize);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform()
		.SetScale(floorSize * 2)
		.SetPosition(position);

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, basicTex, basicShader));
	floor->GetRenderObject()->SetColour(Vector4(0.6, 0, 0.6, 1));
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(floor);

	return floor;
}

GameObject* TutorialGame::AddIceToWorld(const Vector3& position, Vector3 size, Vector3 colour, float e) {
	GameObject* floor = new GameObject();
	floor->SetRestitution(e);
	

	Vector3 floorSize = size;
	AABBVolume* volume = new AABBVolume(floorSize);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform()
		.SetScale(floorSize * 2)
		.SetPosition(position);

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, basicTex, basicShader));
	floor->GetRenderObject()->SetColour(Vector4(0.635, 0.823, 0.874, 1));
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));
	floor->GetPhysicsObject()->SetFriction(0.995f);

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(floor);

	return floor;
}

GameObject* TutorialGame::AddSwampToWorld(const Vector3& position, Vector3 size, Vector3 colour, float e) {
	GameObject* floor = new GameObject();
	floor->SetRestitution(e);


	Vector3 floorSize = size;
	AABBVolume* volume = new AABBVolume(floorSize);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform()
		.SetScale(floorSize * 2)
		.SetPosition(position);

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, basicTex, basicShader));
	floor->GetRenderObject()->SetColour(Vector4(0, 0.392, 0, 1));
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));
	floor->GetPhysicsObject()->SetFriction(0.8f);

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(floor);

	return floor;
}

GameObject* TutorialGame::AddWallToWorld(const Vector3& position, Vector3 size, Vector3 rotation, float angle, Vector3 colour, float e) {
	GameObject* floor = new GameObject();
	floor->SetRestitution(e);

	Vector3 floorSize = size;
	OBBVolume* volume = new OBBVolume(floorSize);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform()
		.SetScale(floorSize * 2)
		.SetPosition(position)
		.SetOrientation(floor->GetTransform().GetOrientation().AxisAngleToQuaterion(rotation, angle));

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, basicTex, basicShader));
	floor->GetRenderObject()->SetColour(colour);
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(floor);

	return floor;
}

GameObject* TutorialGame::AddOBBFloorToWorld(const Vector3& position, Vector3 size, Vector3 rotation, float angle, Vector3 colour, float e) {
	GameObject* floor = new GameObject();
	floor->SetRestitution(e);

	Vector3 floorSize = size;
	OBBVolume* volume = new OBBVolume(floorSize);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform()
		.SetOrientation(floor->GetTransform().GetOrientation().AxisAngleToQuaterion(rotation, angle))
		.SetScale(floorSize * 2)
		.SetPosition(position);
	
	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, basicTex, basicShader));
	floor->GetRenderObject()->SetColour(colour);
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(floor);

	return floor;
}

GameObject* TutorialGame::AddObstaclesToWorld(const Vector3& position) {
	GameObject* floor = new GameObject("floor");

	Vector3 floorSize = Vector3(20, 2, 140);
	AABBVolume* volume = new AABBVolume(floorSize);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	//floor->GetPhysicsObject()->
	floor->GetTransform()
		//.SetOrientation(floor->GetTransform().GetOrientation().AxisAngleToQuaterion(Vector3(1, 0, 0), 30))
		.SetScale(floorSize)
		.SetPosition(position + Vector3(0, 0, -220));

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, basicTex, basicShader));
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));
	floor->GetPhysicsObject()->SetFriction(1.0f);

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(floor);

	GameObject* obbFloor = new GameObject("obbfloor");

	floorSize = Vector3(20, 2, 60);
	OBBVolume* volume2 = new OBBVolume(floorSize);
	obbFloor->SetBoundingVolume((CollisionVolume*)volume2);
	obbFloor->GetTransform()
		.SetScale(floorSize)
		.SetPosition(position + Vector3(0, -15, 315))
		.SetOrientation(obbFloor->GetTransform().GetOrientation().AxisAngleToQuaterion(Vector3(1, 0, 0), 30));

	obbFloor->SetRenderObject(new RenderObject(&obbFloor->GetTransform(), cubeMesh, basicTex, basicShader));
	obbFloor->SetPhysicsObject(new PhysicsObject(&obbFloor->GetTransform(), obbFloor->GetBoundingVolume()));

	obbFloor->GetPhysicsObject()->SetInverseMass(0);
	obbFloor->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(obbFloor);

	return floor;
}

GameObject* TutorialGame::AddGoalToWorld(const Vector3& position, Vector3 size, Vector3 colour, float e) {
	GameObject* floor = new Goal("goal");
	floor->SetRestitution(e);

	Vector3 floorSize = size;
	AABBVolume* volume = new AABBVolume(floorSize);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform()
		.SetScale(floorSize * 2)
		.SetPosition(position);

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, basicTex, basicShader));
	floor->GetRenderObject()->SetColour(colour);
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(floor);

	return floor;
}



/*

Builds a game object that uses a sphere mesh for its graphics, and a bounding sphere for its
rigid body representation. This and the cube function will let you build a lot of 'simple' 
physics worlds. You'll probably need another function for the creation of OBB cubes too.

*/
GameObject* TutorialGame::AddSphereToWorld(const Vector3& position, float radius, float inverseMass) {
	GameObject* sphere = new GameObject("sphere");

	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	sphere->SetBoundingVolume((CollisionVolume*)volume);

	sphere->GetTransform()
		.SetScale(sphereSize)
		.SetPosition(position);

	sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, basicTex, basicShader));
	sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume()));

	sphere->GetPhysicsObject()->SetInverseMass(inverseMass);
	sphere->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(sphere);

	return sphere;
}

GameObject* TutorialGame::AddObstacleToWorld(const Vector3& position, float radius, float inverseMass) {
	GameObject* sphere = new Obstacle();

	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	sphere->SetBoundingVolume((CollisionVolume*)volume);

	sphere->GetTransform()
		.SetScale(sphereSize)
		.SetPosition(position);

	sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, basicTex, basicShader));
	sphere->GetRenderObject()->SetColour(Vector4(1, 0, 0, 0));
	sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume()));

	sphere->GetPhysicsObject()->SetInverseMass(inverseMass);
	sphere->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(sphere);

	return sphere;
}

GameObject* TutorialGame::AddCapsuleToWorld(const Vector3& position, float halfHeight, float radius, Vector3 force, float inverseMass ) {
	GameObject* capsule = new GameObject("capsule");

	CapsuleVolume* volume = new CapsuleVolume(halfHeight, radius);
	capsule->SetBoundingVolume((CollisionVolume*)volume);

	capsule->GetTransform()
		.SetScale(Vector3(radius* 2, halfHeight, radius * 2))
		.SetPosition(position);

	capsule->SetRenderObject(new RenderObject(&capsule->GetTransform(), capsuleMesh, basicTex, basicShader));
	capsule->SetPhysicsObject(new PhysicsObject(&capsule->GetTransform(), capsule->GetBoundingVolume()));

	capsule->GetPhysicsObject()->SetInverseMass(inverseMass);
	capsule->GetPhysicsObject()->InitCubeInertia();
	capsule->GetPhysicsObject()->AddForce(Vector3(0, 0, 100));

	world->AddGameObject(capsule);
	

	return capsule;

}

GameObject* TutorialGame::AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass) {
	GameObject* cube = new GameObject("cube");
	//cube->SetRestitution(e);

	AABBVolume* volume = new AABBVolume(dimensions);

	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(cube);

	return cube;
}

void TutorialGame::InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius) {
	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddSphereToWorld(position, radius, 1.0f);
		}
	}
	//AddFloorToWorld(Vector3(0, -2, 0), Vector3(100, 2, 100));

	//AddOBBFloorToWorld(Vector3(0, -2, 0));
}

void TutorialGame::InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing) {
	float sphereRadius = 1.0f;
	Vector3 cubeDims = Vector3(1, 1, 1);

	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);

			if (rand() % 2) {
				AddCubeToWorld(position, cubeDims);
			}
			else {
				AddSphereToWorld(position, sphereRadius);
			}
		}
	}
}

void TutorialGame::InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims) {
	for (int x = 1; x < numCols+1; ++x) {
		for (int z = 1; z < numRows+1; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddCubeToWorld(position, cubeDims, 1.0f);
		}
	}
}

void TutorialGame::InitDefaultFloor() {
	AddFloorToWorld(Vector3(0, -2, 0), Vector3(100, 2, 100), Vector4(1,1,1,1) ,0.5);
	//AddOBBFloorToWorld(Vector3(0, -10, 0), Vector3(100, 2, 100), Vector3(1, 0, 0), 30);
	AddFloorToWorld(Vector3(0, -2.1, -200), Vector3(20, 2, 140), Vector4(1, 1, 1, 1), 0.5);
	//AddOBBFloorToWorld(Vector3(0, -2, 0), Vector3(20, 2, 60), Vector3(1, 0, 0), 30);
	AddOBBFloorToWorld(Vector3(0, 31.75, -391), Vector3(40, 2, 60), Vector3(1, 0, 0), 30, Vector4(1, 1, 1, 1), 0.5);
	AddFloorToWorld(Vector3(200, -2, 0), Vector3(100, 2, 100), Vector4(1, 0.75, 0.8, 1), 3.0);
	AddIceToWorld(Vector3(-200, -2, 0), Vector3(100, 2, 100), Vector4(1, 1, 1, 1), 0.5);
	AddSwampToWorld(Vector3(-200, -2, -200), Vector3(100, 2, 100), Vector4(1, 1, 1, 1), 0.5);

	//AddObstaclesToWorld(Vector3(0, -2, 0));
}

void TutorialGame::InitGameExamples() {
	AddPlayerToWorld(Vector3(0, 10, 0), true);
	AddSphereToWorld(Vector3(0, 5, 0), 2.0f);
	AddEnemyToWorld(Vector3(5, 5, 0));
	AddBonusToWorld(Vector3(10, 5, 0), false);
	AddBehaviourEnemyToWorld(Vector3(20, 2, 20));
	AddBonusToWorld(Vector3(30, 4, 20), false);



	//BridgeConstraintTest();
}

GameObject* TutorialGame::AddPlayerToWorld(const Vector3& position, bool sphere) {
	float meshSize = 3.0f;
	float inverseMass = 0.5f;

	GameObject* character = new PlayerCharacter(world, "player");

	//PlayerCharacter object = new PlayerCharacter();

	//AABBVolume* volume = new AABBVolume(Vector3(0.3f, 0.85f, 0.3f) * meshSize);

	if (!sphere) {
		CapsuleVolume* volume = new CapsuleVolume(2.5f, 1.0f, true);

		character->SetBoundingVolume((CollisionVolume*)volume);
	}

	else {
		AABBVolume* volume = new AABBVolume(Vector3(2.0f, 2.0f, 2.0f));

		character->SetBoundingVolume((CollisionVolume*)volume);
	}


	character->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	if (rand() % 2) {
		character->SetRenderObject(new RenderObject(&character->GetTransform(), charMeshA, nullptr, basicShader));
	}
	else {
		character->SetRenderObject(new RenderObject(&character->GetTransform(), charMeshB, nullptr, basicShader));
	}
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(character);

	lockedObject = character;

	return character;
}

GameObject* TutorialGame::AddEnemyToWorld(const Vector3& position, NavigationPath path) {
	float meshSize		= 3.0f;
	float inverseMass	= 0.5f;

	//GameObject* enemy = new Enemy();
	float* scorePointer = &score;
	GameObject* character = new Enemy(path, hard, scorePointer, "enemy");

	SphereVolume* volume = new SphereVolume(2.0f);
	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), enemyMesh, nullptr, basicShader));
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(character);

	return character;
}

GameObject* TutorialGame::AddBehaviourEnemyToWorld(const Vector3& position) {
	float meshSize = 3.0f;
	float inverseMass = 0.5f;

	//GameObject* enemy = new Enemy();

	GameObject* character = new ChasingEnemy(world, "enemy");

	SphereVolume* volume = new SphereVolume(2.0f);
	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), enemyMesh, nullptr, basicShader));
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(character);

	return character;
}

GameObject* TutorialGame::AddBonusToWorld(const Vector3& position, bool die) {
	float* scorePointer = &score;
	GameObject* apple = new Coin(scorePointer, die, "coin");

	SphereVolume* volume = new SphereVolume(1.0f);
	apple->SetBoundingVolume((CollisionVolume*)volume);
	apple->GetTransform()
		.SetScale(Vector3(0.25, 0.25, 0.25))
		.SetPosition(position);

	apple->SetRenderObject(new RenderObject(&apple->GetTransform(), bonusMesh, nullptr, basicShader));
	apple->GetRenderObject()->SetColour(Vector4(0.831, 0.686, 0.216, 1));
	apple->SetPhysicsObject(new PhysicsObject(&apple->GetTransform(), apple->GetBoundingVolume()));

	apple->GetPhysicsObject()->SetInverseMass(1.0f);
	apple->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(apple);

	return apple;
}

StateGameObject* TutorialGame::AddStateObjectToWorld(const Vector3& position) {
	StateGameObject* apple = new StateGameObject();

	SphereVolume* volume = new SphereVolume(2.0f);
	apple->SetBoundingVolume((CollisionVolume*)volume);
	apple->GetTransform()
		.SetScale(Vector3(2, 2, 2))
		.SetPosition(position);

	apple->SetRenderObject(new RenderObject(&apple->GetTransform(), sphereMesh, nullptr, basicShader));
	apple->GetRenderObject()->SetColour(Vector4(1, 1, 0, 1));
	apple->SetPhysicsObject(new PhysicsObject(&apple->GetTransform(), apple->GetBoundingVolume()));

	apple->GetPhysicsObject()->SetInverseMass(0.25f);
	apple->GetPhysicsObject()->NoGravity();
	apple->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(apple);

	return apple;
}

/*

Every frame, this code will let you perform a raycast, to see if there's an object
underneath the cursor, and if so 'select it' into a pointer, so that it can be 
manipulated later. Pressing Q will let you toggle between this behaviour and instead
letting you move the camera around. 

*/
bool TutorialGame::SelectObject() {
	if (debug) {
		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::Q)) {
			inSelectionMode = !inSelectionMode;
			if (inSelectionMode) {
				Window::GetWindow()->ShowOSPointer(true);
				Window::GetWindow()->LockMouseToWindow(false);
			}
			else {
				Window::GetWindow()->ShowOSPointer(false);
				Window::GetWindow()->LockMouseToWindow(true);
			}
		}
		if (inSelectionMode) {
			renderer->DrawString("Press Q to change to camera mode!", Vector2(5, 85));

			if (Window::GetMouse()->ButtonDown(NCL::MouseButtons::LEFT)) {
				if (selectionObject) {	//set colour to deselected;
					selectionObject->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
					selectionObject = nullptr;
					lockedObject = nullptr;
				}

				Ray ray = CollisionDetection::BuildRayFromMouse(*world->GetMainCamera());

				RayCollision closestCollision;
				if (world->Raycast(ray, closestCollision, true)) {
					selectionObject = (GameObject*)closestCollision.node;
					selectionObject->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
					return true;
				}
				else {
					return false;
				}
			}
		}
		else {
			renderer->DrawString("Press Q to change to select mode!", Vector2(5, 85));
		}

		if (lockedObject && debug) {
			renderer->DrawString("Press L to unlock object!", Vector2(5, 80));
			float x = lockedObject->GetTransform().GetPosition().x;
			float y = lockedObject->GetTransform().GetPosition().y;
			float z = lockedObject->GetTransform().GetPosition().z;

			renderer->DrawString("Object Position: (" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ")", Vector2(5, 70));
		}

		else if (selectionObject) {
			renderer->DrawString("Press L to lock selected object object!", Vector2(5, 80));
		}
		
		
		

		if (Window::GetKeyboard()->KeyPressed(NCL::KeyboardKeys::L)) {
			if (selectionObject) {
				if (lockedObject == selectionObject) {
					lockedObject = nullptr;
				}
				else {
					lockedObject = selectionObject;
				}
			}

		}
	}

	return false;
}

/*
If an object has been clicked, it can be pushed with the right mouse button, by an amount
determined by the scroll wheel. In the first tutorial this won't do anything, as we haven't
added linear motion into our physics system. After the second tutorial, objects will move in a straight
line - after the third, they'll be able to twist under torque aswell.
*/
void TutorialGame::MoveSelectedObject() {
	if (debug) {
		renderer->DrawString("Click Force:" + std::to_string(forceMagnitude), Vector2(10, 25));
	}
	forceMagnitude += Window::GetMouse()->GetWheelMovement() * 100.0f;

	if (!selectionObject) {
		return;
	}

	if (Window::GetMouse()->ButtonPressed(NCL::MouseButtons::RIGHT)) {
		Ray ray = CollisionDetection::BuildRayFromMouse(*world->GetMainCamera());
		
		RayCollision closestCollision;
		if (world->Raycast(ray, closestCollision, true)) {
			if (closestCollision.node == selectionObject) {
				//selectionObject->GetPhysicsObject()->AddForce(ray.GetDirection() * forceMagnitude);
				selectionObject->GetPhysicsObject()->AddForceAtPosition(ray.GetDirection() * forceMagnitude, closestCollision.collidedAt);
			}
		}
	}

}
