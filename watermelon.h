#ifndef WATERMELON
#define WATERMELON
const int SCALE_FACTOR = 1000; // Represents 1.0 as 1000

enum class ReelStates {
    STATE_MIN,
    REEL_STOPPED = STATE_MIN,
    REEL_STARTING,
    REEL_SPINNING,
    REEL_STOPPING,
    REEL_NUDGING,
    STATE_MAX = REEL_NUDGING
};

class Reel : public Renderable, public Controllable, public Updateable {
public:

    Reel(Arduboy2* inArduboy, ControllerList* inControllerList, const unsigned char** inSymbols, int* inSymbolIDs, int inNumSymbols, int inSymbolSize, int inVisibleSymbols, int inFrameRate, int inSpinUpRate, int inSpinDownRate, int inMinSpinFrames, int inMaxSpinFrames)
        : Renderable(inArduboy), Controllable(inControllerList), numSymbols(inNumSymbols), visibleSymbols(inVisibleSymbols), stateMachine(inControllerList, ReelStates::REEL_STOPPED), spinUpRate(inSpinUpRate), spinDownRate(inSpinDownRate), minSpinDuration(inMinSpinFrames), maxSpinDuration(inMaxSpinFrames) {
        // Copy the symbol IDs into the reel
        symbolIDs = new int[numSymbols];
        for (int i = 0; i < numSymbols; ++i) {
            symbolIDs[i] = inSymbolIDs[i];
        }

        // Initialize the Animator objects for the symbols
        symbols = new Animator*[numSymbols];
        symbolSize = inSymbolSize;
        for (int i = 0; i < numSymbols; ++i) {
            symbols[i] = new Animator(inArduboy, &inSymbols[symbolIDs[i]], symbolSize, 1, inFrameRate);
        }
    }

    ~Reel() {
        for (int i = 0; i < numSymbols; ++i) {
            delete symbols[i];
        }
        delete[] symbols;
        delete[] symbolIDs;
    }

    //////////////////
    //Controller Ops//
    //////////////////


    void playButton(){
        if (stateMachine.getState() == ReelStates::REEL_STOPPED) {
            stateMachine.setState(ReelStates::REEL_STARTING);
        }
        else if(stateMachine.getState() == ReelStates::REEL_SPINNING){
            if( spinFrames >= minSpinDuration ){
                stateMachine.setState(ReelStates::REEL_STOPPING);   
            }
            else{
                pendingStop = true;
            }
        }
    }

    void addNudge(int inNudges){
        if( !isSpinning() ){
            nudges += inNudges;   
            stateMachine.setState(ReelStates::REEL_STOPPING);
        }
    }

    //////////////
    // CONTROLS //
    //////////////
    static void NUDGE_UP(void* data){
        Reel* reel = static_cast<Reel*>(data);
        reel -> addNudge(1);
    }
    static void NUDGE_DOWN(void* data){
        Reel* reel = static_cast<Reel*>(data);
        reel -> addNudge(-1);
    }
    static void A_PRESSED(void* data) {
        Reel* reel = static_cast<Reel*>(data);
        reel -> playButton();
    }
    static void B_PRESSED(void* data) {
        Reel* reel = static_cast<Reel*>(data);
        reel -> playButton();
    }

    ///////////////////
    // ArduboyEngine //
    ///////////////////

    void update() override {
        stateMachine.update();

        switch (stateMachine.getState()) {
            case ReelStates::REEL_STOPPED:
                pendingStop = false;
                // Reel is fully stopped, no action needed
                break;

            case ReelStates::REEL_STARTING:
                handleReelUpdate();
                handleReelSpinUp();

                if(currentSpinSpeed >= spinSpeed) {
                    currentSpinSpeed = spinSpeed;
                    spinFrames = 0;
                    stateMachine.setState(ReelStates::REEL_SPINNING);
                }

                break;
			case ReelStates::REEL_SPINNING:
                spinFrames += 1;
                if( spinFrames >= maxSpinDuration ){
                    stateMachine.setState(ReelStates::REEL_STOPPING);
                }
                else if( pendingStop && spinFrames >= minSpinDuration ){
                    stateMachine.setState(ReelStates::REEL_STOPPING);
                }

                handleReelUpdate();

	            break;

	        case ReelStates::REEL_STOPPING:
                handleReelUpdate();
                handleReelSpinDown();
                break;

            case ReelStates::REEL_NUDGING:
                if( nudges != 0 ){
                    handleNudge();
                }
                else{
                    stateMachine.setState(ReelStates::REEL_STOPPED);
                }
			    break;
        }
    }


    void render() override {
        int numSymbolsToRender = visibleSymbols;
        if (isSpinning()) {
            numSymbolsToRender += 2; // Render two extra symbols during spinning
        }

        for (int i = 0; i < numSymbolsToRender; ++i) {
            int symbolIndex = (currentPosition + i + numSymbols - 1) % numSymbols; // Adjust for extra symbols
            int baseYOffset = i * symbolSize; // Base offset for each symbol
            int fractionalOffset = (subPosition * symbolSize) / SCALE_FACTOR; // Fractional offset
            int yOffset = baseYOffset - fractionalOffset - symbolSize; // Adjust for extra symbols at the top

            symbols[symbolIndex]->setPosition(posX, posY + yOffset);
            symbols[symbolIndex]->render();
        }

        renderDebugOutput();

    }

    void takeControl() override {
		addControl(BUTTON_JUST_PRESSED, A_BUTTON, &Reel::A_PRESSED, this);
	    addControl(BUTTON_JUST_PRESSED, B_BUTTON, &Reel::B_PRESSED, this);

        addControl(BUTTON_JUST_PRESSED, UP_BUTTON,   &Reel::NUDGE_UP, this);
        addControl(BUTTON_JUST_PRESSED, DOWN_BUTTON, &Reel::NUDGE_DOWN, this);
    }

    ////////////////////////////////
    // Reel Op Helpers/Calcs/Etc. //
    ////////////////////////////////

    void handleReelUpdate(){
        subPosition += currentSpinSpeed * spinDirection;

        // Handle wrap-around for subPosition
        if (subPosition >= SCALE_FACTOR) {
            currentPosition = (currentPosition + spinDirection + numSymbols) % numSymbols;
            subPosition -= SCALE_FACTOR;
        } else if (subPosition < 0) {
            currentPosition = (currentPosition + spinDirection + numSymbols) % numSymbols;
            subPosition += SCALE_FACTOR;
        }
    }

    void handleReelSpinUp(){
        if(currentSpinSpeed < spinSpeed) {
            currentSpinSpeed = min(currentSpinSpeed + spinUpRate, spinSpeed);
        }
    }

    void handleReelSpinDown(){
        if (currentSpinSpeed > 0) {
            currentSpinSpeed = max(currentSpinSpeed - spinDownRate, 0);
        } else {
            // Snap to nearest symbol using midpoint (500) and spin direction
            if (subPosition * spinDirection >= (SCALE_FACTOR / 2)) {
                // Move one full step in the spin direction
                currentPosition = (currentPosition + spinDirection + numSymbols) % numSymbols;
            }
            // Reset to clean position
            subPosition = 0;
            currentSpinSpeed = 0;
            stateMachine.setState(ReelStates::REEL_NUDGING);
        }
    }

    void handleNudge(){
        //Nudge in the direction of the nudge
        subPosition += nudgeSpeed * (nudges > 0 ? 1 : -1);

        //Wrap around with nudge
        if (subPosition >= SCALE_FACTOR) {
            currentPosition = (currentPosition + 1 + numSymbols) % numSymbols;
            subPosition -= SCALE_FACTOR;
            nudges -= 1;
        } else if (subPosition * -1 >= SCALE_FACTOR) {
            currentPosition = (currentPosition - 1 + numSymbols) % numSymbols;
            subPosition += SCALE_FACTOR;
            nudges += 1;
        }

        //Clean values
        if( nudges == 0 ){
            subPosition = 0;
        }
    }

    /////////////
    // Getters //
    /////////////

    bool isSpinning() const {
        return stateMachine.getState() == ReelStates::REEL_SPINNING ||
               stateMachine.getState() == ReelStates::REEL_STOPPING ||
               stateMachine.getState() == ReelStates::REEL_NUDGING;
    }

    int getCurrentPosition() const {
        return currentPosition;
    }

    int getVisibleSymbolID(int index) const {
        if (index >= 0 && index < visibleSymbols) {
            int symbolIndex = (currentPosition + index) % numSymbols;
            return symbolIDs[symbolIndex];
        }
        return -1; // Invalid index
    }


    ReelStates getState() const {
        return stateMachine.getState();
    }

    /////////////
    // Setters //
    /////////////

    void setPosition(int x, int y){
        posX = x;
        posY = y;
    }

    void setSpinDirection(int direction) {
        if (direction == 1 || direction == -1) {
            spinDirection = direction;
        }
    }

    void setSpinRates(int upRate, int downRate) {
        spinUpRate = upRate;
        spinDownRate = downRate;
    }

    void setspinSpeed(int speed) {
        spinSpeed = speed;
    }

    void setDebugOutput(bool b){
        debugOutput = b;
    }

    /////////////////
    // Other Utils //
    /////////////////
    void renderDebugOutput(){
        if(debugOutput){
            // Debugging information (optional)
            arduboy->setCursor(posX + symbolSize + 2, posY - 16 + 4);
            arduboy->print(currentPosition);

            arduboy->setCursor(posX + symbolSize + 2, posY - 8 + 4);
            switch (stateMachine.getState()) {
                case ReelStates::REEL_STOPPED:
                    arduboy->print("STPD");
                    break;
                case ReelStates::REEL_STARTING:
                    arduboy->print("STR");
                    break;
                case ReelStates::REEL_SPINNING:
                    arduboy->print("SPN");
                    break;
                case ReelStates::REEL_STOPPING:
                    arduboy->print("STPN");
                    break;
                case ReelStates::REEL_NUDGING:
                    arduboy->print("NDGE");
                    break;
            }

            arduboy->setCursor(posX + symbolSize + 2, posY + 4);
            arduboy->print(pendingStop ? "t" : "f");
            arduboy->setCursor(posX + symbolSize + 2, posY + 8 + 4);
            arduboy->print(currentSpinSpeed);
            arduboy->setCursor(posX + symbolSize + 2, posY + 16 + 4);
            arduboy->print(nudges);
            // arduboy->setCursor(posX + symbolSize + 2, posY + 24 + 4);
            // arduboy->print(minSpinDuration);
        }
    }

private:
    Animator** symbols;       // Array of Animator objects for the symbols
    int* symbolIDs;           // Array of symbol IDs (e.g., 0, 1, 2, 3, etc.)
    int symbolSize;
    int numSymbols;           // Total number of symbols on the reel
    int visibleSymbols;       // Number of symbols visible at a time
    int currentPosition = 0;  // Current position of the reel
    int spinSlowdownCounter = 0; // Counter for slowing down the reel
    const int spinSlowdownFrames = 30; // Number of frames to slow down before stopping

    int posX = 0; // X position of the reel
    int posY = 0; // Y position of the reel
    int spinDirection = 1;

    int subPosition = 0; // Scaled by SCALE_FACTOR (e.g., 1000 = 1.0)
    int spinSpeed = 150; // Scaled by SCALE_FACTOR (e.g., 100 = 0.1)
    int currentSpinSpeed = 0;
    int spinUpRate = 50;
    int spinDownRate = 10;
    int nudgeSpeed = 30;

    uint32_t spinStartFrame = 0;    // Track when spinning started
    bool pendingStop = false;       // Track if stop was requested early
    int minSpinDuration = 60;  // Minimum spin duration (frames)
    int maxSpinDuration = 300; // Maximum spin duration (frames)
    bool debugOutput = false;
    int debugID = 0;
    int spinFrames = 0;

    int nudges = 0;

    StateMachine<ReelStates> stateMachine; // State machine for managing reel states
};


#endif