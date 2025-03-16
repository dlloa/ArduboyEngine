#ifndef SNAKE_GAME
#define SNAKE_GAME

// Structure to hold a position (x, y)
struct Position {
    int x;
    int y;
};

class SnakeTrail {
public:
    // Constructor
    SnakeTrail(int maxLength) : maxLength(maxLength), currentLength(0) {
        trail = new Position[maxLength]; // Dynamically allocate the array
    }

    // Destructor
    ~SnakeTrail() {
        delete[] trail; // Free the allocated memory
    }

    // Add a new position to the head of the trail
    void pushHead(int x, int y) {
        // Shift all positions down the array
        for (int i = currentLength - 1; i > 0; i--) {
            trail[i] = trail[i - 1];
        }

        // Add the new head position
        trail[0].x = x;
        trail[0].y = y;
    }

    void increaseLength(){
    	currentLength++;
    }

    // Get the position at a specific index
    Position getPosition(int index) const {
        return trail[index];
    }

    // Get the current length of the trail
    int getLength() const {
        return currentLength;
    }

    // Get the maximum length of the trail
    int getMaxLength() const {
        return maxLength;
    }

    bool trailExists(int x, int y){
    	for( int trailIndex = 0; trailIndex < currentLength; trailIndex++ ){
    		if( trail[trailIndex].x == x && trail[trailIndex].y == y ){
    			return true;
    		}
    	}
    	return false;
    }

    bool checkGameOver(){
        for( int trailIndex = 1; trailIndex < currentLength; trailIndex++ ){
            if( trail[0].x == trail[trailIndex].x && trail[0].y == trail[trailIndex].y ){
                return true;
            }
        }
        return false;
    }

private:
    Position* trail; // Array to hold the trail positions
    int maxLength;   // Maximum length of the trail
    int currentLength; // Current length of the trail
};

class Snake : public Controllable, public Updateable, public Renderable {
public:
	//Number of spots on the Grid
    int gridsize = 10;
    //Size of a Block on the Grid
    int blocksize = 6;
    //Position where to draw Grid
    int screenPosX = 0;
    int screenPosY = 0;

    int curX = 0;
    int curY = 0;

    int foodX = 0;
    int foodY = 0;

    int framecounter = 0;
    int updatedelay = 20;
    int direction = DOWN_BUTTON;
    bool justAte = false;
    bool hasEaten = false;
    bool bGameOver = false;

    SnakeTrail* trail;

    // Constructor
    Snake(ControllerList* inControllerList, Arduboy2* arduboy)
        : Controllable(inControllerList), Renderable(arduboy) {
        // Add controls to the ControllerList
        trail = new SnakeTrail(100);

        trail -> pushHead(0,0);
        trail -> pushHead(0,0);
        trail -> pushHead(0,0);

        trail -> increaseLength();
        trail -> increaseLength();
        trail -> increaseLength();

        setRandomFood();
    }

    void takeControl() override{
      clearControls();
      addControl(BUTTON_JUST_PRESSED, UP_BUTTON, &Snake::UP_PRESSED, this);
      addControl(BUTTON_JUST_PRESSED, DOWN_BUTTON, &Snake::DOWN_PRESSED, this);
      addControl(BUTTON_JUST_PRESSED, LEFT_BUTTON, &Snake::LEFT_PRESSED, this);
      addControl(BUTTON_JUST_PRESSED, RIGHT_BUTTON, &Snake::RIGHT_PRESSED, this);
    }

    // Set the position of the snake
    void setPosition(int inX, int inY) {
        screenPosX = inX;
        screenPosY = inY;
    }

    void setRandomFood(){
    	foodX = rand() % gridsize;
    	foodY = rand() % gridsize;
    	while( trail -> trailExists(foodX, foodY) ){
	    	foodX = rand() % gridsize;
	    	foodY = rand() % gridsize;
    	}
    }

    // Static function to decrement X selection
    static void UP_PRESSED(void* data) {
        Snake* snake = static_cast<Snake*>(data);
        if( snake -> direction != DOWN_BUTTON ){
	        snake -> direction = UP_BUTTON;	
        }
    }
    static void DOWN_PRESSED(void* data) {
        Snake* snake = static_cast<Snake*>(data);
        if( snake -> direction != UP_BUTTON ){
        	snake -> direction = DOWN_BUTTON;
        }
    }
    static void LEFT_PRESSED(void* data) {
        Snake* snake = static_cast<Snake*>(data);
        if( snake -> direction != RIGHT_BUTTON ){
        	snake -> direction = LEFT_BUTTON;
        }
    }
    static void RIGHT_PRESSED(void* data) {
        Snake* snake = static_cast<Snake*>(data);
        if( snake -> direction != LEFT_BUTTON ){
        	snake -> direction = RIGHT_BUTTON;
        }
    }

    // Render the snake
    void render() override {
        // arduboy->setCursor(screenPosX + (gridsize * blocksize) + 4, 0);
        // arduboy->print(curX);
        // arduboy->setCursor(screenPosX + (gridsize * blocksize) + 4, 16);
        // arduboy->print(curY);

        for (int y = 0; y < gridsize; y++) {
            for (int x = 0; x < gridsize; x++) {
                arduboy->drawRect(screenPosX + (x * blocksize), screenPosY + (y * blocksize), blocksize, blocksize, WHITE);
            }
        }

        for (int trailIndex = 0; trailIndex < trail -> getLength(); trailIndex++){

        	int xPos = trail -> getPosition(trailIndex).x;
        	int yPos = trail -> getPosition(trailIndex).y;

            arduboy->fillRect(screenPosX + (xPos * blocksize), screenPosY + (yPos * blocksize), blocksize, blocksize, WHITE);
        }

        arduboy -> fillCircle(screenPosX + (foodX * blocksize) + (blocksize / 2), screenPosY + (foodY * blocksize) + (blocksize / 2), (blocksize - 3) /2, WHITE );

    }

    void update() override {
      framecounter++;
      if( framecounter > updatedelay ){
        framecounter = 0;
        switch(direction){
          case(UP_BUTTON):
            curY--;
            if( curY < 0 ){
              curY = 0;
            }
            break;
          case(RIGHT_BUTTON):
            curX++;
            if( curX >= gridsize ){
              curX = gridsize - 1;
            }
            break;
          case(DOWN_BUTTON):
            curY++;
            if( curY >= gridsize ){
              curY = gridsize - 1;
            }
            break;
          case(LEFT_BUTTON):
            curX--;
            if( curX < 0 ){
              curX = 0;
            }
            break;
        }
        if( justAte ){
            justAte = false;
            updatedelay--;
            if( updatedelay == 0 ){
                updatedelay = 1;
            }
            trail -> increaseLength();
            setRandomFood();
        }
        else if( hasEaten ){
            bGameOver = trail -> checkGameOver();
        }
        trail -> pushHead(curX, curY);
      }

      if( curX == foodX && curY == foodY){
        hasEaten = true;
        justAte = true;
      }

    }
};

class Menu : public Controllable, public Renderable{
  public:
    Menu(ControllerList* inControllerList, Arduboy2* arduboy) : Controllable(inControllerList), Renderable(arduboy){

    };
    bool bDisplay = false;
    int nMenuSelection = 0;
    static const int nMaxSelection = 3;

    int getSelection(){
      return nMenuSelection;
    }

    static void incrementSelection(void* data){
      int& nMenuSelection = *static_cast<int*>(data);
      nMenuSelection++;
      if( nMenuSelection >= nMaxSelection ){
        nMenuSelection = 0;
      }
    }
    static void decrementSelection(void* data){
      int& nMenuSelection = *static_cast<int*>(data);
      nMenuSelection--;
      if( nMenuSelection < 0 ){
        nMenuSelection = nMaxSelection - 1;
      }
    }

    void render() override {
      arduboy -> setCursor(64, 0);
      arduboy -> print( this -> getSelection() );
    }

    void takeControl() override {
      clearControls();
      addControl(BUTTON_JUST_PRESSED, UP_BUTTON, &Menu::decrementSelection, this);
      addControl(BUTTON_JUST_PRESSED, DOWN_BUTTON, &Menu::incrementSelection, this);
    }
};

enum class GameStates {
    STATE_MIN,
    GAME_MENU = STATE_MIN,
    GAME_PLAY,
    GAME_PAUSE,
    GAME_OVER,
    STATE_MAX = GAME_OVER, // Keep this as the last element to track the number of states
};


#endif