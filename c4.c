#include <stdio.h>







/*** prototypes ***/
	/*** init ***/

void enableRaw(); /*clears screen upon entering program*/
void disableRaw(); /*returns to previous environment after extiting program*/
void initArray(int array[][7]); /* creates 7x7 array and populates with EMPTY tokens*/
void drawGameBoard();	/* Graphically represents game to terminal in ASCII characters */

	/*** input ***/


	/*** Gameplay loop***/
void moveToken(); /*moves cursor above game board to drop token*/
void tokenPresent(); /*verify if toke is present and if so displays token correctly*/
void drop(); /*drops token at the bottom of the array or stacks*/
int connectFourPresent(); /*searches from right to left, bottom to top for connect 4*/






/***GameData***/
enum token {EMPTY = -1, YELLOW, RED};
int gameData[7][7];



/*** main ***/
int main()
{
	initArray(gameData);	

	

	


return 0;

}


/*** init ***/
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
