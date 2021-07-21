#include <stdio.h>







/*** prototypes ***/
	/*** init ***/

void enableRaw(); /*clears screen upon entering program*/
void disableRaw(); /*returns to previous environment after extiting program*/
void initArray(int array[][7]); /* creates 7x7 array and populates with EMPTY tokens*/
void drawGameBoard(int array[][7]);	/* Graphically represents game to terminal in ASCII characters */

	/*** input ***/
void moveCursor();  /*moves cursor accross 7 possible zones above board */
void moveFloatingToken(); /*moves representation of token to drop under cursor */

	/*** Gameplay loop***/
void moveToken(); /*moves cursor above game board to drop token, also contains logic for quitting*/
char tokenPresent(int array[][7], int x, int y);/*verify if toke is present and if so displays token correctly*/
void drop(); /*drops token at the bottom of the array or stacks*/
int connectFourPresent(); /*searches from right to left, bottom to top for connect 4*/

	/*** Game wrapup***/
void endGame(); /*endgame upon winning */
void quitGame(); /*quitting with ctrl-q */
void resetGame(); /*endgame and reinit if players want to replay*/

	/*** Status Bars***/
void directionsSB(); /*normally displayed directions*/
void playerTurnSB(); /*changes depending on players turn*/
void winnerSB();     /*upon connect four present true*/



/***GameData***/
enum token {EMPTY = -1, YELLOW, RED};



/*** main ***/

int main()
{
	int gameData[7][7];
	//int moveCounter = 0;  /* might need to track move, could be used for to decide whose turn it is */


	initArray(gameData);	

//	while(1)
//	{
		/* drawGameBoard is prior to connectFourPresent so that upon */
		/* a winning move the last token placement is displayed */
		
		drawGameBoard(gameData);		
		connectFourPresent();
		moveToken();	
//	}	

	return 0;

}


/*** init ***/



void enableRaw()
{

}



void disableRaw()
{

}



void initArray(int array[][7])
{
	int i,j;

	for (i=0; i<7; i++)
	{
		for (j=0; j<7; j++)
		{
			array[i][j] = EMPTY;
		}
	} 
} 



void drawGameBoard(int array[][7])
{
	/*Two possible ways:*/
	/* Display one long script with 49 calls to tokenPresent()*/
	/* 	ie "| %c | %c | %c | %c | %c | %c | %c |", tokenPresent(),tokenPresent()....   */
	/* Or */
	/* For loop with if statments using index%x to determine which line should have which charcters */
	/* second for loop for the lines that contain tokens to use tokenPresent with a counter */     
 

	/* ideal shape  */

		/* +---+---+---+---+---+---+---+ */
		/* |   |   |   |   |   |   |   | */
		/* +---+---+---+---+---+---+---+ */
		/* |   |   |   |   |   |   |   | */
		/* +---+---+---+---+---+---+---+ */
		/* |   |   |   |   |   |   |   | */
		/* +---+---+---+---+---+---+---+ */
		/* |   |   | X |   |   |   |   | */
		/* +---+---+---+---+---+---+---+ */
		/* |   |   | X |   |   |   |   | */
		/* +---+---+---+---+---+---+---+ */
		/* |   |   | O |   |   | X |   | */
		/* +---+---+---+---+---+---+---+ */
		/* | X | O | X | X | O | O |   | */
		/* +---+---+---+---+---+---+---+ */


//	printf("+---+---+---+---+---+---+---+\n| %d | %d | %d | %d | %d | %d | %d |\n+---+---+---+---+---+---+---+\n| %d | %d | %d | %d | %d | %d | %d |\n+---+---+---+---+---+---+---+\n| %d | %d | %d | %d | %d | %d | %d |\n+---+---+---+---+---+---+---+\n| %d | %d | %d | %d | %d | %d | %d |\n+---+---+---+---+---+---+---+\n| %d | %d | %d | %d | %d | %d | %d |\n+---+---+---+---+---+---+---+\n| %d | %d | %d | %d | %d | %d | %d |\n+---+---+---+---+---+---+---+\n| %d | %d | %d | %d | %d | %d | %d |\n+---+---+---+---+---+---+---+\n", array[0][0],array[0][0],array[0][0],array[0][0],array[0][0],array[0][0],array[0][0],array[0][0],array[0][0],array[0][0],array[0][0],array[0][0],array[0][0],array[0][0],array[0][0],array[0][0],array[0][0],array[0][0],array[0][0],array[0][0],array[0][0],array[0][0],array[0][0],array[0][0],array[0][0],array[0][0],array[0][0],array[0][0],array[0][0],array[0][0],array[0][0],array[0][0],array[0][0],array[0][0],array[0][0],array[0][0],array[0][0],array[0][0],array[0][0],array[0][0],array[0][0],array[0][0],array[0][0],array[0][0],array[0][0],array[0][0],array[0][0],array[0][0],array[0][0]);
	


	int i,j;
	for (i=0; i<15; i++)
	{
		if(i%2 == 0) printf("+---+---+---+---+---+---+---+\n");
		else
		{
			for(j=0;j<29;j++)
			{
				if(j%4 == 0) printf("|");
				else if( (j-2)%4 == 0) printf("%d", 0);
				else printf(" ");

					
			}
			printf("\n");

		}



	}


}


/*** Input ***/

void moveCursor()
{

}



void moveFloatingToken() 
{

}



/*** Gameplay Loop ***/

void moveToken() /* section of code where all player interaction occurs */
{

	while (1)
	{
		/* switch statement for arrows, ctrl-q, enter */

			/*arrows : moveCursor(), and moveFloatingToken() */
			/*enter : drop() */
			/*Ctrl-Q : quitGame() */

	}

}




char tokenPresent(int array[][7], int x, int y)
{
	int temp;
	temp=array[x][y];
	
	if (temp == 1) return '0';
	if (temp == 0) return 'O';
//	if (temp < 0 ) return ' ';

	return ' ';
}



void drop()
{

}




int connectFourPresent()
{


 return 1;
}




/*** Game wrapup***/

void endGame()
{

}



void quitGame()
{

}



void resetGame()
{

}



/*** Status Bars***/

void directionsSB()
{

}


void playerTurnSB()
{

}



void winnerSB()
{

}

