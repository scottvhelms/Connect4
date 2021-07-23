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

/*** enum ***/

enum token
{
    EMPTY = -1,
    RED,
    YELLOW
};

/*** Structures ***/

struct editorConfig
{
    int screenrows;
    int screencols;
    struct termios orig_termios;
};

typedef struct gameDataElements
{
    struct editorConfig E;
    int array[7][7];
    int moveCounter;
} gameDataElements;

/*** prototypes ***/

/** init **/

// clears screen upon entering program
void enableRawInputMode(gameDataElements* gameData);

// returns to previous environment after extiting program
void disableRawInputMode(gameDataElements* gameData);

// initalizes array, move counter and screenrows and screencols in gameData
// struct
void initGameData(gameDataElements* gameData);

// Graphically represents game to terminal in ASCII characters
void drawGameBoard(gameDataElements* gameData);

// gets terminal size to center display
int getWindowSize(int* out_rows, int* out_cols);

// error reader upon failure
void die(gameDataElements* gameData, const char* s);

/** Gameplay loop **/

// moves cursor above game board to drop token
void moveToken(gameDataElements* gameData);

// verify if token is present and if so displays token correctly
char* tokenPresent(gameDataElements* gameData, int y, int x);

// drops token at the bottom of the array or stacks
int drop(gameDataElements* gameData, int index);

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

/*** main ***/

int main()
{
    gameDataElements gameData;

    enableRawInputMode(&gameData);
    initGameData(&gameData);

    // drawGameBoard is prior to connectFourPresent so that upon a winning move
    // the last token placement is displayed
    while (1)
    {
        drawGameBoard(&gameData);
        connectFourPresent(&gameData);
        moveToken(&gameData);
        gameData.moveCounter++;
    }

    return 0;
}

/*** init ***/

void enableRawInputMode(gameDataElements* gameData)
{
    // error processing
    if (tcgetattr(STDIN_FILENO, &gameData->E.orig_termios) == -1)
    {
        die(gameData, "tsgetattr");
    }

    struct termios raw = gameData->E.orig_termios;
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

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
    {
        die(gameData, "tcsetattr");
    }
}

void disableRawInputMode(gameDataElements* gameData)
{
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &gameData->E.orig_termios) == -1)
    {
        die(gameData, "tcsetattr");
    }
}

void initGameData(gameDataElements* gameData)
{

    // sets move counter to 0
    gameData->moveCounter = 0;

    // populate array with 49 EMPTY tokes
    int i, j;
    for (i = 0; i < 7; i++)
    {
        for (j = 0; j < 7; j++)
        {
            gameData->array[i][j] = EMPTY;
        }
    }

    // determine screen size
    if (getWindowSize(&gameData->E.screenrows, &gameData->E.screencols) == -1)
    {
        die(gameData, "getWindowSize");
    }
}

void drawGameBoard(gameDataElements* gameData)
{
    // hides cursor
    write(STDOUT_FILENO, "\x1b[?25l", 6);

    // clears screen
    write(STDOUT_FILENO, "\x1b[2J", 4);

    // return cursor to 1,1
    moveCursor(0, CORNER);

    // centers and moves cursor to the beginning of the game screen, and writes
    // "CONNECT FOUR"
    int i, j;
    for (i = 0; i < (gameData->E.screenrows / 2) - 13; i++)
    {
        moveCursor(1, DOWN);
    }
    for (i = 0; i < (gameData->E.screencols / 2) - 6; i++)
    {
        moveCursor(1, RIGHT);
    }
    write(STDOUT_FILENO, "CONNECT FOUR", 12);

    // moves down to were the board will be
    moveCursor(7, DOWN);

    moveCursor(21, LEFT);

    // draws game board
    int counterx = 0;
    int countery = 0;
    for (i = 0; i < 15; i++)
    {
        moveCursor(1, DOWN);

        if (i % 2 == 0)
        {
            write(STDOUT_FILENO, "+---+---+---+---+---+---+---+", 29);
        }
        else
        {
            for (j = 0; j < 29; j++)
            {
                if (j % 4 == 0)
                {
                    write(STDOUT_FILENO, "|", 1);
                }
                else if ((j - 2) % 4 == 0)
                {
                    write(STDOUT_FILENO,
                          tokenPresent(gameData, countery++, counterx), 1);
                }
                else
                    write(STDOUT_FILENO, " ", 1);
            }
            countery = 0;
            counterx++;
        }
        moveCursor(29, LEFT);
    }

    // draws status boards
    directionsStatusBar();
    playerTurnStatusBar();
    winnerStatusBar();
}

int getWindowSize(int* out_rows, int* out_cols)
{
    struct winsize ws;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0)
    {
        return -1;
    }
    else
    {
        *out_cols = ws.ws_col;
        *out_rows = ws.ws_row;
        return 0;
    }
}

void die(gameDataElements* gameData, const char* s)
{
    // clear screen
    write(STDOUT_FILENO, "\x1b[2J", 4);
    // move cursor to upper left hand corner
    moveCursor(0, CORNER);
    // unhide cursor
    write(STDOUT_FILENO, "\x1b[?25h", 6);

    disableRawInputMode(gameData);

    perror(s);
    exit(1);
}

/*** Gameplay Loop ***/

void moveToken(gameDataElements* gameData)
{
    // establishes token type by whose turn it is
    char* token;
    if (gameData->moveCounter % 2 == 0)
    {
        token = "X";
    }
    else
    {
        token = "O";
    }

    // places cursor above graphic representation of array[0][0]
    moveCursor(15, UP);
    moveCursor(2, RIGHT);
    write(STDOUT_FILENO, token, 1);
    moveCursor(1, LEFT);

    // loop for player entry
    int nread;
    char c;
    int playerTurn = 1;
    int index = 0;
    while (playerTurn)
    {
        while ((nread = read(STDIN_FILENO, &c, 1)) != 1)
        {
            if (nread == -1 && errno != EAGAIN)
            {
                die(gameData, "read");
            }
        }
        // if arrow key was used, figures out which one (finding the third
        // character)
        if (c == '\x1b')
        {
            if (read(STDIN_FILENO, &c, 1) == -1)
            {
                die(gameData, "read");
            }

            if (c == '[')
            {
                if (read(STDIN_FILENO, &c, 1) == -1)
                {
                    die(gameData, "read");
                }
            }
        }

        // for extiting the game
        switch (c)
        {
            case CTRL_KEY('q'):
                // clear screen
                write(STDOUT_FILENO, "\x1b[2J", 4);
                // move cursor to upper left hand corner
                moveCursor(0, CORNER);
                // unhide cursor
                write(STDOUT_FILENO, "\x1b[?25h", 6);
                disableRawInputMode(gameData);
                exit(0);
                break;

            // right arrow
            case 'C':
                // wrap logic
                if (index == 6)
                {
                    write(STDOUT_FILENO, " ", 1);
                    moveCursor(25, LEFT);
                    write(STDOUT_FILENO, token, 1);
                    moveCursor(1, LEFT);

                    index = 0;
                }
                else
                {
                    write(STDOUT_FILENO, " ", 1);
                    moveCursor(3, RIGHT);
                    write(STDOUT_FILENO, token, 1);
                    moveCursor(1, LEFT);

                    index++;
                }

                break;

            // left arrow
            case 'D':
                // wrap logic
                if (index == 0)
                {
                    write(STDOUT_FILENO, " ", 1);
                    moveCursor(23, RIGHT);
                    write(STDOUT_FILENO, token, 1);
                    moveCursor(1, LEFT);

                    index = 6;
                }
                else
                {
                    write(STDOUT_FILENO, " ", 1);
                    moveCursor(5, LEFT);
                    write(STDOUT_FILENO, token, 1);
                    moveCursor(1, LEFT);

                    index--;
                }

                break;
            // enter , drops token and end players turn
            case '\r':
                if (drop(gameData, index))
                {
                    playerTurn--;
                }

                break;
        }
    }
}

char* tokenPresent(gameDataElements* gameData, int y, int x)
{
    // RED
    if (gameData->array[x][y] == 0)
    {
        return "X";
    }

    // YELLOW
    if (gameData->array[x][y] == 1)
    {
        return "O";
    }

    // EMPTY
    return " ";
}

int drop(gameDataElements* gameData, int index)
{

    if (gameData->array[0][index] != EMPTY)
    {
        return 0;
    }

    int i;
    for (i = 6; i >= 0; i--)
    {
        if (gameData->array[i][index] == EMPTY)
        {
            if ((gameData->moveCounter % 2) == 0)
            {
                gameData->array[i][index] = RED;
            }
            else
            {
                gameData->array[i][index] = YELLOW;
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

void moveCursor(int amount, char* direction)
{

    char esc[10] = ESC;
    if (amount > 1)
    {
        char buffer[10];

        sprintf(buffer, "%d", amount);
        strcat(esc, buffer);
    }
    strcat(esc, direction);

    write(STDOUT_FILENO, esc, sizeof(esc));
}
