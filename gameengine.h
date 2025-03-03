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
    int nMaxRenderable;
    int nNumRenderable = 0;
    Renderable** aRenderables;

    // Constructor
    RenderList(int inMaxRenderable){
      nMaxRenderable = inMaxRenderable;
      aRenderables = new Renderable*[inMaxRenderable];
      for (int i = 0; i < inMaxRenderable; i++) {
          aRenderables[i] = nullptr; // Initialize each pointer to nullptr
      }
    }

    void addRenderable(Renderable* inRenderable){
      aRenderables[nNumRenderable] = inRenderable;
      nNumRenderable++;
    }

    // Destructor
    ~RenderList() {
        delete[] aRenderables; // Free the allocated memory
    }

    void renderAll() {
        for (int i = 0; i < nMaxRenderable; i++) {
            if (aRenderables[i] != nullptr) { // Check if the Renderable is valid
                aRenderables[i] -> render(); // Call the render method
            }
        }
    }
};

class Animator : public Renderable{
  public:
    const unsigned char** sprite;
    int frames;
    int size;

    int currentframe = 0;
    int framecounter;
    int framerate;

    int posX = 0;
    int posY = 0;

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

    void render() override {
      arduboy -> drawBitmap(posX, posY, sprite[currentframe], size, size, WHITE);
      framecounter++;
      if( framecounter >= framerate ){
        framecounter = 0;
        currentframe++;
        if( currentframe >= frames ){
          currentframe = 0;
        }
      }
    }
};

#endif