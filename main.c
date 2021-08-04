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

// This function clears screen upon entering program.
void enableRawInputMode(GameDataElements* GameData);

// This function returns to previous terminal environment after exiting program.
void disableRawInputMode(GameDataElements* GameData);

// This function initializes the elements of the GameData struct.
void initGameData(GameDataElements* GameData);

// This function graphically represents game in the terminal in ASCII
// characters.
void drawGameBoard(GameDataElements* GameData);

// This function gets the terminal size, which is used to center display.
int getWindowSize(int* out_rows, int* out_cols);

// This function reads an error name upon program failure.
void die(GameDataElements* GameData, const char* s);

/** Gameplay loop **/

// This function moves cursor above game board in order to drop token.
void playerInput(GameDataElements* GameData);

// This function verifies if atoken is present at the y and x index.
// If token is present, it is displayed on the screen.
char* tokenAtIndex(GameDataElements* GameData, int y, int x);

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

/*** init ***/

void enableRawInputMode(GameDataElements* GameData) {
  // This is used for error processing incase of a failure with tcgetattr.
  if (tcgetattr(STDIN_FILENO, &GameData->E.orig_termios) == -1) {
    die(GameData, "tcgetattr");
  }

  struct termios raw = GameData->E.orig_termios;
  // Documentation for termios.h flags:
  // pubs.opengroup.org/onlinepubs/000095399/basedefs/termios.h.html
  //
  // BRKINT: misc flag, ICRNL: ctrl-m, INPCK: misc flag,
  // ISTRIP: misc flag, IXON: ctrl_s and ctrl_q
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  // OPOST: output processing /r/n
  raw.c_oflag &= ~(OPOST);
  // CS8: misc flag
  raw.c_cflag |= (CS8);
  // ECHO:Print text to screen, ICANON:Canonical Mode, IEXTEN & ISIG:ctrl=c
  // and ctrl-v
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  // This enables the timeout for read().
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
  // Initializes move_counter to 0.
  GameData->move_counter = 0;

  // Populates array with 49 EMPTY tokes.
  int i, j;
  for (i = 0; i < 7; i++) {
    for (j = 0; j < 7; j++) {
      GameData->array[i][j] = EMPTY;
    }
  }

  // Determine terminal window size to center the game display.
  if (getWindowSize(&GameData->E.screen_rows, &GameData->E.screen_cols) == -1) {
    die(GameData, "getWindowSize");
  }
}

void drawGameBoard(GameDataElements* GameData) {
  hideCursor();
  clearScreen();
  moveCursor(0, CORNER);

  // Centers and moves cursor to the beginning of the game screen, and writes
  // "CONNECT FOUR".
  int i, j;
  for (i = 0; i < (GameData->E.screen_rows / 2) - 13; i++) {
    moveCursor(1, DOWN);
  }
  for (i = 0; i < (GameData->E.screen_cols / 2) - 6; i++) {
    moveCursor(1, RIGHT);
  }
  write(STDOUT_FILENO, "CONNECT FOUR", 12);

  // Moves cursor to where the board will be.
  moveCursor(7, DOWN);
  moveCursor(21, LEFT);

  // Draws the game board.
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

  // Displays the status bars.
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
  clearScreen();
  moveCursor(0, CORNER);
  unhideCursor();

  disableRawInputMode(GameData);

  perror(s);
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

  // Places the cursor above graphic representation of array[0][0].
  moveCursor(15, UP);
  moveCursor(2, RIGHT);
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
      disableRawInputMode(GameData);
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
    // The function drawGameBoard is prior to the function connectFourPresent so
    // that upon a winning move the last token placement is displayed.
    drawGameBoard(&GameData);
    connectFourPresent(&GameData);
    playerInput(&GameData);
    GameData.move_counter++;
  }

  return 0;
}
