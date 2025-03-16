#ifndef CONTROLLER
#define CONTROLLER

enum ControllerID {
  BUTTON_JUST_PRESSED,
  BUTTON_JUST_RELEASED,
  BUTTON_HELD,
  BUTTON_REPEATING,
  BUTTON_DELAYED_REPEATING,
  ANY_BUTTON_JUST_PRESSED,
  ANY_BUTTON_REPEATING,
  ANY_BUTTON_DELAYED_REPEATING
};

class Controller{
  public:
    byte prevButtons = 0;
    byte currButtons = 0;

    byte currHeld = 0;
    byte currPressed = 0;
    byte currReleased = 0;
    byte currRepeating = 0;

    byte frameCounter = 0;
    byte repeatDelayFrames = 10;
    byte buttonTimers[8] = {0,0,0,0,0,0,0,0};
    byte buttonMemorySize = 10;
    byte buttonMemory[10];
    byte buttonMemoryFrames = 30;
    byte buttonMemoryCurrentFrames = 0;
    bool memoryCleared = true;

    Controller(){};
    Controller(byte inRepeatDelayFrames){
      repeatDelayFrames = inRepeatDelayFrames;
    };

    void setRepeatDelay(byte inFrames){
      repeatDelayFrames = inFrames;
    }
    byte getRepeatDelay(){
      return repeatDelayFrames;
    }

    void update(byte inCurrentButtons){
      frameCounter++;
      prevButtons = currButtons;
      currButtons = inCurrentButtons;

      currHeld = prevButtons & currButtons;
      currReleased = (prevButtons ^ currButtons) & prevButtons;
      currPressed = (prevButtons ^ currButtons) & currButtons;

      for( byte bit = 0; bit < 8; bit++ ){
        if( currHeld & ( 1 << bit ) ){
          if( buttonTimers[bit] < repeatDelayFrames){
            buttonTimers[bit]++;
          }
        }
        if( currReleased & (1 << bit)){
          buttonTimers[bit] = 0;
        }
      }

      for( byte bit = 0; bit < 8; bit++ ){
        if( buttonTimers[bit] >= repeatDelayFrames ){
          currRepeating |= 1 << bit;
        }
        else{
          byte mask = 0b00000001 << bit;
          mask = ~mask;
          currRepeating &= mask;
        }
      }

      if( prevButtons != currButtons ){
        //New button combo pressed
        if( currButtons != 0 ){
          moveButtonMemory();
          addButtonMemory(currButtons);
        }
        buttonMemoryCurrentFrames = 0;
      }

      if( buttonMemoryCurrentFrames < 255 ){
        buttonMemoryCurrentFrames += 1;
      }

      if( currButtons == 0 ){
        if( buttonMemoryCurrentFrames == buttonMemoryFrames ){
          clearButtonMemory();
        }
      }
    }

    bool isID(ControllerID inID, byte buttonToCheck, byte inFrames = 1){
      bool toReturn = false;
      switch(inID){
        case 0: //isButtonJustPressed
          toReturn = isButtonJustPressed(buttonToCheck);
          break;
        case 1: //isButtonJustReleased
          toReturn = isButtonJustReleased(buttonToCheck);
          break;
        case 2: //isButtonHeld
          toReturn = isButtonHeld(buttonToCheck);
          break;
        case 3: //isButtonRepeating
          toReturn = isButtonRepeating(buttonToCheck);
          break;
        case 4: //isButtonDelayedRepeating
          toReturn = isButtonDelayedRepeating(buttonToCheck, inFrames);
          break;
        case 5: //isAnyButtonJustPressed
          toReturn = isAnyButtonJustPressed(buttonToCheck);
          break;
        case 6: //isAnyButtonRepeating
          toReturn = isAnyButtonRepeating(buttonToCheck);
          break;
        case 7: //isAnyButtonDelayedRepeating
          toReturn = isAnyButtonDelayedRepeating(buttonToCheck, inFrames);
          break;
      }
      return toReturn;
    }

    bool isButtonJustPressed(byte buttonToCheck){
      return (currPressed & buttonToCheck) == buttonToCheck;
    }
    bool isButtonJustReleased(byte buttonToCheck){
      return (currReleased & buttonToCheck) == buttonToCheck;
    }
    bool isButtonHeld(byte buttonToCheck){
      return (currHeld & buttonToCheck) == buttonToCheck;
    }
    bool isButtonRepeating(byte buttonToCheck){
      return (currRepeating & buttonToCheck) == buttonToCheck;
    }
    bool isButtonDelayedRepeating(byte buttonToCheck, byte framesToDelay){
      return (frameCounter % framesToDelay == 0) && isButtonRepeating(buttonToCheck);
    }

    bool isAnyButtonJustPressed(byte buttonToCheck){
      return currPressed & buttonToCheck;
    }
    bool isAnyButtonRepeating(byte buttonToCheck){
      return (currRepeating & buttonToCheck);
    }
    bool isAnyButtonDelayedRepeating(byte buttonToCheck, byte framesToDelay){
      return (frameCounter % framesToDelay == 0) && isAnyButtonRepeating(buttonToCheck);
    }

    void addButtonMemory(byte inMemory){
      memoryCleared = false;
      buttonMemory[0] = inMemory;
    }
    void clearButtonMemory(){
      if( !memoryCleared ){
        for( byte memoryIndex=0; memoryIndex < buttonMemorySize; memoryIndex++){
          buttonMemory[memoryIndex] = 0;
        }
      }
      memoryCleared = true;
    }
    void moveButtonMemory(){
      byte temp = buttonMemory[0];
      buttonMemory[0] = 0;
      for( byte memoryIndex=0; memoryIndex < buttonMemorySize; memoryIndex++){
        byte newtemp;
        if( memoryIndex + 1 < buttonMemorySize){
          newtemp = buttonMemory[memoryIndex + 1];
          buttonMemory[memoryIndex + 1] = temp;
          temp = newtemp;
        }
      }
    }

    String memoryPrint(){
      String toReturn = "";
      for( byte memoryIndex=0; memoryIndex < buttonMemorySize; memoryIndex++){
        if( buttonMemory[memoryIndex] != 0 ){
          toReturn += String(buttonMemory[memoryIndex]) + String(",");
        }
      }
      return toReturn;
    }

    String debugPrint(){
      String string = String(prevButtons, BIN) + String("\n");
      string += String(currButtons, BIN) + String("\n");
      string += String(currHeld, BIN) + String("\n");
      string += String(currReleased, BIN) + String("\n");
      string += String(currRepeating, BIN) + String("\n");
      string += String(buttonMemoryCurrentFrames) + String("\n");
      string += String(memoryPrint()) + String("\n");
      return string;
    }

    String quickPrint(){
      String string = String(currButtons, BIN);
      return string;
    }

};

class ControllerList{
  public:
    typedef void (*ControlFunction)(void*);
    Controller* controller;

    static const int MAX_CONTROLS = 12;
    byte listIDs[MAX_CONTROLS];
    byte listControls[MAX_CONTROLS];
    ControlFunction funcControls[MAX_CONTROLS];
    void* funcArgs[MAX_CONTROLS];
    byte numControls = 0;


    ControllerList(Controller* inController){
      controller = inController;
    }

    void addControl(byte inID, byte inControl, ControlFunction inFunc, void* inArgs) {
      if (numControls < MAX_CONTROLS) {
          listIDs[numControls] = inID;
          listControls[numControls] = inControl;
          funcControls[numControls] = inFunc;
          funcArgs[numControls] = inArgs;
          numControls++;
      }
    }

    void clearControls() {
        numControls = 0; // Simply reset the count, no need to delete memory
    }

    void runControls() {
      for (byte controlIndex = 0; controlIndex < numControls; ++controlIndex) {
        if (controller->isID(listIDs[controlIndex], listControls[controlIndex], 1)) {
            funcControls[controlIndex](funcArgs[controlIndex]);
        }
      }
    }

};

#endif