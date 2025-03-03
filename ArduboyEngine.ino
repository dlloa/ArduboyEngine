#include <Arduboy2.h>
#define DEBUG false
#define DEFAULT_FRAMERATE 60

// ENGINE //
#include "gameengine.h"
#include "controller.h"

// GAME //
#include "snake.h"
#include "sprites.h"

//////////////////
// Arduboy Data //
//////////////////
Arduboy2 arduboy;
byte _framerate = DEFAULT_FRAMERATE;

/////////////////////
// Controller Data //
/////////////////////
Controller controller;
ControllerList cl(&controller, 10);

///////////////
// Game Data //
///////////////
Snake snakegame(&cl, &arduboy);
StateMachine<GameStates> stateMachine(&cl, GAME_MENU);
RenderList renderlist(10);

/////////
// WIP //
/////////
Animator testAnim(&arduboy, simpleAnimation, 8, 4, 5);
Animator anotherTestAnim(&arduboy, sprite_allArray, 16, 8, 10);

void setup() {

  // Arduboy Setup //
  arduboy.begin();
  arduboy.initRandomSeed();
  arduboy.setFrameRate(_framerate);

  stateMachine.setState(GAME_MENU);

  // WIP
  testAnim.setPosition(64, 32);
  anotherTestAnim.setPosition(76, 32);
  snakegame.setPosition((128 - 60) / 2, 0);

  // System Controls //
  cl.addControl(BUTTON_JUST_PRESSED, B_BUTTON, &StateMachine<GameStates>::nextStateWrapper, &stateMachine);

  // Render List //
  // renderlist.addRenderable(&testAnim);
  // renderlist.addRenderable(&anotherTestAnim);
  renderlist.addRenderable(&snakegame);

}

void loop() {

  // Arduboy //
  if (!(arduboy.nextFrame())) return;
  arduboy.clear();
  arduboy.pollButtons();

  // Controller //
  controller.update(arduboy.buttonsState());
  cl.runControls();

  // Update //
  ////////////
  // Render //

  switch(stateMachine.getState()){
    case GAME_MENU:
      arduboy.setCursor(0, 0);
      arduboy.print("GAME MENU");
      break;
    case GAME_PLAY:
      if( !stateMachine.isTransitionFinished() ){
        stateMachine.markTransitionFinished();
        snakegame.takeControl();
        cl.addControl(BUTTON_JUST_PRESSED, DOWN_BUTTON | UP_BUTTON, &StateMachine<GameStates>::nextStateWrapper, &stateMachine);
      }

      // Update //
      snakegame.update();

      if( snakegame.bGameOver ){
        stateMachine.setState(GAME_OVER);
      }

      renderlist.renderAll();

      break;
    case GAME_PAUSE:

      //
      if( !stateMachine.isTransitionFinished() ){
        stateMachine.markTransitionFinished();
        cl.clearControls();
        cl.addControl(BUTTON_JUST_PRESSED, A_BUTTON, &StateMachine<GameStates>::previousStateWrapper, &stateMachine);
        cl.addControl(BUTTON_JUST_PRESSED, B_BUTTON, &StateMachine<GameStates>::previousStateWrapper, &stateMachine);
      }

      arduboy.setCursor(0, 0);
      arduboy.print("PAUSE");
      break;
    case GAME_OVER:
      arduboy.setCursor(0, 0);
      arduboy.print("GAME OVER");
      break;
  }

  ///////////
  // Debug //
  ///////////
  if( DEBUG ){
    arduboy.setCursor(0, 0);
    arduboy.print(controller.debugPrint());
  }

  arduboy.display();
}
