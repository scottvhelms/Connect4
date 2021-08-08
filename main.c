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
#define NO_ERRORS ""
#define PLAYER1 "X"
#define PLAYER2 "O"
#define TITLE "CONNECT FOUR"
#define BOARDTOP "+---+---+---+---+---+---+---+"
#define DIRECTIONS_ARROWS "PRESS ARROW KEY TO MOVE THE TOKEN"
#define DIRECTIONS_ENTER "PRESS ENTER KEY TO DROP THE TOKEN"
#define P1TURN "PLAYER 1's TURN"
#define P2TURN "PLAYER 2's TURN"
#define P1WIN "PLAYER 1 IS THE WINNER"
#define P2WIN "PLAYER 2 IS THE WINNER"
#define RED_COLOR "\x1b[31m"
#define YELLOW_COLOR "\x1b[33m"
#define BLUE_COLOR "\x1b[34m"
#define DEFAULT_COLOR "\x1b[39m"
#define BLINKING_ON "\x1b[1;5;7m"
#define BLINKING_OFF "\x1b[m"


/*** enum ***/

enum token { EMPTY = -1, RED, YELLOW };
enum arrow_enter { ENTER = 13, RIGHT_ARROW = 67, LEFT_ARROW = 68 };
enum bounds { LEFT_BOUNDARY = 0, RIGHT_BOUNDARY = 6 };
enum directions{ HORIZONTAL, LEFTDIAG, VERTICAL, RIGHTDIAG};

/*** Structures ***/

typedef struct TermHandle {
  int successful_initialization;
  int screen_rows;
  int screen_cols;
  struct termios orig_termios;
} TermSettingsData;

typedef struct CursorLoc {
  int row;
  int col;
} CursorLoc;

typedef struct GameDataElements {
  int array[7][7];
  int move_counter;
  CursorLoc ConnectFourTitleLoc;
  CursorLoc GameBoardLoc;
  CursorLoc FirstTokenLoc;
  CursorLoc PlayersInitialLoc;
  CursorLoc PlayerTurnStatusBarLoc;
  CursorLoc DirectionsStatusBarLoc;
  CursorLoc WinnerStatusBarLoc;
} GameDataElements;

/*** prototypes ***/

/** init **/

// This function clears screen upon entering program.
int enableRawInputMode(struct termios OriginalTerm, char* error_message);

// This function returns to previous terminal environment after exiting program.
int disableRawInputMode(TermSettingsData* TermSettings, char* error_message);

// This function initializes the elements of the GameData struct.
void initGameData(GameDataElements* GameData);

// This function graphically represents game in the terminal in ASCII
// characters.
void displayGameBoard(GameDataElements* GameData);

// This function gets the terminal size, which is used to center display.
int getWindowSize(int* out_rows, int* out_cols);

// This function reads an error name upon program failure.
void exitProgram(TermSettingsData* TermSettings, char* error_message);

/** Gameplay loop **/

// This function moves cursor above game board in order to drop token.
int playerInput(GameDataElements* GameData, char* error_message);

// This function verifies if atoken is present at the y and x index.
// If token is present, it is displayed on the screen.
void displayTokenAt(int array[7][7], int y, int x);

// This function drops token to the bottom of the array or stacks on top of
// previously placed tokens.
int drop(GameDataElements* GameData, int cur_col_position);

// This function searches for the presence of a four tokens in a line
// hoizontally, diagonally and vertically.
int connectFourPresent(GameDataElements* GameData);

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
void displayDirectionsStatusBar(CursorLoc DirectionsStatusBarLoc);

// This function displays the status bar that changes between player 1 and 2,
// depending on players turn.
void playerTurnStatusBar(GameDataElements* GameData);

// This function displays a status bar upon a connect four being found and the
// game being over.
void displayWinnerStatusBar(GameDataElements* GameData);

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

// new
TermSettingsData initializeTermSettings(char* error_message);

void turnOff_iflags(tcflag_t* c_iflag);
void turnOff_oflags(tcflag_t* c_oflag);
void turnOff_cflags(tcflag_t* c_cflag);
void turnOff_lflags(tcflag_t* c_lflag);
void enableTimeOutForRead(struct termios* NewSettings);
int applyNewTermSettings(struct termios NewSettings, char* error_message);

CursorLoc determineConnectFourTitleLoc(TermSettingsData* TermSettings);
CursorLoc determineGameBoardLoc(TermSettingsData* TermSettings);
CursorLoc determineFirstTokenLoc(TermSettingsData* TermSettings);
CursorLoc determinePlayersInitialLoc(TermSettingsData* TermSettings);
CursorLoc determineDirectionsStatusBarLoc(TermSettingsData* TermSettings);
CursorLoc determinePlayerTurnStatusBarLoc(TermSettingsData* TermSettings);
CursorLoc determineWinnerStatusBarLoc(TermSettingsData* TermSettings);
int textOffsetForCentering(char* text);

void clearTerm();
void displayTitle(CursorLoc ConnectFourTitleLoc);
void drawGameBoard(CursorLoc GameBoardLoc);

char* determineCurPlayersToken(int move_counter);
void displayCurPlayersToken(char* cur_players_token);
int playerInputReader(char* player_input, char* error_message);
void placeTokenAtLeftBoundary(char* cur_players_token, int* cur_position);
void moveTokenRight(char* cur_players_token, int* cur_position);
void placeTokenAtRightBoundary(char* cur_players_token, int* cur_position);
void moveTokenLeft(char* cur_players_token, int* cur_position);

int connectFourHorizontal(int array[7][7], int row, int col);
int connectFourLeftDiagonal(int array[7][7], int row, int col);
int connectFourVertical(int array[7][7], int row, int col);
int connectFourRightDiagonal(int array[7][7], int row, int col);
void showConnectFour(GameDataElements* GameData, int row, int col, int direction);

void putCursorAt(int row, int col);

void displayRedColorText(); 
void displayYellowColorText();
void displayDefaultColorText();
void displayBlueColorText(); 
void enableBlinkingText();
void disableBlinkingText();

/*** init ***/

TermSettingsData initializeTermSettings(char* error_message) {
  TermSettingsData OldSettings;
  OldSettings.successful_initialization = 0;

  // Captures cur terminal settings, used to enable raw mode.
  if (tcgetattr(STDIN_FILENO, &OldSettings.orig_termios) == -1) {
    strcat(error_message, "initializeTermSettings->tcgetattr");
    OldSettings.successful_initialization = -1;
  }

  // Determine terminal window size to center the game display.
  if (getWindowSize(&OldSettings.screen_rows, &OldSettings.screen_cols) == -1) {
    strcat(error_message, "initialize_TermSettings->getWindowSize");
    OldSettings.successful_initialization = -1;
  }

  return OldSettings;
}

int enableRawInputMode(struct termios OriginalTerm, char* error_message) {
  struct termios NewSettings = OriginalTerm;

  // Documentation for termios.h flags:
  // pubs.opengroup.org/onlinepubs/000095399/basedefs/termios.h.html
  turnOff_iflags(&NewSettings.c_iflag);
  turnOff_oflags(&NewSettings.c_oflag);
  turnOff_cflags(&NewSettings.c_cflag);
  turnOff_lflags(&NewSettings.c_lflag);
  enableTimeOutForRead(&NewSettings);
  return applyNewTermSettings(NewSettings, error_message);
}

void turnOff_iflags(tcflag_t* c_iflag) {
  // BRKINT: misc flag, ICRNL: ctrl-m, INPCK: misc flag,
  // ISTRIP: misc flag, IXON: ctrl_s and ctrl_q
  *c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
}

void turnOff_oflags(tcflag_t* c_oflag) {
  // OPOST: output processing /r/n
  *c_oflag &= ~(OPOST);
}

void turnOff_cflags(tcflag_t* c_cflag) {
  // CS8: misc flag
  *c_cflag |= (CS8);
}

void turnOff_lflags(tcflag_t* c_lflag) {
  // ECHO:Print text to screen, ICANON:Canonical Mode, IEXTEN & ISIG:ctrl=c
  // and ctrl-v
  *c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
}

void enableTimeOutForRead(struct termios* NewSettings) {
  NewSettings->c_cc[VMIN] = 0;
  NewSettings->c_cc[VTIME] = 1;
}

int applyNewTermSettings(struct termios NewSettings, char* error_message) {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &NewSettings) == -1) {
    strcat(error_message, "enableRawMode->applyNewTermSettings->tcsetattr");
    return -1;
  }

  return 0;
}

GameDataElements initilizeGameData(TermSettingsData* TermSettings) {
  GameDataElements NewGame;

  NewGame.move_counter = 0;

  // Populates array with 49 EMPTY tokes.
  int i, j;
  for (i = 0; i < 7; i++) {
    for (j = 0; j < 7; j++) {
      NewGame.array[i][j] = EMPTY;
    }
  }

  NewGame.ConnectFourTitleLoc = determineConnectFourTitleLoc(TermSettings);
  NewGame.GameBoardLoc = determineGameBoardLoc(TermSettings);
  NewGame.FirstTokenLoc = determineFirstTokenLoc(TermSettings);
  NewGame.PlayersInitialLoc = determinePlayersInitialLoc(TermSettings);
  NewGame.DirectionsStatusBarLoc = determineDirectionsStatusBarLoc(TermSettings);
  NewGame.PlayerTurnStatusBarLoc = determinePlayerTurnStatusBarLoc(TermSettings);
  NewGame.WinnerStatusBarLoc = determineWinnerStatusBarLoc(TermSettings);

  return NewGame;
}

CursorLoc determineConnectFourTitleLoc(TermSettingsData* TermSettings) {
  CursorLoc Title;

  Title.col = (TermSettings->screen_cols / 2) - textOffsetForCentering(TITLE);
  Title.row = (TermSettings->screen_rows / 2) - 14;

  return Title;
}

CursorLoc determineGameBoardLoc(TermSettingsData* TermSettings) {
  CursorLoc GameBoard;

  GameBoard.col = (TermSettings->screen_cols / 2) - textOffsetForCentering(BOARDTOP);
  GameBoard.row = (TermSettings->screen_rows / 2) - 6;

  return GameBoard;
}

CursorLoc determineFirstTokenLoc(TermSettingsData* TermSettings) {
  CursorLoc FirstToken;

  FirstToken.col = (TermSettings->screen_cols / 2) - (textOffsetForCentering(BOARDTOP)-2);
  FirstToken.row = (TermSettings->screen_rows / 2) - 4;

  return FirstToken;
}

CursorLoc determinePlayersInitialLoc(TermSettingsData* TermSettings) {
  CursorLoc PlayersInitial;

  PlayersInitial.col = (TermSettings->screen_cols / 2) - (textOffsetForCentering(BOARDTOP)-2);
  PlayersInitial.row = (TermSettings->screen_rows / 2) - 6;

  return PlayersInitial;
}

CursorLoc determineDirectionsStatusBarLoc(TermSettingsData* TermSettings) {
  CursorLoc Directions;

  Directions.col = (TermSettings->screen_cols / 2) - textOffsetForCentering(DIRECTIONS_ARROWS);
  Directions.row = (TermSettings->screen_rows / 2) + 12; 

  return Directions;
}

CursorLoc determinePlayerTurnStatusBarLoc(TermSettingsData* TermSettings) {
  CursorLoc PlayerTurn;

  PlayerTurn.col = (TermSettings->screen_cols / 2) - textOffsetForCentering(P1TURN);
  PlayerTurn.row = (TermSettings->screen_rows / 2) - 8; 

  return PlayerTurn;
}

CursorLoc determineWinnerStatusBarLoc(TermSettingsData* TermSettings) {
  CursorLoc WinnerStatusBar;

  WinnerStatusBar.col = (TermSettings->screen_cols / 2) - textOffsetForCentering(P1WIN);
  WinnerStatusBar.row = (TermSettings->screen_rows / 2) - 8;

  return WinnerStatusBar;
}

int textOffsetForCentering(char* text){ 
	return strlen(text)/(2);
}


void displayGameBoard(GameDataElements* GameData) {
  clearTerm();
  displayTitle(GameData->ConnectFourTitleLoc);
  displayDirectionsStatusBar(GameData->DirectionsStatusBarLoc);
  drawGameBoard(GameData->GameBoardLoc);
}

void clearTerm() {
  hideCursor();
  clearScreen();
  moveCursor(0, CORNER);
}

void displayTitle(CursorLoc ConnectFourTitleLoc) {

  putCursorAt(ConnectFourTitleLoc.row, ConnectFourTitleLoc.col);
  display(TITLE);
}

void drawGameBoard(CursorLoc GameBoardLoc) {
  putCursorAt(GameBoardLoc.row, GameBoardLoc.col);
  displayBlueColorText(); 

  int i, j;
  for (i = 0; i < 15; i++) {
    moveCursor(1, DOWN);

    if (i % 2 == 0) {
      display(BOARDTOP);
    } else {
      for (j = 0; j < 29; j++) {
        if (j % 4 == 0) {
          display("|");
        } else
          display(" ");
      }
    }
    moveCursor(29, LEFT);
  }

  displayDefaultColorText();
}


void displayTokens(GameDataElements* GameData) {
  int cursor_col = GameData->FirstTokenLoc.col;
  int cursor_row = GameData->FirstTokenLoc.row;
  int token_col, token_row;
  for (token_col = 0; token_col < 7; token_col++) {
    for (token_row = 0; token_row < 7; token_row++) {
      putCursorAt(cursor_row, cursor_col);
      displayTokenAt(GameData->array, token_col, token_row);
      cursor_row += 2;
    }
    cursor_row = GameData->FirstTokenLoc.row;
    cursor_col += 4;
  }
}

void displayTokenAt(int array[7][7], int col, int row) {
  if (array[row][col] == 0) {
    displayRedColorText();
    write(STDOUT_FILENO, PLAYER1, strlen(PLAYER1));
    displayDefaultColorText();
 }

 else if (array[row][col] == 1) {
    displayYellowColorText();
    write(STDOUT_FILENO, PLAYER2, strlen(PLAYER2));
    displayDefaultColorText();

  } else {

  write(STDOUT_FILENO, " ", 1);
 }
}

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

void exitProgram(TermSettingsData* TermSettings, char* error_message) {
  clearScreen();
  moveCursor(0, CORNER);
  unhideCursor();

  if (disableRawInputMode(TermSettings, error_message) == -1) {
    strcat(error_message,
           " Failed to disable Raw Input Mode. Restart terminal");
  }

  if (strcmp(error_message, NO_ERRORS) == 0) {
    exit(0);
  } else {
    perror(error_message);
    exit(1);
  }
}

int disableRawInputMode(TermSettingsData* TermSettings, char* error_message) {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &TermSettings->orig_termios) == -1) {
    strcat(error_message, "die->disableRawInputMode->tcsetattr");
    return -1;
  }
  return 0;
}

/*** Gameplay Loop ***/

int playerInput(GameDataElements* GameData, char* error_message) {
  char* cur_players_token = determineCurPlayersToken(GameData->move_counter);
  putCursorAt(GameData->PlayersInitialLoc.row, GameData->PlayersInitialLoc.col);
  displayCurPlayersToken(cur_players_token);

  char player_input;
  int cur_player_turn = 1;
  int cur_position = 0;
  while (cur_player_turn) {

    if (playerInputReader(&player_input, error_message) == -1) {
      return 0;
    }

    switch (player_input) {
    case CTRL_KEY('q'):
      return 0;
      break;

    case RIGHT_ARROW:
      if (cur_position == RIGHT_BOUNDARY) {
        placeTokenAtLeftBoundary(cur_players_token, &cur_position);
      } else {
        moveTokenRight(cur_players_token, &cur_position);
      }
      break;

    case LEFT_ARROW:
      if (cur_position == LEFT_BOUNDARY) {
        placeTokenAtRightBoundary(cur_players_token, &cur_position);
      } else {
        moveTokenLeft(cur_players_token, &cur_position);
      }
      break;

    case ENTER:
      if (drop(GameData, cur_position)) {
        cur_player_turn--;
        GameData->move_counter++;
      }
      break;
    }
  }
  return 1;
}

char* determineCurPlayersToken(int move_counter) {
  if (move_counter % 2 == 0) {
    return PLAYER1;
  } else {
    return PLAYER2;
  }
}

void displayCurPlayersToken(char* cur_players_token) {
  if(strcmp (cur_players_token, PLAYER1) == 0 ){
	displayRedColorText();
  } else {
	displayYellowColorText();
  }
  display(cur_players_token);
  moveCursor(1, LEFT);

  displayDefaultColorText();
}

int playerInputReader(char* player_input, char* error_message) {
  int readerOutput;
  while ((readerOutput = read(STDIN_FILENO, player_input, 1)) != 1) {
    if (readerOutput == -1 && errno != EAGAIN) {
      strcat(error_message, "playerInput->read");
      return 0;
    }
  }

  // If arrow key was used, figures out which one (finding the third
  // character).
  if (*player_input == '\x1b') {
    if (read(STDIN_FILENO, player_input, 1) == -1) {
      strcat(error_message, "playerInput->playerInputReader->read");
      return -1;
    }

    if (*player_input == '[') {
      if (read(STDIN_FILENO, player_input, 1) == -1) {
        strcat(error_message, "playerInput->playerInputReader->read");
        return -1;
      }
    }
  }

  return 0;
}

void placeTokenAtLeftBoundary(char* cur_players_token, int* cur_position) {
  display(" ");
  moveCursor(25, LEFT);
  displayCurPlayersToken(cur_players_token);
  *cur_position = LEFT_BOUNDARY;
}

void moveTokenRight(char* cur_players_token, int* cur_position) {
  display(" ");
  moveCursor(3, RIGHT);
  displayCurPlayersToken(cur_players_token);
  *cur_position = *cur_position + 1;
}

void placeTokenAtRightBoundary(char* cur_players_token, int* cur_position) {
  display(" ");
  moveCursor(23, RIGHT);
  displayCurPlayersToken(cur_players_token);
  *cur_position = RIGHT_BOUNDARY;
}

void moveTokenLeft(char* cur_players_token, int* cur_position) {
  display(" ");
  moveCursor(5, LEFT);
  displayCurPlayersToken(cur_players_token);
  *cur_position = *cur_position - 1;
}

int drop(GameDataElements* GameData, int cur_col_position) {
  if (GameData->array[0][cur_col_position] != EMPTY) {
    return 0;
  }

  int row;
  for (row = 6; row >= 0; row--) {
    if (GameData->array[row][cur_col_position] == EMPTY) {
      if ((GameData->move_counter % 2) == 0) {
        GameData->array[row][cur_col_position] = RED;
      } else {
        GameData->array[row][cur_col_position] = YELLOW;
      }
      break;
    }
  }
  display(" ");
  return 1;
}



void showConnectFour(GameDataElements* GameData, int row, int col, int direction){
	enableBlinkingText();
	int i;
	int temp_col = col;
	switch(direction){

	case HORIZONTAL:
		//	int i;
  		//	int temp_col = col;

			// FIXME moveCURSOR here is the fix
  			for (i = 0; i < 4; i++) {
				putCursorAt(GameData->FirstTokenLoc.row+(row*2),GameData->FirstTokenLoc.col+(temp_col*4));
				displayTokenAt(GameData->array, temp_col--, row);
			//	putCursorAt(GameData->FirstTokenLoc.col+(row*2), GameData->FirstTokenLoc.row+(temp_col*4));

   			 //	if (array[row][col] != array[row][--temp_col]) {
			//		return 0;
   			 //	}
			  }

			break;

	case LEFTDIAG:
			break;

	case VERTICAL:
			break;

	case RIGHTDIAG:
			break;
}
	disableBlinkingText();
}




int connectFourPresent(GameDataElements* GameData) {
  int row, col;
  for (row = 6; row >= 0; row--) {
    for (col = 6; col >= 0; col--) {
      if (connectFourHorizontal(GameData->array, row, col)) {
	showConnectFour(GameData, row, col, HORIZONTAL);
        return 1;
      }
      if (connectFourLeftDiagonal(GameData->array, row, col)) {
        return 1;
      }
      if (connectFourVertical(GameData->array, row, col)) {
        return 1;
      }
      if (connectFourRightDiagonal(GameData->array, row, col)) {
        return 1;
      }
    }
  }
  return 0;
}

int connectFourHorizontal(int array[7][7], int row, int col) {
  if (col < 3) {
    return 0;
  }

  if (array[row][col] == EMPTY) {
    return 0;
  }

  int i;
  int temp_col = col;
  for (i = 0; i < 3; i++) {
    if (array[row][col] != array[row][--temp_col]) {
	return 0;
    }
  }

 return 1;
}

int connectFourLeftDiagonal(int array[7][7], int row, int col) {
  if (col < 3 || row < 3) {
    return 0;
  }

  if (array[row][col] == EMPTY) {
    return 0;
  }

  int i;
  int temp_col = col;
  int temp_row = row;
  for (i = 0; i < 3; i++) {
    if (array[row][col] != array[--temp_row][--temp_col]) {
	return 0;
    }
  }
	return 1;
}

int connectFourVertical(int array[7][7], int row, int col) {
  if (row < 3) {
    return 0;
  }
  if (array[row][col] == EMPTY) {
    return 0;
  }

  int i;
  int temp_row = row;
  for (i = 0; i < 3; i++) {
    if (array[row][col] != array[--temp_row][col]) {


	return 0;
    }
  }
	return 1;
}

int connectFourRightDiagonal(int array[7][7], int row, int col) {
  if (col > 3 || row < 3) {
    return 0;
  }
  if (array[row][col] == EMPTY) {
    return 0;
  }

  int i;
  int temp_col = col;
  int temp_row = row;
  for (i = 0; i < 3; i++) {
    if (array[row][col] != array[--temp_row][++temp_col]) {
	return 0;
    }
  }
	return 1;
}


/*** Game wrapup ***/

void endGame() {}

void quitGame() {}

void resetGame() {}

/*** Status Bars ***/

void displayDirectionsStatusBar(CursorLoc DirectionsStatusBarLoc) {
	
  putCursorAt(DirectionsStatusBarLoc.row, DirectionsStatusBarLoc.col);
  displayBlueColorText();
  display(DIRECTIONS_ARROWS);
  putCursorAt(DirectionsStatusBarLoc.row+1, DirectionsStatusBarLoc.col);
  display(DIRECTIONS_ENTER);
  displayDefaultColorText();
} 


void displayPlayerTurnStatusBar(GameDataElements* GameData) {

  putCursorAt(GameData->PlayerTurnStatusBarLoc.row, GameData->PlayerTurnStatusBarLoc.col);

  if (GameData->move_counter % 2 == 0) {
      displayRedColorText();
      display(P1TURN);
      displayDefaultColorText();
    } else {
      displayYellowColorText();
      display(P2TURN);
      displayDefaultColorText();
    }


} 

void displayWinnerStatusBar(GameDataElements* GameData) {

  putCursorAt(GameData->WinnerStatusBarLoc.row, GameData->WinnerStatusBarLoc.col);
  enableBlinkingText();
  if (GameData->move_counter % 2 == 0) {
      displayYellowColorText();
      display(P2WIN);
    } else {
      displayRedColorText();
      display(P1WIN);
    }
   displayDefaultColorText();
   disableBlinkingText();
}

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

void putCursorAt(int row, int col) {
  char esc[20] = ESC;

  char buffer[10];
  sprintf(buffer, "%d", row);
  strcat(esc, buffer);
  strcat(esc, ";");

  sprintf(buffer, "%d", col);
  strcat(esc, buffer);

  strcat(esc, "H");

  write(STDOUT_FILENO, esc, sizeof(esc));
}

void display(char* item) { write(STDOUT_FILENO, item, strlen(item)); }

void clearScreen() { write(STDOUT_FILENO, CLEAR, 4); }

void hideCursor() { write(STDOUT_FILENO, HIDE, 6); }

void unhideCursor() { write(STDOUT_FILENO, UNHIDE, 6); }

void displayRedColorText() { write(STDOUT_FILENO, RED_COLOR, 5); }

void displayYellowColorText() { write(STDOUT_FILENO, YELLOW_COLOR, 5); }

void displayBlueColorText() {  write(STDOUT_FILENO, BLUE_COLOR, 5); }

void displayDefaultColorText() { write(STDOUT_FILENO, DEFAULT_COLOR, 5); }

void enableBlinkingText() { write(STDOUT_FILENO, BLINKING_ON, strlen(BLINKING_ON)); }

void disableBlinkingText() { write(STDOUT_FILENO, BLINKING_OFF, strlen(BLINKING_OFF)); }



/*** main ***/

int main() {
  char error_message[50] = NO_ERRORS;

  // Cannot use exitProgram function for failure of initalizeTermSettings
  // because the state of TermSettings will be unknown. However, no settings
  // have been applied and the terminal is unchanged, exit(1) is sufficent.
  TermSettingsData TermSettings = initializeTermSettings(error_message);
  if (TermSettings.successful_initialization == -1) {
    perror(error_message);
    exit(1);
  }

  if (enableRawInputMode(TermSettings.orig_termios, error_message) == -1) {
    exitProgram(&TermSettings, error_message);
  }

  GameDataElements GameData = initilizeGameData(&TermSettings);
  displayGameBoard(&GameData);

  int game_not_quit = 1;
  while (game_not_quit) {
    // The function displayTokens is prior to the function connectFourPresent so
    // that upon a winning move the last token placement is displayed.
    displayTokens(&GameData);

    if (connectFourPresent(&GameData)) {
      displayWinnerStatusBar(&GameData);//TODO add endgame here
    } else {
      displayPlayerTurnStatusBar(&GameData);
    }

    game_not_quit = playerInput(&GameData, error_message);
  }

  exitProgram(&TermSettings, error_message);

  return 0;
}
