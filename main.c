// Connect Four
// Author: Scott Helms

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

/*** define ***/

#define CTRL_KEY(k) ((k)&0x1f)
#define UP "A"
#define DOWN "B"
#define RIGHT "C"
#define LEFT "D"
#define ESC "\x1b["
#define CORNER "H"
#define HIDE "\x1b[?25l"
#define UNHIDE "\x1b[?25h"
#define CLEAR "\x1b[2J"

/*** enum ***/

enum token { EMPTY = -1, RED, YELLOW };

/*** Structures ***/

typedef struct TerminalHandle {
  int successful_initialization;
  int screen_rows;
  int screen_cols;
  struct termios orig_termios;
}TerminalSettingsData;

typedef struct CursorLocation{
	int row;
	int col;
}CursorLocation;

typedef struct GameDataElements {
  int array[7][7];
  int move_counter;
  CursorLocation ConnectFourTitleLocation;
  CursorLocation GameBoardLocation;
  CursorLocation FirstTokenLocation;
  CursorLocation PlayersInitialLocation;
  CursorLocation PlayerTurnStatusBoardLocation;
  CursorLocation DirectionsStatusBoardLocation;
  CursorLocation WinnerStatusBoardLocation;
}GameDataElements;


/*** prototypes ***/

/** init **/

// This function clears screen upon entering program.
int enableRawInputMode(struct termios OriginalTerminal, char* error_message);

// This function returns to previous terminal environment after exiting program.
void disableRawInputMode(GameDataElements* GameData);

// This function initializes the elements of the GameData struct.
void initGameData(GameDataElements* GameData);

// This function graphically represents game in the terminal in ASCII
// characters.
void displayGameBoard(GameDataElements* GameData);

// This function gets the terminal size, which is used to center display.
int getWindowSize(int* out_rows, int* out_cols);

// This function reads an error name upon program failure.
void die();//GameDataElements* GameData, const char* s);

/** Gameplay loop **/

// This function moves cursor above game board in order to drop token.
void playerInput(GameDataElements* GameData);

// This function verifies if atoken is present at the y and x index.
// If token is present, it is displayed on the screen.
char* tokenAt(GameDataElements* GameData, int y, int x);

// This function drops token to the bottom of the array or stacks on top of
// previously placed tokens.
int drop(GameDataElements* GameData, int index);

// This function searches for the presence of a four tokens in a line
// hoizontally, diagonally and vertically.
int connectFourPresent();

/** Game wrapup **/

//  This function displays the endgame status bars and menu upon a player
//  winning.
void endGame();

// This function exits the program when the players using Ctrl-q.
void quitGame();

// This function resets the GameData array and displays a fresh game display.
void resetGame();

/** Status Bars **/

// This function displays a status bar that contains the directions for playing
// the game.
void directionsStatusBar();

// This function displays the status bar that changes between player 1 and 2,
// depending on players turn.
void playerTurnStatusBar();

// This function displays a status bar upon a connect four being found and the
// game being over.
void winnerStatusBar();

/** Misc **/

// This function is a helper function for constructing and executing write() via
// <unistd.h>.
void moveCursor(int amount, char* direction);

// This function ia a helper function for displaying strings to game display by
// executing write().
void display(char* item);

// This function is a helper funtion that the clears the screen.
void clearScreen();

// This function is a helper funtion that hides the cursor.
void hideCursor();

// This function is ahHelper function that unhids the cursor.
void unhideCursor();






//new
TerminalSettingsData initializeTerminalSettings(char* error_message);



void turnOff_iflags(tcflag_t* c_iflag);
void turnOff_oflags(tcflag_t* c_oflag);
void turnOff_cflags(tcflag_t* c_cflag);
void turnOff_lflags(tcflag_t* c_lflag);
void enableTimeOutForRead(struct termios* NewSettings);
int applyNewTerminalSettings(struct termios NewSettings, char* error_message);


CursorLocation determineConnectFourTitleLocation(TerminalSettingsData*  TerminalSettings);
CursorLocation determineGameBoardLocation(TerminalSettingsData*  TerminalSettings);
CursorLocation determineFirstTokenLocation(TerminalSettingsData*  TerminalSettings);
CursorLocation determinePlayersInitialLocation(TerminalSettingsData*  TerminalSettings);




void clearTerminal();
//void centerCursor(TerminalSettingsData* TerminalSettings);
void drawTitle(CursorLocation ConnectFourTitleLocation);
void drawGameBoard(CursorLocation GameBoardLocation);



void putCursorAt(int row, int col);


/*** init ***/


TerminalSettingsData initializeTerminalSettings(char* error_message){
	TerminalSettingsData OldSettings;
	OldSettings.successful_initialization = 0;
	
	// Captures current terminal settings, used to enable raw mode.
	if (tcgetattr(STDIN_FILENO, &OldSettings.orig_termios) == -1) {
		strcat(error_message, "initializeTerminalSettings->tcgetattr");
		OldSettings.successful_initialization = -1;

	}

	// Determine terminal window size to center the game display.
  	if (getWindowSize(&OldSettings.screen_rows, &OldSettings.screen_cols) == -1) {
    		strcat(error_message, "initialize_TerminalSettings->getWindowSize");
		OldSettings.successful_initialization = -1;

  	}

	return OldSettings;
}

//TODO: error processing

int enableRawInputMode(struct termios OriginalTerminal, char* error_message){
	struct termios NewSettings = OriginalTerminal;

	// Documentation for termios.h flags:
  	// pubs.opengroup.org/onlinepubs/000095399/basedefs/termios.h.html
	turnOff_iflags(&NewSettings.c_iflag);
	turnOff_oflags(&NewSettings.c_oflag);
	turnOff_cflags(&NewSettings.c_cflag);
	turnOff_lflags(&NewSettings.c_lflag);
	enableTimeOutForRead(&NewSettings);
	return applyNewTerminalSettings(NewSettings, error_message);

}

void turnOff_iflags(tcflag_t* c_iflag){
	// BRKINT: misc flag, ICRNL: ctrl-m, INPCK: misc flag,
  	// ISTRIP: misc flag, IXON: ctrl_s and ctrl_q
	*c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
}

void turnOff_oflags(tcflag_t* c_oflag){
	// OPOST: output processing /r/n
	*c_oflag &= ~(OPOST);
}

void turnOff_cflags(tcflag_t* c_cflag){
	 // CS8: misc flag
 	 *c_cflag |= (CS8);
}

void turnOff_lflags(tcflag_t* c_lflag){
  	// ECHO:Print text to screen, ICANON:Canonical Mode, IEXTEN & ISIG:ctrl=c
  	// and ctrl-v
  	*c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
}

void enableTimeOutForRead(struct termios* NewSettings){
 	 NewSettings->c_cc[VMIN] = 0;
	 NewSettings->c_cc[VTIME] = 1;
}

//TODO: error processing
int applyNewTerminalSettings(struct termios NewSettings, char* error_message){
//	tcsetattr(STDIN_FILENO, TCSAFLUSH, &NewSettings);

	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &NewSettings) == -1) {

		strcat(error_message, "enableRawMode->applyNewTerminalSettings->tcsetattr");
		return -1;
	}
	return 0;

}


GameDataElements initilizeGameData(TerminalSettingsData* TerminalSettings) {
  	GameDataElements NewGame;

	NewGame.move_counter = 0;

  	// Populates array with 49 EMPTY tokes.
 	int i, j;
 	for (i = 0; i < 7; i++) {
    		for (j = 0; j < 7; j++) {
      			NewGame.array[i][j] = EMPTY; 
    		}
  	}

	NewGame.ConnectFourTitleLocation = determineConnectFourTitleLocation(TerminalSettings);	
	NewGame.GameBoardLocation = determineGameBoardLocation(TerminalSettings);	
	NewGame.FirstTokenLocation = determineFirstTokenLocation(TerminalSettings);
	NewGame.PlayersInitialLocation = determinePlayersInitialLocation(TerminalSettings);		
	
	

	return NewGame;
}

CursorLocation determineConnectFourTitleLocation(TerminalSettingsData*  TerminalSettings){

	CursorLocation Title;
	
	Title.row = (TerminalSettings->screen_cols / 2) - 6;
	Title.col = (TerminalSettings->screen_rows / 2) - 13;

	return Title;
}


CursorLocation determineGameBoardLocation(TerminalSettingsData*  TerminalSettings){

	CursorLocation GameBoard;
	
	GameBoard.row = (TerminalSettings->screen_cols / 2) - 15;
	GameBoard.col = (TerminalSettings->screen_rows / 2) - 6;

	return GameBoard;
}

CursorLocation determineFirstTokenLocation(TerminalSettingsData*  TerminalSettings){

	CursorLocation FirstToken;

	FirstToken.row = (TerminalSettings->screen_cols / 2) - 13;
	FirstToken.col = (TerminalSettings->screen_rows / 2) - 4;

	return FirstToken;
}

CursorLocation determinePlayersInitialLocation(TerminalSettingsData*  TerminalSettings){

	CursorLocation PlayersInitial;

	PlayersInitial.row = (TerminalSettings->screen_cols / 2) - 13;
	PlayersInitial.col = (TerminalSettings->screen_rows / 2) - 6;

	return PlayersInitial;
}

//TODO statusboardLocations









void displayGameBoard(GameDataElements* GameData) {
  clearTerminal();
  drawTitle(GameData->ConnectFourTitleLocation);
  drawGameBoard(GameData->GameBoardLocation);
}




void clearTerminal(){
	hideCursor();
  	clearScreen();
  	moveCursor(0, CORNER);
}



void drawTitle(CursorLocation ConnectFourTitleLocation){

  putCursorAt(ConnectFourTitleLocation.col, ConnectFourTitleLocation.row);  
  display("CONNECT FOUR");	
}


void drawGameBoard(CursorLocation GameBoardLocation){
  putCursorAt(GameBoardLocation.col, GameBoardLocation.row);

  // Draws the game board.
  int i, j;
  for (i = 0; i < 15; i++) {
    moveCursor(1, DOWN);

    if (i % 2 == 0) {
      display("+---+---+---+---+---+---+---+");
    } else {
      for (j = 0; j < 29; j++) {
        if (j % 4 == 0) {
          display("|");
	}else
          display(" ");
      }
     }
    moveCursor(29, LEFT);
  }
}


void displayTokens(GameDataElements* GameData){
	int cursor_col = GameData->FirstTokenLocation.col;
	int cursor_row = GameData->FirstTokenLocation.row;	
	int token_col,  token_row;
	for(token_col=0; token_col<7; token_col++){
		for(token_row=0; token_row<7; token_row++){
			putCursorAt(cursor_col, cursor_row);
			display(tokenAt(GameData, token_row, token_col));
			cursor_row += 4;		
					}
		cursor_row = GameData->FirstTokenLocation.row;
		cursor_col += 2;
	}
}
























/*


void disableRawInputMode(GameDataElements* GameData) {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &GameData->E.orig_termios) == -1) {
    die(GameData, "tcsetattr");
  }
}

*/














//TODO error processing
int getWindowSize(int* out_rows, int* out_cols) {
  struct winsize Ws;

  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &Ws) == -1 || Ws.ws_col == 0) {
    return -1;
  } else {
    *out_cols = Ws.ws_col;
    *out_rows = Ws.ws_row;
    return 0;
  }
}

void die(GameDataElements* GameData, const char* error_message) {
  clearScreen();
  moveCursor(0, CORNER);
  unhideCursor();

 // disableRawInputMode(GameData);

  perror(error_message);
  exit(1);
}

/*** Gameplay Loop ***/

void playerInput(GameDataElements* GameData) {
  // Establishes token type by player turn.
  char* token;
  if (GameData->move_counter % 2 == 0) {
    token = "X";
  } else {
    token = "O";
  }

  putCursorAt(GameData->PlayersInitialLocation.col, GameData->PlayersInitialLocation.row);

  display(token);
  moveCursor(1, LEFT);

  // Loop that contains all player commands to the game.
  int nread;
  char c;
  int player_turn = 1;
  int index = 0;
  while (player_turn) {
    while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
      if (nread == -1 && errno != EAGAIN) {
        die(GameData, "read");
      }
    }
    // If arrow key was used, figures out which one (finding the third
    // character).
    if (c == '\x1b') {
      if (read(STDIN_FILENO, &c, 1) == -1) {
        die(GameData, "read");
      }

      if (c == '[') {
        if (read(STDIN_FILENO, &c, 1) == -1) {
          die(GameData, "read");
        }
      }
    }

    // For exiting the game using Ctrl-q.
    switch (c) {
    case CTRL_KEY('q'):
      clearScreen();
      moveCursor(0, CORNER);
      unhideCursor();
     // disableRawInputMode(GameData);
      exit(0);
      break;

    // 'C' represents the right arrow.
    case 'C':
      // If at the end of the array, wraps to the front of the array.
      if (index == 6) {
        display(" ");
        moveCursor(25, LEFT);
        display(token);
        moveCursor(1, LEFT);
        index = 0;
      } else {
        display(" ");
        moveCursor(3, RIGHT);
        display(token);
        moveCursor(1, LEFT);
        index++;
      }

      break;

    // 'D' represents the left arrow.
    case 'D':
      // If at the front of the array, wraps to the end of the array.
      if (index == 0) {
        display(" ");
        moveCursor(23, RIGHT);
        display(token);
        moveCursor(1, LEFT);
        index = 6;
      } else {
        display(" ");
        moveCursor(5, LEFT);
        display(token);
        moveCursor(1, LEFT);
        index--;
      }

      break;
    // '/r' represents the enter key. This drops the token and end the players
    // turn.
    case '\r':
      display(" "); //FIXME if column is full isn't working right

      if (drop(GameData, index)) {
        player_turn--;
      }

      break;
    }
  }
}

char* tokenAt(GameDataElements* GameData, int col, int row) {
  // RED
  if (GameData->array[row][col] == 0) {
    return "X";
  }

  // YELLOW
  if (GameData->array[row][col] == 1) {
    return "O";
  }

  // EMPTY
  return " ";
}

int drop(GameDataElements* GameData, int index) {
  if (GameData->array[0][index] != EMPTY) {
    return 0;
  }

  int i;
  for (i = 6; i >= 0; i--) {
    if (GameData->array[i][index] == EMPTY) {
      if ((GameData->move_counter % 2) == 0) {
        GameData->array[i][index] = RED;
      } else {
        GameData->array[i][index] = YELLOW;
      }
      break;
    }
  }
  return 1;
}

int connectFourPresent() { return 1; }

/*** Game wrapup ***/

void endGame() {}

void quitGame() {}

void resetGame() {}

/*** Status Bars ***/

void directionsStatusBar() {}

void playerTurnStatusBar() {}

void winnerStatusBar() {}

/*** Misc ***/

void moveCursor(int amount, char* direction) {
  char esc[10] = ESC;
  if (amount > 1) {
    char buffer[10];

    sprintf(buffer, "%d", amount);
    strcat(esc, buffer);
  }
  strcat(esc, direction);

  write(STDOUT_FILENO, esc, sizeof(esc));
}

void putCursorAt(int col, int row){
  char esc[20] = ESC;

  char buffer[10];
  sprintf(buffer, "%d", col);
  strcat(esc, buffer);
  strcat(esc,";");

  sprintf(buffer, "%d", row);
  strcat(esc, buffer);

  strcat(esc, "H");

  write(STDOUT_FILENO, esc, sizeof(esc));
}





void display(char* item) { write(STDOUT_FILENO, item, strlen(item)); }

void clearScreen() { write(STDOUT_FILENO, CLEAR, 4); }

void hideCursor() { write(STDOUT_FILENO, HIDE, 6); }

void unhideCursor() { write(STDOUT_FILENO, UNHIDE, 6); }











/*** main ***/

int main() {
  char error_message[50] = "";

  TerminalSettingsData TerminalSettings = initializeTerminalSettings(error_message);
  if ( TerminalSettings.successful_initialization == -1 ){
	perror(error_message);
  	exit(1);
  }

  if ( enableRawInputMode(TerminalSettings.orig_termios, error_message) == -1 ){
	perror(error_message);
  	exit(1);
  }

  GameDataElements GameData = initilizeGameData(&TerminalSettings);
  displayGameBoard(&GameData);


  while ( strcmp(error_message, "NONE") == 0 ){
    // The function drawGameBoard is prior to the function connectFourPresent so
    // that upon a winning move the last token placement is displayed.
    displayTokens(&GameData);
    connectFourPresent(&GameData);
    playerInput(&GameData);
    GameData.move_counter++; //FIXME  needs to be in a better place
  }

   



  return 0;
}
