#include <Arduboy2.h>
#define DEBUG false
#define DEFAULT_FRAMERATE 60

// ENGINE //
#include "gameengine.h"
#include "controller.h"

// GAME //
// #include "snake.h"
#include "sprites.h"
#include "watermelon.h"

//////////////////
// Arduboy Data //
//////////////////
Arduboy2 arduboy;
byte _framerate = DEFAULT_FRAMERATE;

/////////////////////
// Controller Data //
/////////////////////
Controller controller;
ControllerList cl = ControllerList(&controller);

///////////////
// Game Data //
///////////////
// Snake snakegame(&cl, &arduboy);
// StateMachine<GameStates> stateMachine(&cl, GAME_MENU);
RenderList renderlist = RenderList();

/////////
// WIP //
/////////
// Assuming you have an array of symbols defined in sprites.h
const unsigned char* reelSymbols[] = {
    simpleAnimation1, // ID 0
    simpleAnimation2, // ID 1
    simpleAnimation3, // ID 2
    simpleAnimation4  // ID 3
};

// Define the symbol IDs for each reel
int reel1SymbolIDs[] = {0,1,2,3,4,5,6,7}; // Reel 1 has symbols in order 0, 1, 2, 3
int reel2SymbolIDs[] = {7,6,5,4,3,2,1,1}; // Reel 2 has symbols in order 1, 2, 3, 0
int reel3SymbolIDs[] = {2,3,0,1,7,5,4,6}; // Reel 3 has symbols in order 2, 3, 0, 1

// Create the reels with custom symbol IDs
Reel reel1(&arduboy, &cl, sprite_allArray, reel1SymbolIDs, 8, 16, 3, 5, 10, 3, 120, 300);
Reel reel2(&arduboy, &cl, sprite_allArray, reel2SymbolIDs, 8, 16, 3, 5, 10, 3, 180, 360);
Reel reel3(&arduboy, &cl, sprite_allArray, reel3SymbolIDs, 8, 16, 3, 5, 10, 3, 240, 420);

void setup() {

  // Arduboy Setup //
  arduboy.begin();
  arduboy.initRandomSeed();
  arduboy.setFrameRate(_framerate);

  // WIP
  reel1.setPosition(0, 24); // Set reel1 at (10, 10)
  reel2.setPosition(64 - 8, 24); // Set reel2 at (50, 10)
  reel3.setPosition(128 - 16, 24); // Set reel3 at (90, 10)

  renderlist.addRenderable(&reel1);
  renderlist.addRenderable(&reel2);
  renderlist.addRenderable(&reel3);

  reel1.takeControl();
  reel2.takeControl();
  reel3.takeControl();

  reel1.setSpinDirection(-1);
  reel2.setSpinDirection(-1);
  reel3.setSpinDirection(-1);

  reel1.setDebugOutput(true);
}

void loop() {

  /////////////
  // Arduboy //
  /////////////
  if (!(arduboy.nextFrame())) return;
  arduboy.clear();
  arduboy.pollButtons();

  ////////////////
  // Controller //
  ////////////////
  controller.update(arduboy.buttonsState());
  cl.runControls();

  ////////////
  // Update //
  ////////////
  reel1.update();
  reel2.update();
  reel3.update();

  ////////////
  // Render //
  ////////////
  renderlist.renderAll(); 
  
  // arduboy.fillRect(0, 0, 128, 8, BLACK);
  // arduboy.fillRect(0, 56, 128, 8, BLACK);
  // arduboy.drawFastHLine(0, 8, 128, WHITE);
  // arduboy.drawFastHLine(0, 56, 128, WHITE);

  ///////////
  // Debug //
  ///////////
  if( DEBUG ){
    arduboy.setCursor(0, 0);
    arduboy.print(controller.debugPrint());
  }

  /////////////
  // Arduboy //
  /////////////
  arduboy.display();
}
