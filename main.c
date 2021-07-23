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

struct TerminalHandle {
  int screen_rows;
  int screen_cols;
  struct termios orig_termios;
};

typedef struct GameDataElements {
  struct TerminalHandle E;
  int array[7][7];
  int move_counter;
} GameDataElements;

/*** prototypes ***/

/** init **/

// clears screen upon entering program
void enableRawInputMode(GameDataElements* GameData);

// returns to previous environment after exiting program
void disableRawInputMode(GameDataElements* GameData);

// initializes array, move counter and screen_rows and screen_cols in GameData
// struct
void initGameData(GameDataElements* GameData);

// Graphically represents game to terminal in ASCII characters
void drawGameBoard(GameDataElements* GameData);

// gets terminal size to center display
int getWindowSize(int* out_rows, int* out_cols);

// error reader upon failure
void die(GameDataElements* GameData, const char* s);

/** Gameplay loop **/

// moves cursor above game board to drop token
void playerInput(GameDataElements* GameData);

// verify if token is present and if so displays token correctly
char* tokenAtIndex(GameDataElements* GameData, int y, int x);

// drops token at the bottom of the array or stacks
int drop(GameDataElements* GameData, int index);

// searches from right to left, bottom to top for connect 4
int connectFourPresent();

/** Game wrapup **/

// endgame upon winning
void endGame();

// quitting with ctrl-q
void quitGame();

// endgame and reinit if players want to replay
void resetGame();

/** Status Bars **/

// normally displayed directions
void directionsStatusBar();

// changes depending on players turn
void playerTurnStatusBar();

// upon connect four present true
void winnerStatusBar();

/** Misc **/

// Helper function for constructing and executing write()
void moveCursor(int amount, char* direction);

// Helper function for displaying strings to game by executing write
void display(char* item);

// Helper funtion the clears screen
void clearScreen();

// Helper funtion the hides the cursor
void hideCursor();

// Helper function that unhids the cursor
void unhideCursor();

/*** init ***/

void enableRawInputMode(GameDataElements* GameData) {
  // error processing
  if (tcgetattr(STDIN_FILENO, &GameData->E.orig_termios) == -1) {
    die(GameData, "tsgetattr");
  }

  struct termios raw = GameData->E.orig_termios;
  // termios.h flag documentation:
  // pubs.opengroup.org/onlinepubs/000095399/basedef/termios.h.html BRKINT:
  // misc flag, ICRNL: ctrl-m, INPCK: misc flag, ISTRIP: misc flag, IXON:
  // ctrl_s and ctrl_q
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  // OPOST: output processing /r/n
  raw.c_oflag &= ~(OPOST);
  // CS8: misc flag
  raw.c_cflag |= (CS8);
  // ECHO:Print text to screen, ICANON:Canonical Mode, IEXTEN & ISIG:ctrl=c
  // and ctrl-v
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  // timeout for read()
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
    die(GameData, "tcsetattr");
  }
}

void disableRawInputMode(GameDataElements* GameData) {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &GameData->E.orig_termios) == -1) {
    die(GameData, "tcsetattr");
  }
}

void initGameData(GameDataElements* GameData) {

  // sets move counter to 0
  GameData->move_counter = 0;

  // populate array with 49 EMPTY tokes
  int i, j;
  for (i = 0; i < 7; i++) {
    for (j = 0; j < 7; j++) {
      GameData->array[i][j] = EMPTY;
    }
  }

  // determine screen size
  if (getWindowSize(&GameData->E.screen_rows, &GameData->E.screen_cols) == -1) {
    die(GameData, "getWindowSize");
  }
}

void drawGameBoard(GameDataElements* GameData) {
  // hides cursor
  hideCursor();

  // clears screen
  clearScreen();

  // return cursor to 1,1
  moveCursor(0, CORNER);

  // centers and moves cursor to the beginning of the game screen, and writes
  // "CONNECT FOUR"
  int i, j;
  for (i = 0; i < (GameData->E.screen_rows / 2) - 13; i++) {
    moveCursor(1, DOWN);
  }
  for (i = 0; i < (GameData->E.screen_cols / 2) - 6; i++) {
    moveCursor(1, RIGHT);
  }
  write(STDOUT_FILENO, "CONNECT FOUR", 12);

  // moves down to where the board will be
  moveCursor(7, DOWN);

  moveCursor(21, LEFT);

  // draws game board
  int counter_x = 0;
  int counter_y = 0;
  for (i = 0; i < 15; i++) {
    moveCursor(1, DOWN);

    if (i % 2 == 0) {
      display("+---+---+---+---+---+---+---+");
    } else {
      for (j = 0; j < 29; j++) {
        if (j % 4 == 0) {
          display("|");
        } else if ((j - 2) % 4 == 0) {
          display(tokenAtIndex(GameData, counter_y++, counter_x));
        } else
          display(" ");
      }
      counter_y = 0;
      counter_x++;
    }
    moveCursor(29, LEFT);
  }

  // draws status bars
  directionsStatusBar();
  playerTurnStatusBar();
  winnerStatusBar();
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

void die(GameDataElements* GameData, const char* s) {
  // clear screen
  clearScreen();
  // move cursor to upper left hand corner
  moveCursor(0, CORNER);
  // unhide cursor
  unhideCursor();

  disableRawInputMode(GameData);

  perror(s);
  exit(1);
}

/*** Gameplay Loop ***/

void playerInput(GameDataElements* GameData) {
  // establishes token type by whose turn it is
  char* token;
  if (GameData->move_counter % 2 == 0) {
    token = "X";
  } else {
    token = "O";
  }

  // places cursor above graphic representation of array[0][0]
  moveCursor(15, UP);
  moveCursor(2, RIGHT);
  display(token);
  moveCursor(1, LEFT);

  // loop for player entry
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
    // if arrow key was used, figures out which one (finding the third
    // character)
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

    // for exiting the game
    switch (c) {
    case CTRL_KEY('q'):
      // clear screen
      clearScreen();
      // move cursor to upper left hand corner
      moveCursor(0, CORNER);
      // unhide cursor
      unhideCursor();
      disableRawInputMode(GameData);
      exit(0);
      break;

    // right arrow
    case 'C':
      // wrap logic
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

    // left arrow
    case 'D':
      // wrap logic
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
    // enter , drops token and end players turn
    case '\r':
      if (drop(GameData, index)) {
        player_turn--;
      }

      break;
    }
  }
}

char* tokenAtIndex(GameDataElements* GameData, int y, int x) {
  // RED
  if (GameData->array[x][y] == 0) {
    return "X";
  }

  // YELLOW
  if (GameData->array[x][y] == 1) {
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

void display(char* item) { write(STDOUT_FILENO, item, strlen(item)); }

void clearScreen() { write(STDOUT_FILENO, CLEAR, 4); }

void hideCursor() { write(STDOUT_FILENO, HIDE, 6); }

void unhideCursor() { write(STDOUT_FILENO, UNHIDE, 6); }

/*** main ***/

int main() {
  GameDataElements GameData;

  enableRawInputMode(&GameData);
  initGameData(&GameData);

  while (1) {
    // drawGameBoard is prior to connectFourPresent so that upon a winning move
    // the last token placement is displayed
    drawGameBoard(&GameData);
    connectFourPresent(&GameData);
    playerInput(&GameData);
    GameData.move_counter++;
  }

  return 0;
}
