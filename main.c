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

/*** Define ***/

#define BLANK_LINE "                                           "
#define BLINKING_OFF "\x1b[m"
#define BLINKING_ON "\x1b[1;5;7m"
#define BLUE_COLOR "\x1b[34m"
#define BOARDTOP "+---+---+---+---+---+---+---+"
#define CLEAR "\x1b[2J"
#define CORNER "H"
#define CTRL_KEY(k) ((k)&0x1f)
#define DEFAULT_COLOR "\x1b[39m"
#define DIRECTION_ARROW "PRESS ARROW KEY TO MOVE THE TOKEN"
#define DIRECTIONS_ENTER "PRESS ENTER KEY TO DROP THE TOKEN"
#define DOWN "B"
#define ENDGAME_DIRECTIONS "GAME OVER, DO YOU WANT TO PLAY AGAIN? (Y/N)"
#define ESC "\x1b["
#define HIDE "\x1b[?25l"
#define LEFT "D"
#define NO_ERRORS ""
#define PLAYER1 "X"
#define PLAYER2 "O"
#define P1TURN "PLAYER 1's TURN"
#define P1WIN "PLAYER 1 IS THE WINNER"
#define P2TURN "PLAYER 2's TURN"
#define P2WIN "PLAYER 2 IS THE WINNER"
#define RED_COLOR "\x1b[31m"
#define RIGHT "C"
#define TITLE "CONNECT FOUR"
#define UNHIDE "\x1b[?25h"
#define UP "A"
#define YELLOW_COLOR "\x1b[33m"

/*** Enum ***/

enum arrow_enter { ENTER = 13, RIGHT_ARROW = 67, LEFT_ARROW = 68 };
enum bounds { LEFT_BOUNDARY = 0, RIGHT_BOUNDARY = 6 };
enum token { EMPTY = -1, RED, YELLOW };
enum vectors { HORIZONTAL, LEFTDIAG, VERTICAL, RIGHTDIAG };

/*** Structures ***/

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
  CursorLoc TurnStatusBarLoc;
  CursorLoc DirectionsStatusBarLoc;
  CursorLoc WinStatusBarLoc;
  CursorLoc EndGameStatusBarLoc;
  CursorLoc BlankLineColLoc;
} GameDataElements;

typedef struct TermHandle {
  int successful_initialization;
  int screen_rows;
  int screen_cols;
  struct termios orig_termios;
} TermSettingsData;

/*** Declorations ***/

// applyNewTerSettings returns the new terminal settings that initTermSettings()
// function establishes. error_message is used incase of failures.
int applyNewTermSettings(struct termios NewSettings, char* error_message);

// centerText returns the offset of half the text length, which is used to
// center the text in the terminal.
int centerText(char* text);

// clearScreen clears the screen.
void clearScreen();

// clearTerm clears the terminal by clearing the screen, hiding the cursor, and
// placing the cursor to the top right corner.
void clearTerm();

// connectFourPresent searches for the presence of a four tokens in a line
// hoizontally, left diagonally, vertically, and right diagonally. Returns 1 if
// found, 0 otherwise.
int connectFourPresent(GameDataElements* GameData);

// connectFourHorizontal returns 1 if a connect four is found in the horizontal
// vector in the array at the row and column index, 0 otherwise.
int connectFourHorizontal(int array[7][7], int row, int col);

// connectFourLeftDiagonal returns 1 if a connect four is found in the left
// diagonal vector in the array at the row and column index, 0 otherwise.
int connectFourLeftDiagonal(int array[7][7], int row, int col);

// connectFourRightDiagonal returns 1 if a connect four is found in the right
// diagonal vector in the array at the row and column index, 0 otherwise.
int connectFourRightDiagonal(int array[7][7], int row, int col);

// connectFourVertical returns 1 if a connect four is found in the vertical
// vector in the array at the row and column index, 0 otherwise.
int connectFourVertical(int array[7][7], int row, int col);

// disableBlinkinText applies the default esc sequence to return the text to
// default.
void disableBlinkingText();

// disableRawInputMode returns the terminal to the previous settings.
// error_message is used in case of failures.
int disableRawInputMode(TermSettingsData* TermSettings, char* error_message);

// Displays strings to terminal by executing write().
void display(char* item);

// displayBlueColorText changes the text color to blue.
void displayBlueColorText();

// displayCurPlayersToken displays the current token that the player interacts
// with prior to the token getting dropped.
void displayCurPlayersToken(char* cur_players_token);

// displayDefaultColorText returns colored text to the default color.
void displayDefaultColorText();

// displayDirectionsStatusBar displays the status bar that contains the
// directions for playing the game.
void displayDirectionsStatusBar(GameDataElements* GameData);

// displayEndGameStatusBar displays the end game status bar upon a connect four
// being found.
void displayEndGameStatusBar(GameDataElements* GameData);

// displayGameBoard displays the title and game board.
void displayGameBoard(GameDataElements* GameData);

// displayBlueColorText changes the text color to red.
void displayRedColorText();

// displayTitle displays the "CONNECT FOUR" title.
void displayTitle(CursorLoc ConnectFourTitleLoc);

// displayTokenAt displays the token in the game board in the array at the
// column and row.
void displayTokenAt(int array[7][7], int col, int row);

// displayTokens displays all the tokens that are present in the game data
// array.
void displayTokens(GameDataElements* GameData);

// displayTurnStatusBar displays which players turn it is by number and color.
void displayTurnStatusBar(GameDataElements* GameData);

// displayWinStatusBar displays which player won when a connect four is
// discovered.
void displayWinStatusBar(GameDataElements* GameData);

// displayBlueColorText changes the text color to yellow.
void displayYellowColorText();

// drawGameBoard displays the outline of the game board.
void drawGameBoard(CursorLoc GameBoardLoc);

// dropToken place the token in the game data array in the current column
// position and stacks the token on top of the highest unused row index.
int dropToken(GameDataElements* GameData, int cur_col_position);

// enableBlinkingText bolds, inverts, and blinks the text. Used for the player
// status bar and highlights the connect four tokens.
void enableBlinkingText();

// enableRawInputMode is user to prepare the terminal for the game.
int enableRawInputMode(struct termios OriginalTerm, char* error_message);

// enableTimeOutForRead enables a timeout for the read function().
void enableTimeOutForRead(struct termios* NewSettings);

// endGame is used after the game is won to allow the player to determine if
// they want to replay the game or quit.
int endGame(GameDataElements* GameData, char* error_message);

// exitProgram exits the game for both error and non error game states.
void exitProgram(TermSettingsData* TermSettings, char* error_message);

// findBlankLineLoc returns the location to place the BLANK_LINE string, based
// of the center of the terminal. Only finds the col, row is not used.
CursorLoc findBlankLineLoc(TermSettingsData* TermSettings);

// findConnectFourLoc returns the location to place the TITLE string, based of
// the center of the terminal.
CursorLoc findConnectFourTitleLoc(TermSettingsData* TermSettings);

char* findCurPlayersToken(int move_counter);

// findDirectionStatusBarLoc finds the location to place the DIRECTION_ARROW and
// DIRECTION_ENTER strings, based of the center of the terminal.
CursorLoc findDirectionsStatusBarLoc(TermSettingsData* TermSettings);

// findEndGameStatusBarLoc returns the location to place the P1WIN and P2WIN
// strings, based of the center of the terminal.
CursorLoc findEndGameStatusBarLoc(TermSettingsData* TermSettings);

// findFirstTokenLoc returns the location to place the token at the [0][0] index
// of the game data array, based of the center of the terminal.
CursorLoc findFirstTokenLoc(TermSettingsData* TermSettings);

// findGameBoardLoc returns the location to place the game board, based of the
// center of the terminal.
CursorLoc findGameBoardLoc(TermSettingsData* TermSettings);

// findPlayersInitialLoc returns the location to place the token being moved and
// dropped, based of the center of the terminal.
CursorLoc findPlayersInitialLoc(TermSettingsData* TermSettings);

// findTurnStatusBarLoc returns the location to place the P1TURN and P2TURN
// strings, based of the center of the terminal.
CursorLoc findTurnStatusBarLoc(TermSettingsData* TermSettings);

// findWinStatusBarLoc returns the location to place a the ENDGAME_DIRECTIONS
// string, based of the center of the terminal.
CursorLoc findWinStatusBarLoc(TermSettingsData* TermSettings);

// gamePlayLoop contains the while loop that takes user input to move the
// players token and drop the token. error_message is used in case of failures.
int gamePlayLoop(GameDataElements* GameData, char* error_message);

// getWindowSize gets the terminal size, which is used to center display.
int getWindowSize(int* out_rows, int* out_cols);

// hideCursor hides the cursor.
void hideCursor();

// initGameData initializes the elements of the GameData struct.
GameDataElements initGameData(TermSettingsData* TermSettings);

// initSettingsData initializes the elements of the termSettingData struct.
TermSettingsData initTermSettings(char* error_message);

// moveCursor moves the cursor by an amount in the direction by executing
// write().
void moveCursor(int amount, char* direction);

// moveTokenLeft moves the current token in play left.
void moveTokenLeft(char* cur_players_token, int* cur_position);

// moveTokenRight moves the current token in play right.
void moveTokenRight(char* cur_players_token, int* cur_position);

// placeTokenAtLeftBoundary moves the current token to the left boundary if the
// token is at the right boundary and the player uses the right arrow key.
void placeTokenAtLeftBoundary(char* cur_players_token, int* cur_position);

// placeTokenAtRightBoundary moves the current token to the right boundary if
// the token is at the left boundary and the player uses the left arrow key.
void placeTokenAtRightBoundary(char* cur_players_token, int* cur_position);

// playerInputReader returns the char that the player inputs from the keyboard.
int playerInputReader(char* player_input, char* error_message);

// putCursorAt puts the cursor at the row and col on the terminal.
void putCursorAt(int row, int col);

// resetGame resets the gameDataElements to restart the game.
void resetGame(TermSettingsData* TermSettings, GameDataElements* GameData);

// showConnectFour highlights the connect four tokens found by
// connectFourPresent functions.
void showConnectFour(GameDataElements* GameData, int row, int col, int vector);

// turnOff_cflags turns off CS8 flag. Used by enableRawInputMode.
void turnOff_cflags(tcflag_t* c_cflag);

// turnOff_iflags turns off BRKINT, ICRNL, INPCK, ISTRIP, IXON flags. Used by
// enableRawInputMode.
void turnOff_iflags(tcflag_t* c_iflag);

// turnOff_lflags turns off ECHO, ICANNON, IEXTEN, ISIG flags. Used by
// enableRawInputMode.
void turnOff_lflags(tcflag_t* c_lflag);

// turnOff_oflags turns off OPOST flag. Used by enableRawInputMode.
void turnOff_oflags(tcflag_t* c_oflag);

// unhideCursor unhides the cursor.
void unhideCursor();

/*** Functions ***/

int applyNewTermSettings(struct termios NewSettings, char* error_message) {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &NewSettings) == -1) {
    strcat(error_message, "enableRawMode->applyNewTermSettings->tcsetattr");
    return -1;
  }
  return 0;
}

int centerText(char* text) { return strlen(text) / (2); }

void clearScreen() { write(STDOUT_FILENO, CLEAR, 4); }

void clearTerm() {
  hideCursor();
  clearScreen();
  moveCursor(0, CORNER);
}

int connectFourPresent(GameDataElements* GameData) {
  int row, col;
  // Search pattern starts at the bottom right hand corner and moves left. This
  // way only four of the eight possible vectors have to be checked.
  for (row = 6; row >= 0; row--) {
    for (col = 6; col >= 0; col--) {
      if (connectFourHorizontal(GameData->array, row, col)) {
        showConnectFour(GameData, row, col, HORIZONTAL);
        return 1;
      }
      if (connectFourLeftDiagonal(GameData->array, row, col)) {
        showConnectFour(GameData, row, col, LEFTDIAG);
        return 1;
      }
      if (connectFourVertical(GameData->array, row, col)) {
        showConnectFour(GameData, row, col, VERTICAL);
        return 1;
      }
      if (connectFourRightDiagonal(GameData->array, row, col)) {
        showConnectFour(GameData, row, col, RIGHTDIAG);
        return 1;
      }
    }
  }
  return 0;
}

int connectFourHorizontal(int array[7][7], int row, int col) {
  // Any index less than 4 cannot have a four or more tokens.
  if (col < 3) {
    return 0;
  }

  if (array[row][col] == EMPTY) {
    return 0;
  }
  // Returns 0 (not found) if the next three tokens do not match the initial
  // row/col token.
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
  // Any index less than 4 cannot have a four or more tokens.
  if (col < 3 || row < 3) {
    return 0;
  }

  if (array[row][col] == EMPTY) {
    return 0;
  }
  // Returns 0 (not found) if the next three tokens do not match the initial
  // row/col token.
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

int connectFourRightDiagonal(int array[7][7], int row, int col) {
  // Any index less than 4 cannot have a four or more tokens.
  if (col > 3 || row < 3) {
    return 0;
  }
  if (array[row][col] == EMPTY) {
    return 0;
  }
  // Returns 0 (not found) if the next three tokens do not match the initial
  // row/col token.
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

int connectFourVertical(int array[7][7], int row, int col) {
  // Any index less than 4 cannot have a four or more tokens.
  if (row < 3) {
    return 0;
  }
  if (array[row][col] == EMPTY) {
    return 0;
  }
  // Returns 0 (not found) if the next three tokens do not match the initial
  // row/col token.
  int i;
  int temp_row = row;
  for (i = 0; i < 3; i++) {
    if (array[row][col] != array[--temp_row][col]) {

      return 0;
    }
  }
  return 1;
}

void disableBlinkingText() {
  write(STDOUT_FILENO, BLINKING_OFF, strlen(BLINKING_OFF));
}

int disableRawInputMode(TermSettingsData* TermSettings, char* error_message) {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &TermSettings->orig_termios) == -1) {
    strcat(error_message, "die->disableRawInputMode->tcsetattr");
    return -1;
  }
  return 0;
}

void display(char* item) { write(STDOUT_FILENO, item, strlen(item)); }

void displayBlueColorText() { write(STDOUT_FILENO, BLUE_COLOR, 5); }

void displayCurPlayersToken(char* cur_players_token) {
  if (strcmp(cur_players_token, PLAYER1) == 0) {
    displayRedColorText();
  } else {
    displayYellowColorText();
  }
  display(cur_players_token);
  moveCursor(1, LEFT);

  displayDefaultColorText();
}

void displayDefaultColorText() { write(STDOUT_FILENO, DEFAULT_COLOR, 5); }

void displayDirectionsStatusBar(GameDataElements* GameData) {
  putCursorAt(GameData->DirectionsStatusBarLoc.row,
              GameData->BlankLineColLoc.col);
  display(BLANK_LINE);
  putCursorAt(GameData->DirectionsStatusBarLoc.row + 1,
              GameData->BlankLineColLoc.col);
  display(BLANK_LINE);

  putCursorAt(GameData->DirectionsStatusBarLoc.row,
              GameData->DirectionsStatusBarLoc.col);
  displayBlueColorText();
  display(DIRECTION_ARROW);
  putCursorAt(GameData->DirectionsStatusBarLoc.row + 1,
              GameData->DirectionsStatusBarLoc.col);
  display(DIRECTIONS_ENTER);
  displayDefaultColorText();
}

void displayEndGameStatusBar(GameDataElements* GameData) {
  putCursorAt(GameData->EndGameStatusBarLoc.row, GameData->BlankLineColLoc.col);
  display(BLANK_LINE);
  putCursorAt(GameData->EndGameStatusBarLoc.row + 1,
              GameData->BlankLineColLoc.col);
  display(BLANK_LINE);

  putCursorAt(GameData->EndGameStatusBarLoc.row,
              GameData->EndGameStatusBarLoc.col);
  displayBlueColorText();
  display(ENDGAME_DIRECTIONS);
  displayDefaultColorText();
}

void displayGameBoard(GameDataElements* GameData) {
  clearTerm();
  displayTitle(GameData->ConnectFourTitleLoc);
  displayDirectionsStatusBar(GameData);
  drawGameBoard(GameData->GameBoardLoc);
}

void displayRedColorText() { write(STDOUT_FILENO, RED_COLOR, 5); }

void displayTitle(CursorLoc ConnectFourTitleLoc) {
  putCursorAt(ConnectFourTitleLoc.row, ConnectFourTitleLoc.col);
  display(TITLE);
}

void displayTokenAt(int array[7][7], int col, int row) {
  if (array[row][col] == 0) {
    displayRedColorText();
    write(STDOUT_FILENO, PLAYER1, strlen(PLAYER1));
    displayDefaultColorText();
  } else if (array[row][col] == 1) {
    displayYellowColorText();
    write(STDOUT_FILENO, PLAYER2, strlen(PLAYER2));
    displayDefaultColorText();
  } else {
    write(STDOUT_FILENO, " ", 1);
  }
}

void displayTokens(GameDataElements* GameData) {
  int cursor_col = GameData->FirstTokenLoc.col;
  int cursor_row = GameData->FirstTokenLoc.row;
  int token_col, token_row;
  for (token_col = 0; token_col < 7; token_col++) {
    for (token_row = 0; token_row < 7; token_row++) {
      putCursorAt(cursor_row, cursor_col);
      displayTokenAt(GameData->array, token_col, token_row);
      // Plus 2 is used because there are two spaces between the rows on the
      // ASCII representation of the board.
      cursor_row += 2;
    }
    cursor_row = GameData->FirstTokenLoc.row;
    // Plus t4 is used because there are four spaces between the columns on the
    // ASCII representation of the board.
    cursor_col += 4;
  }
}

void displayTurnStatusBar(GameDataElements* GameData) {
  putCursorAt(GameData->TurnStatusBarLoc.row, GameData->BlankLineColLoc.col);
  display(BLANK_LINE);

  putCursorAt(GameData->TurnStatusBarLoc.row, GameData->TurnStatusBarLoc.col);
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

void displayWinStatusBar(GameDataElements* GameData) {
  putCursorAt(GameData->WinStatusBarLoc.row, GameData->BlankLineColLoc.col);
  display(BLANK_LINE);

  putCursorAt(GameData->WinStatusBarLoc.row, GameData->WinStatusBarLoc.col);
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

void displayYellowColorText() { write(STDOUT_FILENO, YELLOW_COLOR, 5); }

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

int dropToken(GameDataElements* GameData, int cur_col_position) {
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

void enableBlinkingText() {
  write(STDOUT_FILENO, BLINKING_ON, strlen(BLINKING_ON));
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

void enableTimeOutForRead(struct termios* NewSettings) {
  NewSettings->c_cc[VMIN] = 0;
  NewSettings->c_cc[VTIME] = 1;
}

int endGame(GameDataElements* GameData, char* error_message) {
  displayEndGameStatusBar(GameData);

  char player_input;
  while (1) {
    if (playerInputReader(&player_input, error_message) == -1) {
      return 0;
    }

    switch (player_input) {
    case 'y':
    case 'Y':
      return 1;
      break;
    case CTRL_KEY('q'):
    case 'n':
    case 'N':
      return 0;
      break;
    }
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

CursorLoc findBlankLineLoc(TermSettingsData* TermSettings) {
  CursorLoc BlankLineCol;
  BlankLineCol.col = (TermSettings->screen_cols / 2) - centerText(BLANK_LINE);
  BlankLineCol.row = 0;
  return BlankLineCol;
}

CursorLoc findConnectFourTitleLoc(TermSettingsData* TermSettings) {
  CursorLoc Title;

  Title.col = (TermSettings->screen_cols / 2) - centerText(TITLE);
  Title.row = (TermSettings->screen_rows / 2) - 14;

  return Title;
}

char* findCurPlayersToken(int move_counter) {
  if (move_counter % 2 == 0) {
    return PLAYER1;
  } else {
    return PLAYER2;
  }
}

CursorLoc findDirectionsStatusBarLoc(TermSettingsData* TermSettings) {
  CursorLoc Direct;

  Direct.col = (TermSettings->screen_cols / 2) - centerText(DIRECTION_ARROW);
  Direct.row = (TermSettings->screen_rows / 2) + 12;

  return Direct;
}

CursorLoc findEndGameStatusBarLoc(TermSettingsData* TermSettings) {
  CursorLoc End;

  End.col = (TermSettings->screen_cols / 2) - centerText(ENDGAME_DIRECTIONS);
  End.row = (TermSettings->screen_rows / 2) + 12;

  return End;
}

CursorLoc findFirstTokenLoc(TermSettingsData* TermSettings) {
  CursorLoc FirstToken;

  FirstToken.col = (TermSettings->screen_cols / 2) - (centerText(BOARDTOP) - 2);
  FirstToken.row = (TermSettings->screen_rows / 2) - 4;

  return FirstToken;
}

CursorLoc findGameBoardLoc(TermSettingsData* TermSettings) {
  CursorLoc GameBoard;

  GameBoard.col = (TermSettings->screen_cols / 2) - centerText(BOARDTOP);
  GameBoard.row = (TermSettings->screen_rows / 2) - 6;

  return GameBoard;
}

CursorLoc findPlayersInitialLoc(TermSettingsData* TermSettings) {
  CursorLoc Players;

  Players.col = (TermSettings->screen_cols / 2) - (centerText(BOARDTOP) - 2);
  Players.row = (TermSettings->screen_rows / 2) - 6;

  return Players;
}

CursorLoc findTurnStatusBarLoc(TermSettingsData* TermSettings) {
  CursorLoc Turn;

  Turn.col = (TermSettings->screen_cols / 2) - centerText(P1TURN);
  Turn.row = (TermSettings->screen_rows / 2) - 8;

  return Turn;
}

CursorLoc findWinStatusBarLoc(TermSettingsData* TermSettings) {
  CursorLoc WinStatusBar;

  WinStatusBar.col = (TermSettings->screen_cols / 2) - centerText(P1WIN);
  WinStatusBar.row = (TermSettings->screen_rows / 2) - 8;

  return WinStatusBar;
}

int gamePlayLoop(GameDataElements* GameData, char* error_message) {
  char* cur_players_token = findCurPlayersToken(GameData->move_counter);
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
    // Used for quitting the game manually.
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
      if (dropToken(GameData, cur_position)) {
        cur_player_turn--;
        GameData->move_counter++;
      }
      break;
    }
  }
  return 1;
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

void hideCursor() { write(STDOUT_FILENO, HIDE, 6); }

GameDataElements initGameData(TermSettingsData* TermSettings) {
  GameDataElements NewGame;

  NewGame.move_counter = 0;

  // Populates array with 49 EMPTY tokes.
  int i, j;
  for (i = 0; i < 7; i++) {
    for (j = 0; j < 7; j++) {
      NewGame.array[i][j] = EMPTY;
    }
  }

  NewGame.ConnectFourTitleLoc = findConnectFourTitleLoc(TermSettings);
  NewGame.GameBoardLoc = findGameBoardLoc(TermSettings);
  NewGame.FirstTokenLoc = findFirstTokenLoc(TermSettings);
  NewGame.PlayersInitialLoc = findPlayersInitialLoc(TermSettings);
  NewGame.DirectionsStatusBarLoc = findDirectionsStatusBarLoc(TermSettings);
  NewGame.TurnStatusBarLoc = findTurnStatusBarLoc(TermSettings);
  NewGame.WinStatusBarLoc = findWinStatusBarLoc(TermSettings);
  NewGame.BlankLineColLoc = findBlankLineLoc(TermSettings);
  NewGame.EndGameStatusBarLoc = findEndGameStatusBarLoc(TermSettings);

  return NewGame;
}

TermSettingsData initTermSettings(char* error_message) {
  TermSettingsData OldSettings;
  OldSettings.successful_initialization = 0;

  // Captures cur terminal settings, used to enable raw mode.
  if (tcgetattr(STDIN_FILENO, &OldSettings.orig_termios) == -1) {
    strcat(error_message, "initTermSettings->tcgetattr");
    OldSettings.successful_initialization = -1;
  }

  // find terminal window size to center the game display.
  if (getWindowSize(&OldSettings.screen_rows, &OldSettings.screen_cols) == -1) {
    strcat(error_message, "initialize_TermSettings->getWindowSize");
    OldSettings.successful_initialization = -1;
  }

  return OldSettings;
}

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

void moveTokenLeft(char* cur_players_token, int* cur_position) {
  display(" ");
  moveCursor(5, LEFT);
  displayCurPlayersToken(cur_players_token);
  *cur_position = *cur_position - 1;
}

void moveTokenRight(char* cur_players_token, int* cur_position) {
  display(" ");
  moveCursor(3, RIGHT);
  displayCurPlayersToken(cur_players_token);
  *cur_position = *cur_position + 1;
}

void placeTokenAtLeftBoundary(char* cur_players_token, int* cur_position) {
  display(" ");
  moveCursor(25, LEFT);
  displayCurPlayersToken(cur_players_token);
  *cur_position = LEFT_BOUNDARY;
}

void placeTokenAtRightBoundary(char* cur_players_token, int* cur_position) {
  display(" ");
  moveCursor(23, RIGHT);
  displayCurPlayersToken(cur_players_token);
  *cur_position = RIGHT_BOUNDARY;
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

void showConnectFour(GameDataElements* GameData, int row, int col, int vector) {
  enableBlinkingText();
  int i;
  int temp_col = col;
  int temp_row = row;
  switch (vector) {
  // Using the first token as a base, the rows and col have thier indexs
  // multipled by the distance between tokens(2 and 4 respectively) to overwrite
  // the token in that position with a blinking token.
  case HORIZONTAL:
    for (i = 0; i < 4; i++) {
      putCursorAt(GameData->FirstTokenLoc.row + (temp_row * 2),
                  GameData->FirstTokenLoc.col + (temp_col * 4));
      displayTokenAt(GameData->array, temp_col--, temp_row);
    }

    break;

  case LEFTDIAG:
    for (i = 0; i < 4; i++) {
      putCursorAt(GameData->FirstTokenLoc.row + (temp_row * 2),
                  GameData->FirstTokenLoc.col + (temp_col * 4));
      displayTokenAt(GameData->array, temp_col--, temp_row--);
    }

    break;

  case VERTICAL:
    for (i = 0; i < 4; i++) {
      putCursorAt(GameData->FirstTokenLoc.row + (temp_row * 2),
                  GameData->FirstTokenLoc.col + (temp_col * 4));
      displayTokenAt(GameData->array, temp_col, temp_row--);
    }

    break;

  case RIGHTDIAG:
    for (i = 0; i < 4; i++) {
      putCursorAt(GameData->FirstTokenLoc.row + (temp_row * 2),
                  GameData->FirstTokenLoc.col + (temp_col * 4));
      displayTokenAt(GameData->array, temp_col++, temp_row--);
    }

    break;
  }
  disableBlinkingText();
}

void resetGame(TermSettingsData* TermSettings, GameDataElements* GameData) {

  *GameData = initGameData(TermSettings);

  displayDirectionsStatusBar(GameData);
  displayTurnStatusBar(GameData);
  displayTokens(GameData);
}

void turnOff_cflags(tcflag_t* c_cflag) {
  // CS8: misc flag
  *c_cflag |= (CS8);
}

void turnOff_iflags(tcflag_t* c_iflag) {
  // BRKINT: misc flag, ICRNL: ctrl-m, INPCK: misc flag,
  // ISTRIP: misc flag, IXON: ctrl_s and ctrl_q
  *c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
}

void turnOff_lflags(tcflag_t* c_lflag) {
  // ECHO:Print text to screen, ICANON:Canonical Mode, IEXTEN & ISIG:ctrl=c
  // and ctrl-v
  *c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
}

void turnOff_oflags(tcflag_t* c_oflag) {
  // OPOST: output processing /r/n
  *c_oflag &= ~(OPOST);
}

void unhideCursor() { write(STDOUT_FILENO, UNHIDE, 6); }

/*** Main ***/

int main() {
  char error_message[50] = NO_ERRORS;

  // Cannot use exitProgram function for failure of initalizeTermSettings
  // because the state of TermSettings will be unknown. However, no settings
  // have been applied and the terminal is unchanged, exit(1) is sufficent.
  TermSettingsData TermSettings = initTermSettings(error_message);
  if (TermSettings.successful_initialization == -1) {
    perror(error_message);
    exit(1);
  }
  // Sets enables raw input mode, extis program if an error is incurred.
  if (enableRawInputMode(TermSettings.orig_termios, error_message) == -1) {
    exitProgram(&TermSettings, error_message);
  }

  // initialized game data and draws the board / title.
  GameDataElements GameData = initGameData(&TermSettings);
  displayGameBoard(&GameData);

  int game_not_quit = 1;
  while (game_not_quit) {
    // The function displayTokens is prior to the function connectFourPresent so
    // that upon a winning move the last token placement is displayed.
    displayTokens(&GameData);

    // If connect four is present, show who won and gives the option to restart
    // game or quit.
    if (connectFourPresent(&GameData)) {
      displayWinStatusBar(&GameData);
      if (endGame(&GameData, error_message) == 0) {
        break;
      } else {
        resetGame(&TermSettings, &GameData);
      }
    } else {
      displayTurnStatusBar(&GameData);
    }

    // Contains the main gameplay loop and returns if the player decided to quit
    // manually.
    game_not_quit = gamePlayLoop(&GameData, error_message);
  }

  // Exits the program for both error and non error modes.
  exitProgram(&TermSettings, error_message);

  return 0;
}
