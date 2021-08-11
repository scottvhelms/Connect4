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
typedef enum boolean { FALSE, TRUE } boolean;
enum bounds { LEFT_BOUNDARY = 0, RIGHT_BOUNDARY = 6 };
enum token { EMPTY = -1, RED, YELLOW };
enum vectors { HORIZONTAL, LEFTDIAG, VERTICAL, RIGHTDIAG };

/*** Structures ***/

typedef struct CursorLocation {
  int row;
  int col;
} CursorLocation;

typedef struct GameData {
  int array[7][7];
  int move_counter;
  CursorLocation connect_four_title_location;
  CursorLocation game_board_location;
  CursorLocation first_token_location;
  CursorLocation players_initial_location;
  CursorLocation turn_status_bar_location;
  CursorLocation directions_status_bar_location;
  CursorLocation winner_status_bar_location;
  CursorLocation end_game_status_bar_location;
  CursorLocation blank_line_column_location;
} GameData;

typedef struct TerminalSettings {
  int successful_initialization;
  int screen_rows;
  int screen_cols;
  struct termios orig_termios;
} TerminalSettings;

/*** Declorations ***/

// applyNewTerSettings returns the new terminal settings that
// initializeTerminalSettings() function establishes. error_message is used
// incase of failures.
int applyNewterminal_settings(struct termios new_settings, char* error_message);

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
boolean connectFourPresent(GameData* game_data);

// connectFourHorizontal returns 1 if a connect four is found in the horizontal
// vector in the array at the row and column index, 0 otherwise.
boolean connectFourHorizontal(int array[7][7], int row, int col);

// connectFourLeftDiagonal returns 1 if a connect four is found in the left
// diagonal vector in the array at the row and column index, 0 otherwise.
boolean connectFourLeftDiagonal(int array[7][7], int row, int col);

// connectFourRightDiagonal returns 1 if a connect four is found in the right
// diagonal vector in the array at the row and column index, 0 otherwise.
boolean connectFourRightDiagonal(int array[7][7], int row, int col);

// connectFourVertical returns 1 if a connect four is found in the vertical
// vector in the array at the row and column index, 0 otherwise.
boolean connectFourVertical(int array[7][7], int row, int col);

// createGameData initializes the elements of the game_data struct.
GameData createGameData(TerminalSettings* terminal_settings);

// disableBlinkinText applies the default esc sequence to return the text to
// default.
void disableBlinkingText();

// disableRawInputMode returns the terminal to the previous settings.
// error_message is used in case of failures.
int disableRawInputMode(TerminalSettings* terminal_settings,
                        char* error_message);

// displayBlueColorText changes the text color to blue.
void displayBlueColorText();

// displayCurrentPlayersToken displays the current token that the player
// interacts with prior to the token getting dropped.
void displayCurrentPlayersToken(char* current_players_token);

// displayDefaultColorText returns colored text to the default color.
void displayDefaultColorText();

// displayDirectionsStatusBar displays the status bar that contains the
// directions for playing the game.
void displayDirectionsStatusBar(GameData* game_data);

// displayEndGameStatusBar displays the end game status bar upon a connect four
// being found.
void displayEndGameStatusBar(GameData* game_data);

// displayGameBoard displays the title and game board.
void displayGameBoard(GameData* game_data);

// displayBlueColorText changes the text color to red.
void displayRedColorText();

// Displays strings to terminal by executing write().
void displayStrings(char* item);

// displayTitle displays the "CONNECT FOUR" title.
void displayTitle(CursorLocation connect_four_title_location);

// displayTokenAt displays the token in the game board in the array at the
// column and row.
void displayTokenAt(int array[7][7], int col, int row);

// displayTokens displays all the tokens that are present in the game data
// array.
void displayTokens(GameData* game_data);

// displayTurnStatusBar displays which players turn it is by number and color.
void displayTurnStatusBar(GameData* game_data);

// displayWinStatusBar displays which player won when a connect four is
// discovered.
void displayWinStatusBar(GameData* game_data);

// displayBlueColorText changes the text color to yellow.
void displayYellowColorText();

// drawGameBoard displays the outline of the game board.
void drawGameBoard(CursorLocation game_board_location);

// dropToken place the token in the game data array in the current column
// position and stacks the token on top of the highest unused row index.
boolean dropToken(GameData* game_data, int current_col_position);

// enableBlinkingText bolds, inverts, and blinks the text. Used for the player
// status bar and highlights the connect four tokens.
void enableBlinkingText();

// enableRawInputMode is user to prepare the terminal for the game.
int enableRawInputMode(struct termios OriginalTerm, char* error_message);

// enableTimeOutForRead enables a timeout for the read function().
void enableTimeOutForRead(struct termios* new_settings);

// endGame is used after the game is won to allow the player to determine if
// they want to replay the game or quit.
boolean endGame(GameData* game_data, char* error_message);

// exitProgram exits the game for both error and non error game states.
void exitProgram(TerminalSettings* terminal_settings, char* error_message);

// findBlankLineLocation returns the location to place the BLANK_LINE string,
// based of the center of the terminal. Only finds the col, row is not used.
CursorLocation findBlankLineLocation(TerminalSettings* terminal_settings);

// findConnectFourLocation returns the location to place the TITLE string, based
// of the center of the terminal.
CursorLocation
findConnectFourTitleLocation(TerminalSettings* terminal_settings);

char* findCurrentPlayersToken(int move_counter);

// findDirectionStatusBarLocation finds the location to place the
// DIRECTION_ARROW and DIRECTION_ENTER strings, based of the center of the
// terminal.
CursorLocation
findDirectionsStatusBarLocation(TerminalSettings* terminal_settings);

// findEndGameStatusBarLocation returns the location to place the P1WIN and
// P2WIN strings, based of the center of the terminal.
CursorLocation
findEndGameStatusBarLocation(TerminalSettings* terminal_settings);

// findFirstTokenLocation returns the location to place the token at the [0][0]
// index of the game data array, based of the center of the terminal.
CursorLocation findFirstTokenLocation(TerminalSettings* terminal_settings);

// findGameBoardLocation returns the location to place the game board, based of
// the center of the terminal.
CursorLocation findGameBoardLocation(TerminalSettings* terminal_settings);

// findPlayersInitialLocation returns the location to place the token being
// moved and dropped, based of the center of the terminal.
CursorLocation findPlayersInitialLocation(TerminalSettings* terminal_settings);

// findTurnStatusBarLocation returns the location to place the P1TURN and P2TURN
// strings, based of the center of the terminal.
CursorLocation findTurnStatusBarLocation(TerminalSettings* terminal_settings);

// findWinnerStatusBarLocation returns the location to place a the
// ENDGAME_DIRECTIONS string, based of the center of the terminal.
CursorLocation findWinnerStatusBarLocation(TerminalSettings* terminal_settings);

// gamePlayLoop contains the while loop that takes user input to move the
// players token and drop the token. error_message is used in case of failures.
boolean gamePlayLoop(GameData* game_data, char* error_message);

// getWindowSize gets the terminal size, which is used to center display.
int getWindowSize(int* out_rows, int* out_cols);

// hideCursor hides the cursor.
void hideCursor();

// initSettingsData initializes the elements of the termSettingData struct.
TerminalSettings initializeTerminalSettings(char* error_message);

// moveCursor moves the cursor by an amount in the direction by executing
// write().
void moveCursor(int amount, char* direction);

// moveTokenLeft moves the current token in play left.
void moveTokenLeft(char* current_players_token, int* current_position);

// moveTokenRight moves the current token in play right.
void moveTokenRight(char* current_players_token, int* current_position);

// placeTokenAtLeftBoundary moves the current token to the left boundary if the
// token is at the right boundary and the player uses the right arrow key.
void placeTokenAtLeftBoundary(char* current_players_token,
                              int* current_position);

// placeTokenAtRightBoundary moves the current token to the right boundary if
// the token is at the left boundary and the player uses the left arrow key.
void placeTokenAtRightBoundary(char* current_players_token,
                               int* current_position);

// playerInputReader returns the char that the player inputs from the keyboard.
int playerInputReader(char* player_input, char* error_message);

// putCursorAt puts the cursor at the row and col on the terminal.
void putCursorAt(int row, int col);

// recreateGame resets the gameDataElements to restart the game.
void recreateGame(TerminalSettings* terminal_settings, GameData* game_data);

// showConnectFour highlights the connect four tokens found by
// connectFourPresent functions.
void showConnectFour(GameData* game_data, int row, int col, int vector);

// turnOffCflags turns off CS8 flag. Used by enableRawInputMode.
void turnOffCflags(tcflag_t* c_cflag);

// turnOffIflags turns off BRKINT, ICRNL, INPCK, ISTRIP, IXON flags. Used by
// enableRawInputMode.
void turnOffIflags(tcflag_t* c_iflag);

// turnOffLflags turns off ECHO, ICANNON, IEXTEN, ISIG flags. Used by
// enableRawInputMode.
void turnOffLflags(tcflag_t* c_lflag);

// turnOffOflags turns off OPOST flag. Used by enableRawInputMode.
void turnOffOflags(tcflag_t* c_oflag);

// unhideCursor unhides the cursor.
void unhideCursor();

/*** Functions ***/

int applyNewterminal_settings(struct termios new_settings,
                              char* error_message) {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &new_settings) == -1) {
    strcat(error_message,
           "enableRawMode->applyNewterminal_settings->tcsetattr");
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

boolean connectFourPresent(GameData* game_data) {
  int row, col;
  // Search pattern starts at the bottom right hand corner and moves left. This
  // way only four of the eight possible vectors have to be checked.
  for (row = 6; row >= 0; --row) {
    for (col = 6; col >= 0; --col) {
      if (connectFourHorizontal(game_data->array, row, col)) {
        showConnectFour(game_data, row, col, HORIZONTAL);
        return TRUE;
      }
      if (connectFourLeftDiagonal(game_data->array, row, col)) {
        showConnectFour(game_data, row, col, LEFTDIAG);
        return TRUE;
      }
      if (connectFourVertical(game_data->array, row, col)) {
        showConnectFour(game_data, row, col, VERTICAL);
        return TRUE;
      }
      if (connectFourRightDiagonal(game_data->array, row, col)) {
        showConnectFour(game_data, row, col, RIGHTDIAG);
        return TRUE;
      }
    }
  }
  return FALSE;
}

boolean connectFourHorizontal(int array[7][7], int row, int col) {
  // Any index less than 4 cannot have a four or more tokens.
  if (col < 3) {
    return FALSE;
  }

  if (array[row][col] == EMPTY) {
    return FALSE;
  }
  // Returns 0 (not found) if the next three tokens do not match the initial
  // row/col token.
  int i;
  int temp_col = col;
  for (i = 0; i < 3; ++i) {
    if (array[row][col] != array[row][--temp_col]) {
      return FALSE;
    }
  }
  return TRUE;
}

boolean connectFourLeftDiagonal(int array[7][7], int row, int col) {
  // Any index less than 4 cannot have a four or more tokens.
  if (col < 3 || row < 3) {
    return FALSE;
  }

  if (array[row][col] == EMPTY) {
    return FALSE;
  }
  // Returns 0 (not found) if the next three tokens do not match the initial
  // row/col token.
  int i;
  int temp_col = col;
  int temp_row = row;
  for (i = 0; i < 3; ++i) {
    if (array[row][col] != array[--temp_row][--temp_col]) {
      return FALSE;
    }
  }
  return TRUE;
}

boolean connectFourRightDiagonal(int array[7][7], int row, int col) {
  // Any index less than 4 cannot have a four or more tokens.
  if (col > 3 || row < 3) {
    return FALSE;
  }
  if (array[row][col] == EMPTY) {
    return FALSE;
  }
  // Returns 0 (not found) if the next three tokens do not match the initial
  // row/col token.
  int i;
  int temp_col = col;
  int temp_row = row;
  for (i = 0; i < 3; ++i) {
    if (array[row][col] != array[--temp_row][++temp_col]) {
      return FALSE;
    }
  }
  return TRUE;
}

boolean connectFourVertical(int array[7][7], int row, int col) {
  // Any index less than 4 cannot have a four or more tokens.
  if (row < 3) {
    return FALSE;
  }
  if (array[row][col] == EMPTY) {
    return FALSE;
  }
  // Returns 0 (not found) if the next three tokens do not match the initial
  // row/col token.
  int i;
  int temp_row = row;
  for (i = 0; i < 3; ++i) {
    if (array[row][col] != array[--temp_row][col]) {
      return FALSE;
    }
  }
  return TRUE;
}

GameData createGameData(TerminalSettings* terminal_settings) {
  GameData NewGame;

  NewGame.move_counter = 0;

  // Populates array with 49 EMPTY tokes.
  int i, j;
  for (i = 0; i < 7; ++i) {
    for (j = 0; j < 7; ++j) {
      NewGame.array[i][j] = EMPTY;
    }
  }

  NewGame.connect_four_title_location =
      findConnectFourTitleLocation(terminal_settings);
  NewGame.game_board_location = findGameBoardLocation(terminal_settings);
  NewGame.first_token_location = findFirstTokenLocation(terminal_settings);
  NewGame.players_initial_location =
      findPlayersInitialLocation(terminal_settings);
  NewGame.directions_status_bar_location =
      findDirectionsStatusBarLocation(terminal_settings);
  NewGame.turn_status_bar_location =
      findTurnStatusBarLocation(terminal_settings);
  NewGame.winner_status_bar_location =
      findWinnerStatusBarLocation(terminal_settings);
  NewGame.blank_line_column_location = findBlankLineLocation(terminal_settings);
  NewGame.end_game_status_bar_location =
      findEndGameStatusBarLocation(terminal_settings);

  return NewGame;
}

void disableBlinkingText() {
  write(STDOUT_FILENO, BLINKING_OFF, strlen(BLINKING_OFF));
}

int disableRawInputMode(TerminalSettings* terminal_settings,
                        char* error_message) {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &terminal_settings->orig_termios) ==
      -1) {
    strcat(error_message, "die->disableRawInputMode->tcsetattr");
    return -1;
  }
  return 0;
}

void displayBlueColorText() { write(STDOUT_FILENO, BLUE_COLOR, 5); }

void displayCurrentPlayersToken(char* current_players_token) {
  if (strcmp(current_players_token, PLAYER1) == 0) {
    displayRedColorText();
  } else {
    displayYellowColorText();
  }
  displayStrings(current_players_token);
  moveCursor(1, LEFT);

  displayDefaultColorText();
}

void displayDefaultColorText() { write(STDOUT_FILENO, DEFAULT_COLOR, 5); }

void displayDirectionsStatusBar(GameData* game_data) {
  putCursorAt(game_data->directions_status_bar_location.row,
              game_data->blank_line_column_location.col);
  displayStrings(BLANK_LINE);
  putCursorAt(game_data->directions_status_bar_location.row + 1,
              game_data->blank_line_column_location.col);
  displayStrings(BLANK_LINE);

  putCursorAt(game_data->directions_status_bar_location.row,
              game_data->directions_status_bar_location.col);
  displayBlueColorText();
  displayStrings(DIRECTION_ARROW);
  putCursorAt(game_data->directions_status_bar_location.row + 1,
              game_data->directions_status_bar_location.col);
  displayStrings(DIRECTIONS_ENTER);
  displayDefaultColorText();
}

void displayEndGameStatusBar(GameData* game_data) {
  putCursorAt(game_data->end_game_status_bar_location.row,
              game_data->blank_line_column_location.col);
  displayStrings(BLANK_LINE);
  putCursorAt(game_data->end_game_status_bar_location.row + 1,
              game_data->blank_line_column_location.col);
  displayStrings(BLANK_LINE);

  putCursorAt(game_data->end_game_status_bar_location.row,
              game_data->end_game_status_bar_location.col);
  displayBlueColorText();
  displayStrings(ENDGAME_DIRECTIONS);
  displayDefaultColorText();
}

void displayGameBoard(GameData* game_data) {
  clearTerm();
  displayTitle(game_data->connect_four_title_location);
  displayDirectionsStatusBar(game_data);
  drawGameBoard(game_data->game_board_location);
}

void displayStrings(char* item) { write(STDOUT_FILENO, item, strlen(item)); }

void displayRedColorText() { write(STDOUT_FILENO, RED_COLOR, 5); }

void displayTitle(CursorLocation connect_four_title_location) {
  putCursorAt(connect_four_title_location.row, connect_four_title_location.col);
  displayStrings(TITLE);
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

void displayTokens(GameData* game_data) {
  int cursor_col = game_data->first_token_location.col;
  int cursor_row = game_data->first_token_location.row;
  int token_col, token_row;
  for (token_col = 0; token_col < 7; ++token_col) {
    for (token_row = 0; token_row < 7; ++token_row) {
      putCursorAt(cursor_row, cursor_col);
      displayTokenAt(game_data->array, token_col, token_row);
      // Plus 2 is used because there are two spaces between the rows on the
      // ASCII representation of the board.
      cursor_row += 2;
    }
    cursor_row = game_data->first_token_location.row;
    // Plus t4 is used because there are four spaces between the columns on the
    // ASCII representation of the board.
    cursor_col += 4;
  }
}

void displayTurnStatusBar(GameData* game_data) {
  putCursorAt(game_data->turn_status_bar_location.row,
              game_data->blank_line_column_location.col);
  displayStrings(BLANK_LINE);

  putCursorAt(game_data->turn_status_bar_location.row,
              game_data->turn_status_bar_location.col);
  if (game_data->move_counter % 2 == 0) {
    displayRedColorText();
    displayStrings(P1TURN);
    displayDefaultColorText();
  } else {
    displayYellowColorText();
    displayStrings(P2TURN);
    displayDefaultColorText();
  }
}

void displayWinStatusBar(GameData* game_data) {
  putCursorAt(game_data->winner_status_bar_location.row,
              game_data->blank_line_column_location.col);
  displayStrings(BLANK_LINE);

  putCursorAt(game_data->winner_status_bar_location.row,
              game_data->winner_status_bar_location.col);
  enableBlinkingText();
  if (game_data->move_counter % 2 == 0) {
    displayYellowColorText();
    displayStrings(P2WIN);
  } else {
    displayRedColorText();
    displayStrings(P1WIN);
  }
  displayDefaultColorText();
  disableBlinkingText();
}

void displayYellowColorText() { write(STDOUT_FILENO, YELLOW_COLOR, 5); }

void drawGameBoard(CursorLocation game_board_location) {
  putCursorAt(game_board_location.row, game_board_location.col);
  displayBlueColorText();

  int i, j;
  for (i = 0; i < 15; ++i) {
    moveCursor(1, DOWN);

    if (i % 2 == 0) {
      displayStrings(BOARDTOP);
    } else {
      for (j = 0; j < 29; ++j) {
        if (j % 4 == 0) {
          displayStrings("|");
        } else
          displayStrings(" ");
      }
    }
    moveCursor(29, LEFT);
  }

  displayDefaultColorText();
}

boolean dropToken(GameData* game_data, int current_col_position) {
  if (game_data->array[0][current_col_position] != EMPTY) {
    return FALSE;
  }

  int row;
  for (row = 6; row >= 0; row--) {
    if (game_data->array[row][current_col_position] == EMPTY) {
      if ((game_data->move_counter % 2) == 0) {
        game_data->array[row][current_col_position] = RED;
      } else {
        game_data->array[row][current_col_position] = YELLOW;
      }
      break;
    }
  }
  displayStrings(" ");
  return TRUE;
}

void enableBlinkingText() {
  write(STDOUT_FILENO, BLINKING_ON, strlen(BLINKING_ON));
}

int enableRawInputMode(struct termios OriginalTerm, char* error_message) {
  struct termios new_settings = OriginalTerm;

  // Documentation for termios.h flags:
  // pubs.opengroup.org/onlinepubs/000095399/basedefs/termios.h.html
  turnOffIflags(&new_settings.c_iflag);
  turnOffOflags(&new_settings.c_oflag);
  turnOffCflags(&new_settings.c_cflag);
  turnOffLflags(&new_settings.c_lflag);
  enableTimeOutForRead(&new_settings);
  return applyNewterminal_settings(new_settings, error_message);
}

void enableTimeOutForRead(struct termios* new_settings) {
  new_settings->c_cc[VMIN] = 0;
  new_settings->c_cc[VTIME] = 1;
}

boolean endGame(GameData* game_data, char* error_message) {
  displayEndGameStatusBar(game_data);

  char player_input;
  while (TRUE) {
    if (playerInputReader(&player_input, error_message) == -1) {
      return 0;
    }

    switch (player_input) {
    case 'y':
    case 'Y':
      return TRUE;
      break;
    case CTRL_KEY('q'):
    case 'n':
    case 'N':
      return FALSE;
      break;
    }
  }
}

void exitProgram(TerminalSettings* terminal_settings, char* error_message) {
  clearScreen();
  moveCursor(0, CORNER);
  unhideCursor();

  if (disableRawInputMode(terminal_settings, error_message) == -1) {
    strcat(error_message,
           " Failed to disable Raw Input Mode. Restart terminal.");
  }

  if (strcmp(error_message, NO_ERRORS) == 0) {
    exit(0);
  } else {
    perror(error_message);
    exit(1);
  }
}

CursorLocation findBlankLineLocation(TerminalSettings* terminal_settings) {
  CursorLocation BlankLineCol;
  BlankLineCol.col =
      (terminal_settings->screen_cols / 2) - centerText(BLANK_LINE);
  BlankLineCol.row = 0;
  return BlankLineCol;
}

CursorLocation
findConnectFourTitleLocation(TerminalSettings* terminal_settings) {
  CursorLocation Title;

  Title.col = (terminal_settings->screen_cols / 2) - centerText(TITLE);
  Title.row = (terminal_settings->screen_rows / 2) - 14;

  return Title;
}

char* findCurrentPlayersToken(int move_counter) {
  if (move_counter % 2 == 0) {
    return PLAYER1;
  } else {
    return PLAYER2;
  }
}

CursorLocation
findDirectionsStatusBarLocation(TerminalSettings* terminal_settings) {
  CursorLocation Direct;

  Direct.col =
      (terminal_settings->screen_cols / 2) - centerText(DIRECTION_ARROW);
  Direct.row = (terminal_settings->screen_rows / 2) + 12;

  return Direct;
}

CursorLocation
findEndGameStatusBarLocation(TerminalSettings* terminal_settings) {
  CursorLocation End;

  End.col =
      (terminal_settings->screen_cols / 2) - centerText(ENDGAME_DIRECTIONS);
  End.row = (terminal_settings->screen_rows / 2) + 12;

  return End;
}

CursorLocation findFirstTokenLocation(TerminalSettings* terminal_settings) {
  CursorLocation FirstToken;

  FirstToken.col =
      (terminal_settings->screen_cols / 2) - (centerText(BOARDTOP) - 2);
  FirstToken.row = (terminal_settings->screen_rows / 2) - 4;

  return FirstToken;
}

CursorLocation findGameBoardLocation(TerminalSettings* terminal_settings) {
  CursorLocation GameBoard;

  GameBoard.col = (terminal_settings->screen_cols / 2) - centerText(BOARDTOP);
  GameBoard.row = (terminal_settings->screen_rows / 2) - 6;

  return GameBoard;
}

CursorLocation findPlayersInitialLocation(TerminalSettings* terminal_settings) {
  CursorLocation Players;

  Players.col =
      (terminal_settings->screen_cols / 2) - (centerText(BOARDTOP) - 2);
  Players.row = (terminal_settings->screen_rows / 2) - 6;

  return Players;
}

CursorLocation findTurnStatusBarLocation(TerminalSettings* terminal_settings) {
  CursorLocation Turn;

  Turn.col = (terminal_settings->screen_cols / 2) - centerText(P1TURN);
  Turn.row = (terminal_settings->screen_rows / 2) - 8;

  return Turn;
}

CursorLocation
findWinnerStatusBarLocation(TerminalSettings* terminal_settings) {
  CursorLocation WinStatusBar;

  WinStatusBar.col = (terminal_settings->screen_cols / 2) - centerText(P1WIN);
  WinStatusBar.row = (terminal_settings->screen_rows / 2) - 8;

  return WinStatusBar;
}

boolean gamePlayLoop(GameData* game_data, char* error_message) {
  char* current_players_token =
      findCurrentPlayersToken(game_data->move_counter);
  putCursorAt(game_data->players_initial_location.row,
              game_data->players_initial_location.col);
  displayCurrentPlayersToken(current_players_token);

  char player_input;
  int current_player_turn = TRUE;
  int current_position = 0;
  while (current_player_turn) {
    if (playerInputReader(&player_input, error_message) == -1) {
      return FALSE;
    }

    switch (player_input) {
    // Used for quitting the game manually.
    case CTRL_KEY('q'):
      return FALSE;
      break;

    case RIGHT_ARROW:
      if (current_position == RIGHT_BOUNDARY) {
        placeTokenAtLeftBoundary(current_players_token, &current_position);
      } else {
        moveTokenRight(current_players_token, &current_position);
      }
      break;

    case LEFT_ARROW:
      if (current_position == LEFT_BOUNDARY) {
        placeTokenAtRightBoundary(current_players_token, &current_position);
      } else {
        moveTokenLeft(current_players_token, &current_position);
      }
      break;

    case ENTER:
      if (dropToken(game_data, current_position)) {
        current_player_turn = FALSE;
        game_data->move_counter++;
      }
      break;
    }
  }
  return TRUE;
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

TerminalSettings initializeTerminalSettings(char* error_message) {
  TerminalSettings OldSettings;
  OldSettings.successful_initialization = 0;

  if (tcgetattr(STDIN_FILENO, &OldSettings.orig_termios) == -1) {
    strcat(error_message, "initializeTerminalSettings->tcgetattr");
    OldSettings.successful_initialization = -1;
  }

  // find terminal window size to center the game display.
  if (getWindowSize(&OldSettings.screen_rows, &OldSettings.screen_cols) == -1) {
    strcat(error_message, "initialize_terminal_settings->getWindowSize");
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

void moveTokenLeft(char* current_players_token, int* current_position) {
  displayStrings(" ");
  moveCursor(5, LEFT);
  displayCurrentPlayersToken(current_players_token);
  *current_position = *current_position - 1;
}

void moveTokenRight(char* current_players_token, int* current_position) {
  displayStrings(" ");
  moveCursor(3, RIGHT);
  displayCurrentPlayersToken(current_players_token);
  *current_position = *current_position + 1;
}

void placeTokenAtLeftBoundary(char* current_players_token,
                              int* current_position) {
  displayStrings(" ");
  moveCursor(25, LEFT);
  displayCurrentPlayersToken(current_players_token);
  *current_position = LEFT_BOUNDARY;
}

void placeTokenAtRightBoundary(char* current_players_token,
                               int* current_position) {
  displayStrings(" ");
  moveCursor(23, RIGHT);
  displayCurrentPlayersToken(current_players_token);
  *current_position = RIGHT_BOUNDARY;
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

void showConnectFour(GameData* game_data, int row, int col, int vector) {
  enableBlinkingText();
  int i;
  int temp_col = col;
  int temp_row = row;
  switch (vector) {
  // Using the first token as a base, the rows and col have thier indexs
  // multipled by the distance between tokens(2 and 4 respectively) to overwrite
  // the token in that position with a blinking token.
  case HORIZONTAL:
    for (i = 0; i < 4; ++i) {
      putCursorAt(game_data->first_token_location.row + (temp_row * 2),
                  game_data->first_token_location.col + (temp_col * 4));
      displayTokenAt(game_data->array, temp_col--, temp_row);
    }

    break;

  case LEFTDIAG:
    for (i = 0; i < 4; ++i) {
      putCursorAt(game_data->first_token_location.row + (temp_row * 2),
                  game_data->first_token_location.col + (temp_col * 4));
      displayTokenAt(game_data->array, temp_col--, temp_row--);
    }

    break;

  case VERTICAL:
    for (i = 0; i < 4; ++i) {
      putCursorAt(game_data->first_token_location.row + (temp_row * 2),
                  game_data->first_token_location.col + (temp_col * 4));
      displayTokenAt(game_data->array, temp_col, temp_row--);
    }

    break;

  case RIGHTDIAG:
    for (i = 0; i < 4; ++i) {
      putCursorAt(game_data->first_token_location.row + (temp_row * 2),
                  game_data->first_token_location.col + (temp_col * 4));
      displayTokenAt(game_data->array, temp_col++, temp_row--);
    }

    break;
  }
  disableBlinkingText();
}

void recreateGame(TerminalSettings* terminal_settings, GameData* game_data) {

  *game_data = createGameData(terminal_settings);

  displayDirectionsStatusBar(game_data);
  displayTurnStatusBar(game_data);
  displayTokens(game_data);
}

void turnOffCflags(tcflag_t* c_cflag) {
  // CS8: misc flag
  *c_cflag |= (CS8);
}

void turnOffIflags(tcflag_t* c_iflag) {
  // BRKINT: misc flag, ICRNL: ctrl-m, INPCK: misc flag,
  // ISTRIP: misc flag, IXON: ctrl_s and ctrl_q
  *c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
}

void turnOffLflags(tcflag_t* c_lflag) {
  // ECHO:Print text to screen, ICANON:Canonical Mode, IEXTEN & ISIG:ctrl=c
  // and ctrl-v
  *c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
}

void turnOffOflags(tcflag_t* c_oflag) {
  // OPOST: output processing /r/n
  *c_oflag &= ~(OPOST);
}

void unhideCursor() { write(STDOUT_FILENO, UNHIDE, 6); }

/*** Main ***/

int main() {
  char error_message[50] = NO_ERRORS;

  // Cannot use exitProgram function for failure of initalizeterminal_settings
  // because the state of terminal_settings will be unknown. However, no
  // settings have been applied and the terminal is unchanged, exit(1) is
  // sufficent.
  TerminalSettings terminal_settings =
      initializeTerminalSettings(error_message);
  if (terminal_settings.successful_initialization == -1) {
    perror(error_message);
    exit(1);
  }
  // Enables raw input mode, exits program if an error is incurred.
  if (enableRawInputMode(terminal_settings.orig_termios, error_message) == -1) {
    exitProgram(&terminal_settings, error_message);
  }

  // initialized game data and draws the board / title.
  GameData game_data = createGameData(&terminal_settings);
  displayGameBoard(&game_data);

  int game_not_quit = TRUE;
  while (game_not_quit) {
    // The function displayTokens is prior to the function connectFourPresent so
    // that upon a winning move the last token placement is displayed.
    displayTokens(&game_data);

    // If connect four is present, show who won and give the option to restart
    // game or quit.
    if (connectFourPresent(&game_data)) {
      displayWinStatusBar(&game_data);
      if (endGame(&game_data, error_message) == FALSE) {
        break;
      } else {
        recreateGame(&terminal_settings, &game_data);
      }
    } else {
      displayTurnStatusBar(&game_data);
    }

    // Contains the main gameplay loop and returns if the player decided to quit
    // manually.
    game_not_quit = gamePlayLoop(&game_data, error_message);
  }

  // Exits the program for both error and non error modes.
  exitProgram(&terminal_settings, error_message);

  return 0;
}
