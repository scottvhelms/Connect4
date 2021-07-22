// Connect Four
// Author: Scott Helms
//
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#define CTRL_KEY(k) ((k)&0x1f)

/*** prototypes ***/

/*** init ***/
void enableRaw();     // clears screen upon entering program
void disableRaw();    // returns to previous environment after extiting program
void initArray();     // creates 7x7 array and populates with EMPTY tokens
void drawGameBoard(); // Graphically represents game to terminal in ASCII
                      // characters
int getWindowSize(int *rows, int *cols);
void die(const char *s); // error reader upon failure

/*** Gameplay loop***/
void moveToken(); // moves cursor above game board to drop token
char *tokenPresent(
    int y,
    int x); // verify if toke is present and if so displays token correctly
int drop(int index);      // drops token at the bottom of the array or stacks
int connectFourPresent(); // searches from right to left, bottom to top for
                          // connect 4

/*** Game wrapup***/
void endGame();   // endgame upon winning
void quitGame();  // quitting with ctrl-q
void resetGame(); // endgame and reinit if players want to replay

/*** Status Bars***/
void directionsSB(); // normally displayed directions
void playerTurnSB(); // changes depending on players turn
void winnerSB();     // upon connect four present true

/***Structures***/
struct editorConfig {
  int screenrows;
  int screencols;
  struct termios orig_termios;
};

/***GameData***/
enum token { EMPTY = -1, RED, YELLOW };
struct editorConfig E;
int array[7][7];
int moveCounter;

/*** main ***/

int main() {

  enableRaw();
  initArray();

  while (1) {
    // drawGameBoard is prior to connectFourPresent so that upon
    // a winning move the last token placement is displayed

    drawGameBoard();
    connectFourPresent();
    moveToken();
    moveCounter++;
  }

  return 0;
}

/*** init ***/

void enableRaw() {
  if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1)
    die("tsgetattr");
  atexit(disableRaw);

  struct termios raw = E.orig_termios;

  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  raw.c_oflag &= ~(OPOST);
  raw.c_cflag |= (CS8);
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
    die("tcsetattr");
}

void disableRaw() {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1)
    die("tcsetattr");
}

void initArray() {
  int i, j;
  moveCounter = 0;
  for (i = 0; i < 7; i++) {
    for (j = 0; j < 7; j++) {
      array[i][j] = EMPTY;
    }
  }
}

void drawGameBoard() {
  int i, j;
  int counterx = 0;
  int countery = 0;

  write(STDOUT_FILENO, "\x1b[?25l", 6);

  if (getWindowSize(&E.screenrows, &E.screencols) == -1)
    die("getWindowSize"); // determine screen size

  write(STDOUT_FILENO, "\x1b[2J", 4); // clears screen
  write(STDOUT_FILENO, "\x1b[H", 3);  // return cursor to 1,1

  for (i = 0; i < (E.screenrows / 2) - 13; i++)
    write(STDOUT_FILENO, "\x1b[B",
          3); // centers and moves cursor to the beginning of the game screen
  for (i = 0; i < (E.screencols / 2) - 6; i++)
    write(STDOUT_FILENO, "\x1b[C", 3);

  write(STDOUT_FILENO, "CONNECT FOUR", 12);
  write(STDOUT_FILENO, "\x1b[7B", 4); // moves down to were the board will be
  write(STDOUT_FILENO, "\x1b[21D", 5);

  for (i = 0; i < 15; i++) // draws game board
  {
    write(STDOUT_FILENO, "\x1b[B", 3);
    if (i % 2 == 0) {

      write(STDOUT_FILENO, "+---+---+---+---+---+---+---+", 29);
    } else {
      for (j = 0; j < 29; j++) {
        if (j % 4 == 0) {
          write(STDOUT_FILENO, "|", 1);

        } else if ((j - 2) % 4 == 0) {
          write(STDOUT_FILENO, tokenPresent(countery++, counterx), 1);
        } else
          write(STDOUT_FILENO, " ", 1);
      }
      countery = 0;
      counterx++;
    }
    write(STDOUT_FILENO, "\x1b[29D", 5);
  }

  directionsSB();
  playerTurnSB();
  winnerSB();
}

int getWindowSize(int *rows, int *cols) {
  struct winsize ws;

  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
    return -1;
  } else {
    *cols = ws.ws_col;
    *rows = ws.ws_row;
    return 0;
  }
}

void die(const char *s) {
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);
  write(STDOUT_FILENO, "\x1b[?23h", 6);

  perror(s);
  exit(1);
}

/*** Gameplay Loop ***/

void moveToken() {
  int nread;
  char c;
  char *token;
  int index = 0;
  int end = 1;

  if (moveCounter % 2 == 0)
    token = "X";
  else
    token = "O";

  write(STDOUT_FILENO, "\x1b[15A", 5); // cursor position and controll
  write(STDOUT_FILENO, "\x1b[2C", 4);
  write(STDOUT_FILENO, token, 1); // FIXME
  write(STDOUT_FILENO, "\x1b[D", 3);

  while (end) {
    while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
      if (nread == -1 && errno != EAGAIN)
        die("read");
    }

    if (c == '\x1b') {
      if (read(STDIN_FILENO, &c, 1) == -1)
        die("read");
      if (c == '[') {
        if (read(STDIN_FILENO, &c, 1) == -1)
          die("read");
      }
    }

    switch (c) {
    case CTRL_KEY('q'):
      write(STDOUT_FILENO, "\x1b[2J", 4);
      write(STDOUT_FILENO, "\x1b[H", 3);
      write(STDOUT_FILENO, "\x1b[?23h", 6);
      exit(0);
      break;

    case 'C': // right
      if (index == 6) {
        write(STDOUT_FILENO, " ", 1);
        write(STDOUT_FILENO, "\x1b[25D", 5);
        write(STDOUT_FILENO, token, 1);
        write(STDOUT_FILENO, "\x1b[D", 3);

        index = 0;
      }

      else {
        write(STDOUT_FILENO, " ", 1);
        write(STDOUT_FILENO, "\x1b[3C", 4);
        write(STDOUT_FILENO, token, 1);
        write(STDOUT_FILENO, "\x1b[D", 3);

        index++;
      }

      break;
    case 'D': // left
      if (index == 0) {
        write(STDOUT_FILENO, " ", 1);
        write(STDOUT_FILENO, "\x1b[23C", 5);
        write(STDOUT_FILENO, token, 1);
        write(STDOUT_FILENO, "\x1b[D", 3);

        index = 6;
      }

      else {
        write(STDOUT_FILENO, " ", 1);
        write(STDOUT_FILENO, "\x1b[5D", 4);
        write(STDOUT_FILENO, token, 1);
        write(STDOUT_FILENO, "\x1b[D", 3);

        index--;
      }

      break;

    case '\r':
      if (drop(index))
        end--;

      break;
    }
  }
}

char *tokenPresent(int y, int x) {

  if (array[x][y] == 0)
    return "X"; // RED
  if (array[x][y] == 1)
    return "O"; // YELLOW
  return " ";   // EMPTY
}

int drop(int index) {
  int i;

  if (array[0][index] != EMPTY)
    return 0;
  for (i = 6; i >= 0; i--) {
    if (array[i][index] == EMPTY) {
      if ((moveCounter % 2) == 0) {
        array[i][index] = RED;

      } else {
        array[i][index] = YELLOW;
      }
      break;
    }
  }
  return 1;
}

int connectFourPresent() { return 1; }

/*** Game wrapup***/

void endGame() {}

void quitGame() {}

void resetGame() {}

/*** Status Bars***/

void directionsSB() {}

void playerTurnSB() {}

void winnerSB() {}
