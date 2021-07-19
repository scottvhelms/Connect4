#include <stdio.h>







/*** prototypes ***/
	/*** init ***/

void enableRaw(); /*clears screen upon entering program*/
void disableRaw(); /*returns to previous environment after extiting program*/
void initArray(int array[][7]); /* creates 7x7 array and populates with EMPTY tokens*/
void drawGameBoard();	/* Graphically represents game to terminal in ASCII characters */

	/*** Gameplay loop***/
int connectFourPresent(); /*searches from right to left, bottom to top for connect 4*/







enum token {EMPTY = -1, YELLOW, RED};



/*** main ***/
int main()
{
	int gameData[7][7];
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
