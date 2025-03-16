#ifndef GAME_ENGINE
#define GAME_ENGINE

#include "controller.h"

class Controllable {
public:
    Controllable(ControllerList* inControllerList) : controllerList(inControllerList) {}

    void addControl(byte inID, byte inControl, void (*inFunc)(void*), void* inArgs) {
        controllerList->addControl(inID, inControl, inFunc, inArgs);
    }

    virtual void takeControl() = 0;

    void clearControls() {
        controllerList->clearControls();
    }
    ControllerList* controllerList; // Pointer to the ControllerList
};

class Updateable {
public:
    // Virtual update method (to be overridden by derived classes)
    virtual void update() = 0;
};

class Renderable {
public:
    // Constructor
    Renderable(Arduboy2* inArduboy) : arduboy(inArduboy) {}
    // Virtual render method (to be overridden by derived classes)
    virtual void render() = 0;
    Arduboy2* arduboy; // Pointer to the Arduboy instance
};

class RenderList{
  public:

    static const int MAX_RENDERABLES = 10; // Define a maximum number of renderables
    Renderable* aRenderables[MAX_RENDERABLES];
    int nNumRenderable = 0;
    
    void addRenderable(Renderable* inRenderable) {
        if (nNumRenderable < MAX_RENDERABLES) {
            aRenderables[nNumRenderable] = inRenderable;
            nNumRenderable++;
        }
    }

    void renderAll() {
        for (int i = 0; i < nNumRenderable; i++) {
            aRenderables[i]->render();
        }
    }

};


template <typename StateEnum>
class StateMachine : public Controllable, public Updateable {
public:
    // Constructor
    StateMachine(ControllerList* inControllerList, StateEnum initialState)
        : Controllable(inControllerList), currentState(initialState), bTransitionFinished(true) {
        // Add controls to the ControllerList
        // addControl(BUTTON_JUST_PRESSED, UP_BUTTON, &StateMachine::previousStateWrapper, this);
        // addControl(BUTTON_JUST_PRESSED, DOWN_BUTTON, &StateMachine::nextStateWrapper, this);
    }

    // Set the current state
    void setState(StateEnum newState) {
        if (isValidState(newState)) {
            currentState = newState;
            bTransitionFinished = false; // Transition started
        }
    }

    // Get the current state
    StateEnum getState() const {
        return currentState;
    }

    // Transition to the next state (cyclic)
    void nextState() {
        int nextStateValue = static_cast<int>(currentState) + 1;
        if (nextStateValue > static_cast<int>(StateEnum::STATE_MAX)) {
            nextStateValue = static_cast<int>(StateEnum::STATE_MIN); // Wrap around to the first state
        }
        currentState = static_cast<StateEnum>(nextStateValue);
        bTransitionFinished = false; // Transition started
    }

    // Transition to the previous state (cyclic)
    void previousState() {
        int prevStateValue = static_cast<int>(currentState) - 1;
        if (prevStateValue < static_cast<int>(StateEnum::STATE_MIN)) {
            prevStateValue = static_cast<int>(StateEnum::STATE_MAX); // Wrap around to the last state
        }
        currentState = static_cast<StateEnum>(prevStateValue);
        bTransitionFinished = false; // Transition started
    }

    // Check if the current state matches a specific state
    bool isState(StateEnum state) const {
        return currentState == state;
    }

    // Check if the transition is finished
    bool isTransitionFinished() const {
        return bTransitionFinished;
    }

    // Mark the transition as finished
    void markTransitionFinished() {
        bTransitionFinished = true;
    }

    // Static function to transition to the next state (for ControllerList)
    static void nextStateWrapper(void* data) {
        StateMachine* stateMachine = static_cast<StateMachine*>(data);
        stateMachine->nextState();
    }

    // Static function to transition to the previous state (for ControllerList)
    static void previousStateWrapper(void* data) {
        StateMachine* stateMachine = static_cast<StateMachine*>(data);
        stateMachine->previousState();
    }

    void takeControl() override {

    }

    // Update the state machine
    void update() override {
        // Add any update logic here
    }

private:
    StateEnum currentState; // Current state of the state machine
    bool bTransitionFinished; // Flag to track if the transition is finished

    // Check if a state is valid (within bounds)
    bool isValidState(StateEnum state) const {
        return (static_cast<int>(state) >= static_cast<int>(StateEnum::STATE_MIN) &&
                static_cast<int>(state) <= static_cast<int>(StateEnum::STATE_MAX));
    }
};

class Animator : public Renderable, public Updateable {
  public:
    const unsigned char** sprite;
    int frames;
    int size;

    int currentframe = 0;
    int framecounter;
    int framerate;

    int posX = 0;
    int posY = 0;

    bool bAnimating = false;

    void setPosition(int inX, int inY){
      posX = inX;
      posY = inY;
    }

    Animator(Arduboy2* inArduboy, const unsigned char** inSprite, int inSize, int inFrames, int inFrameRate) : Renderable(inArduboy){
      size = inSize;
      sprite = inSprite;
      framerate = inFrameRate;
      frames = inFrames;
    }

    void setSprite(const unsigned char** newSprite, int newFrames) {
      sprite = newSprite;
      frames = newFrames;
      currentframe = 0;
      framecounter = 0;
    }

    void startAnimation() {
      bAnimating = true;
      currentframe = 0;
      framecounter = 0;
    }

    void stopAnimation() {
      bAnimating = false;
      currentframe = 0;
      framecounter = 0;
    }

    void update() override {
      if( bAnimating ){
        framecounter++;
        if( framecounter >= framerate ){
          framecounter = 0;
          currentframe++;
          if( currentframe >= frames ){
            currentframe = 0;
          }
        }
      }
    }

    void render() override {
      arduboy -> drawBitmap(posX, posY, sprite[currentframe], size, size, WHITE);
    }
};

#endif