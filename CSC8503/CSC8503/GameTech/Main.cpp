#include "../../Common/Window.h"

#include "../CSC8503Common/StateMachine.h"
#include "../CSC8503Common/StateTransition.h"
#include "../CSC8503Common/State.h"

#include "../CSC8503Common/NavigationGrid.h"

#include "TutorialGame.h"
#include "../CSC8503Common/PushdownMachine.h"
#include "../CSC8503Common/PushdownState.h"
#include "../CSC8503Common/BehaviourAction.h"
#include "../CSC8503Common/BehaviourSequence.h"
#include "../CSC8503Common/BehaviourSelector.h"

using namespace NCL;
using namespace CSC8503;

/*

The main function should look pretty familar to you!
We make a window, and then go into a while loop that repeatedly
runs our 'game' until we press escape. Instead of making a 'renderer'
and updating it, we instead make a whole game, and repeatedly update that,
instead. 

This time, we've added some extra functionality to the window class - we can
hide or show the 

*/

void TestStateMachine() {
	StateMachine* testMachine = new StateMachine();
	int data = 0;
	State* A = new State([&](float dt)->void
		{
			std::cout << "I'm in state A!\n";
			data++;
		}
	);

	State* B = new State([&](float dt)->void
		{
			std::cout << "I'm in state B!\n";
			data--;
		}
	);

	StateTransition* stateAB = new StateTransition(A, B, [&](void)->bool
		{
			return data > 10;
		}
	);
	StateTransition* stateBA = new StateTransition(B, A, [&](void)->bool
		{
			return data < 0;
		}
	);

	testMachine->AddState(A);
	testMachine->AddState(B);
	testMachine->AddTransition(stateAB);
	testMachine->AddTransition(stateBA);

	for (int i = 0; i < 100; ++i) {
		testMachine->Update(1.0f);
	}
}

vector<Vector3> testNodes;
void TestPathfinding() {
	NavigationGrid grid("TestGrid3.txt");

	NavigationPath outPath;

	Vector3 startPos(10, 0, 130);
	Vector3 endPos(70, 0, 70);

	bool found = grid.FindPath(startPos, endPos, outPath);

	Vector3 pos;
	while (outPath.PopWaypoint(pos)) {
		testNodes.push_back(pos);
	}
}

void DisplayPathfinding() {
	for (int i = 1; i < testNodes.size(); i++) {
		Vector3 a = testNodes[i - 1];
		Vector3 b = testNodes[i];

		Debug::DrawLine(a, b, Vector4(0, 1, 0, 1));
	}
}

class PauseScreen : public PushdownState {
public:
	PauseScreen(Window* win, TutorialGame* gam) {
		w = win;
		g = gam;
	}
	PushdownResult OnUpdate(float dt, PushdownState** newState) override {
		Debug::Print("Press U to unpause game!", Vector2(5, 5));
		w->SetTitle("Paused Game! Gametech frame time: " + std::to_string(1000.0f * dt));
		//g->PausedGame();
		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::U)) {
			return PushdownResult::Pop;
		}
		else {
			return PushdownResult::NoChange;
		}
		
	}
	void OnAwake() override {
		g->FlipPause();
	}
protected:
	Window* w;
	TutorialGame* g;
};

class GameScreen : public PushdownState {
public:
	GameScreen(Window* win, TutorialGame* gam) {
		w = win;
		g = gam;
	}
	PushdownResult OnUpdate(float dt, PushdownState** newState) override {

		//float dt = w->GetTimer()->GetTimeDeltaSeconds();
		if (!g->complete) {
			Debug::Print("Press P to pause game!", Vector2(5, 5));
			Debug::Print("Press F4 to return to menu!", Vector2(5, 10));
			if (Window::GetKeyboard()->KeyDown(KeyboardKeys::P)) {
				*newState = new PauseScreen(w, g);
				return PushdownResult::Push;
			}
			if (Window::GetKeyboard()->KeyDown(KeyboardKeys::F4)) {
				std::cout << "Returning to main menu!\n";
				return PushdownResult::Pop;
			}
			//if (g->complete) {
				//std::cout << "Displaying Results!";
				//return PushdownResult::Pop;
				//*newState = new ResultsScreen(w, g);

			//}
			DisplayPathfinding();

			if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::PRIOR)) {
				w->ShowConsole(true);
			}
			if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::NEXT)) {
				w->ShowConsole(false);
			}

			if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::T)) {
				w->SetWindowPosition(0, 0);
			}
		}

		if (g->complete) {
			string scoreStr = std::to_string(g->score);

			//g->FlipPause();

			if (g->won) {
				Debug::Print("You won!", Vector2(40, 5));
				Debug::Print("Final Score: " + scoreStr, Vector2(25, 15));
				Debug::Print("Press F4 to return to the main menu!", Vector2(15, 25));
			}
			else {
				Debug::Print("You lost!", Vector2(40, 5));
				Debug::Print("Press F4 to return to the main menu and try again!", Vector2(10, 15));
			}

			if (Window::GetKeyboard()->KeyDown(KeyboardKeys::F4)) {
				std::cout << "Returning to main menu!\n";
				g->won = false;
				g->complete = false;
				g->debug = false;
				g->score = 1000.0f;
				return PushdownResult::Pop;
			}
		}

		w->SetTitle("Memory Fault! Gametech frame time: " + std::to_string(1000.0f * dt));

		//g->UpdateGame(dt);
	
		
		//if (rand() % 7 == 0) {
		//	coinsMined++;
		//}
		return PushdownResult::NoChange;
	};
	void OnAwake() override {
		g->FlipPause();
	}
protected:
	Window* w;
	TutorialGame* g;
	int coinsMined = 0;
	float pauseReminder = 1;
};

class IntroScreen : public PushdownState {
public:
	IntroScreen(Window* win, TutorialGame* gam) {
		w = win;
		g = gam;
	}
	
	PushdownResult OnUpdate(float dt, PushdownState** newState) override {
		Debug::Print("Welcome to a really awesome game!!!", Vector2(10, 10));
		Debug::Print("Press 5 to start level 1 - Obstacle Course", Vector2(10, 15));
		Debug::Print("Press 6 to start level 2 - Maze", Vector2(10, 20));
		Debug::Print("Press 7 to start level 3 - Sandbox World", Vector2(10, 25));
		Debug::Print("Press H to turn on Hard Mode!", Vector2(10, 30));
		Debug::Print("Press Escape to quit!", Vector2(10, 35));
		Debug::Print("Use up/down arrows to select no. of enemies...", Vector2(10, 40));
		Debug::Print("...on the obstacle course stage!", Vector2(10, 45));
		Debug::Print("No. Enemies: " + std::to_string(g->noEnemies), Vector2(10, 50));
		Debug::Print("Controls:", Vector2(10, 70));
		Debug::Print("Arrow keys to move!", Vector2(10, 75));
		Debug::Print("Space to jump!", Vector2(10, 80));
		Debug::Print("Q/E/2/3 to rotate your character!", Vector2(10, 85));
		Debug::Print("Left click in debug mode to select an object!", Vector2(10, 90));
		Debug::Print("WASD to move the camera in debug mode!", Vector2(10, 95));


		if(g->hard){
			Debug::Print("Hard Mode on!", Vector2(10, 55));
		}

		if (!g->hard) {
			Debug::Print("Hard Mode off!", Vector2(10, 55));
		}

		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::NUM7)) {
			*newState = new GameScreen(w, g);
			g->InitWorld();
			return PushdownResult::Push;
		}

		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::NUM5)) {
			*newState = new GameScreen(w, g);
			g->InitCourse();
			return PushdownResult::Push;
		}

		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::NUM6)) {
			*newState = new GameScreen(w, g);
			g->InitWorld2();
			return PushdownResult::Push;
		}

		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::NUM7)) {
			*newState = new GameScreen(w, g);
			g->InitWorld();
			return PushdownResult::Push;
		}

		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::UP) && g->noEnemies < 4) {
			g->noEnemies++;
		}

		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::DOWN) && g->noEnemies > 0) {
			g->noEnemies--;
		}

		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::H)) {
			if (g->hard) {
				g->hard = false;
			}
			else {
				g->hard = true;
			}
		}

		//if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::)) {
		//	*newState = new GameScreen(w, g);
		//	return PushdownResult::Push;
		//}
		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::ESCAPE)) {
			return PushdownResult::Pop;
		}
		

		return PushdownResult::NoChange;
	};
	void OnAwake() override {
		//g->FlipPause();
		//Debug::Print("Welcome to a really awesome game!!!", Vector2(50, 50));
		//Debug::Print("Press Space to begin or Escape to quit!", Vector2(50, 60));
		//std::cout << "Welcome to a really awesome game!!!!!!!!!\n";
		//std::cout << "Press Space to begin or Escape to quit!\n";
	}

protected:
	Window* w;
	TutorialGame* g;
};

void TestPushdownAutomata(Window* w, TutorialGame* g) {
	PushdownMachine machine(new IntroScreen(w, g));
	
	
	while (w->UpdateWindow() && !Window::GetKeyboard()->KeyDown(KeyboardKeys::ESCAPE)) {
		float dt = w->GetTimer()->GetTimeDeltaSeconds();
		if (!machine.Update(dt)) {
			return;
		}
		if (dt > 0.1f) {
			std::cout << "Skipping large time delta" << std::endl;
			continue; //must have hit a breakpoint or something to have a 1 second frame time!
		}
		g->UpdateGame(dt);
		//Debug::Print("Press L to start the Game!", Vector2(5, 5));


		/*
		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::PRIOR)) {
			w->ShowConsole(true);
		}
		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::NEXT)) {
			w->ShowConsole(false);
		}

		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::T)) {
			w->SetWindowPosition(0, 0);
		}

		w->SetTitle("Memory Fault! Gametech frame time: " + std::to_string(1000.0f * dt));

		*/
		
	}
	

}

void TestBehaviourTree() {
	float behaviourTimer;
	float distanceToTarget;
	BehaviourAction* findKey = new BehaviourAction("Find Key", [&](float dt, BehaviourState state)->BehaviourState {
		if (state == Initialise) {
			std::cout << "Looking for a key!\n";
			behaviourTimer = rand() % 100;
			state = Ongoing;
		}
		else if (state == Ongoing) {
			behaviourTimer -= dt;
			if (behaviourTimer <= 0.0f) {
				std::cout << "Found a key!\n";
				return Success;
			}
		}
		return state;
		}
	);

	BehaviourAction* goToRoom = new BehaviourAction("Go To Room", [&](float dt, BehaviourState state)->BehaviourState {
		if (state == Initialise) {
			std::cout << "Going to the loot room!\n";
			state = Ongoing;
		}
		else if (state == Ongoing) {
			distanceToTarget -= dt;
			if (distanceToTarget <= 0.0f) {
				std::cout << "Reached room!\n";
				return Success;
			}
		}
		return state; //will be 'ongoing' until success
		}
	);

	BehaviourAction* openDoor = new BehaviourAction("Open Door", [&](float dt, BehaviourState state)->BehaviourState {
		if (state == Initialise) {
			std::cout << "Opening Door!\n";
			return Success;
		}
		return state;
		}
	);

	BehaviourAction* lookForTreasure = new BehaviourAction("Look for Treasure", [&](float dt, BehaviourState state)->BehaviourState {
		if (state == Initialise) {
			std::cout << "Looking for treasure!\n";
			return Ongoing;
		}
		else if (state == Ongoing) {
			bool found = rand() % 2;
			if (found) {
				std::cout << "I found some treasure!\n";
				return Success;
			}
			std::cout << "No treasure in here...\n";
			return Failure;
		}
		return state;
		}
	);

	BehaviourAction* lookForItems = new BehaviourAction("Look for Items", [&](float dt, BehaviourState state)->BehaviourState {
		if (state == Initialise) {
			std::cout << "Looking for items!\n";
			return Ongoing;
		}
		else if (state == Ongoing) {
			bool found = rand() % 2;
			if (found) {
				std::cout << "I found some items!\n";
				return Success;
			}
			std::cout << "No items in here...\n";
			return Failure;
		}
		return state;
		}
	);

	BehaviourSequence* sequence = new BehaviourSequence("Room Sequence");
	sequence->AddChild(findKey);
	sequence->AddChild(goToRoom);
	sequence->AddChild(openDoor);

	BehaviourSelector* selection = new BehaviourSelector("Loot Selection");
	selection->AddChild(lookForTreasure);
	selection->AddChild(lookForItems);

	BehaviourSequence* rootSequence = new BehaviourSequence("Root Sequence");
	rootSequence->AddChild(sequence);
	rootSequence->AddChild(selection);

	for (int i = 0; i < 5; i++) {
		rootSequence->Reset();
		behaviourTimer = 0.0f;
		distanceToTarget = rand() % 250;
		BehaviourState state = Ongoing;
		std::cout << "We're going on an adventure!\n";
		while (state == Ongoing) {
			state = rootSequence->Execute(1.0f); //fake dt
		}
		if (state == Success) {
			std::cout << "What a successful adventure!\n";
		}
		else if (state == Failure) {
			std::cout << "What a waste of time!\n";
		}
	}
	std::cout << "All done!\n";
}

int main() {
	Window*w = Window::CreateGameWindow("CSC8503 Game technology!", 1280, 720);

	if (!w->HasInitialised()) {
		return -1;
	}	
	srand(time(0));
	w->ShowOSPointer(false);
	w->LockMouseToWindow(true);
	TestPathfinding();

	TutorialGame* g = new TutorialGame();
	w->GetTimer()->GetTimeDeltaSeconds(); //Clear the timer so we don't get a larget first dt!
	TestPushdownAutomata(w, g);
	//TestBehaviourTree();
	/*
	while (w->UpdateWindow() && !Window::GetKeyboard()->KeyDown(KeyboardKeys::ESCAPE)) {
		float dt = w->GetTimer()->GetTimeDeltaSeconds();
		DisplayPathfinding();
		if (dt > 0.1f) {
			std::cout << "Skipping large time delta" << std::endl;
			continue; //must have hit a breakpoint or something to have a 1 second frame time!
		}
		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::PRIOR)) {
			w->ShowConsole(true);
		}
		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::NEXT)) {
			w->ShowConsole(false);
		}

		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::T)) {
			w->SetWindowPosition(0, 0);
		}

		w->SetTitle("Memory Fault! Gametech frame time: " + std::to_string(1000.0f * dt));

		g->UpdateGame(dt);
	}
	*/
	Window::DestroyGameWindow();
}

/*class PauseScreen : public PushdownState {
	PushdownResult OnUpdate(float dt, PushdownState** newState) override {
		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::U)) {
			return PushdownResult::Pop;
		}
		return PushdownResult::NoChange;
	}
	void OnAwake() override {
		std::cout << "Press U to unpause game!\n";
	}
};

class GameScreen : public PushdownState {
	PushdownResult OnUpdate(float dt, PushdownState** newState) override {
		pauseReminder -= dt;
		if (pauseReminder < 0) {
			std::cout << "Coins mined: " << coinsMined << "\n";
			std::cout << "Press P to pause game, or F1 to return to main menu!\n";
			pauseReminder += 1.0f;
		}
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::P)) {
			*newState = new PauseScreen();
			return PushdownResult::Push;
		}
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::F1)) {
			std::cout << "Returning to main menu!\n";
			return PushdownResult::Pop;
		}
		if (rand() % 7 == 0) {
			coinsMined++;
		}
		return PushdownResult::NoChange;
	};
	void OnAwake() override {
		std::cout << "Preparing to mine coins!\n";
	}
protected:
	int coinsMined = 0;
	float pauseReminder = 1;
};

class IntroScreen : public PushdownState {
	PushdownResult OnUpdate(float dt, PushdownState** newState) override {
		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::SPACE)) {
			*newState = new GameScreen();
			return PushdownResult::Push;
		}
		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::ESCAPE)) {
			return PushdownResult::Pop;
		}
		return PushdownResult::NoChange;
	};
	void OnAwake() override {
		std::cout << "Welcome to a really awesome game!!!!!!!!!\n";
		std::cout << "Press Space to begin or Escape to quit!\n";
	}
};*/