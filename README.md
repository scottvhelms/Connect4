# Connect4
Connect four game

Connect Four Design Document

Features
This program will be a graphical representation of the Connect 4 game invented by Howard Wexler, and will have the following features:
Will be a 7x7 slotted board in which players select a column in which to drop a token down the column until it hits the bottom or stacks on a previously “dropped” token in the attempt to place four tokens in a row vertically, diagonally, or horizontally. Game ends when such a row has been encountered.
Will be a two-player game, with the players alternating and taking turns. 
Will be graphically represented in the terminal using ASCII Characters.
Will show real time status of the game as it is being played with the colors Red and Yellow representing the different players pieces.
Will use the arrow keys for the players to move a piece about the board and the enter key to “drop” a piece in the correct location.
Will contain a status bar with directions to complete the turn. 
i.e.
USE ARROWS TO MOVE (  ->   or   <-  ) AND HIT ENTER TO DROP
Will contain a color coded status bar to represent which player's turn is in progress that is.
i.e.
PLAYER 1'S TURN
Will identify when 4 of one player’s pieces are in a row vertically, diagonally, or horizontally.
Will apply a status bar upon player winning, with the option to start a new game
i.e
PLAYER X HAS WON!!!!    PRESS ENTER TO START A NEW GAME
Will contain a way to quit the game at any time, with a warning prior to exiting.
Will utilize Ctrl-q to initiate quit and will need to be pressed three times.
Game may be updated in the future to implement other features such as (but not limited to):
Network compatibility
No human player (single player)


Architecture Overview
This program will be written in C. 
Gameplay and interaction with the game will be through the terminal.
The program will be entered without arguments.
The primary structure that will house the data of the status of the game will be a 2D array.
Indexes will be as follows: Array[Row][Column]
Each element of the array will represent a slot on the board.
yellow, red, and empty will be represented by enum.


Arrow keys will move the cursor over the 7 columns represented by the token to drop (a properly colored “O”). If the arrow key is pressed in the direction of an edge then the key will wrap to the other side.

Move right			Move left		Move left, Wrap
O		        	->    O		      	<-	O	           	<-	          O
_ _ _ _ _ _ _		 	_ _ _ _ _ _ _			_ _ _ _ _ _ _		_ _ _ _ _ _ _
_ _ _ _ _ _ _			_ _ _ _ _ _ _			_ _ _ _ _ _ _		_ _ _ _ _ _ _
_ _ _ _ _ _ _			_ _ _ _ _ _ _			_ _ _ _ _ _ _		_ _ _ _ _ _ _
_ _ _ _ _ _ _			_ _ _ _ _ _ _			_ _ _ _ _ _ _		_ _ _ _ _ _ _
_ _ _ _ _ _ _			_ _ _ _ _ _ _			_ _ _ _ _ _ _		_ _ _ _ _ _ _
_ _ _ _ _ _ _			_ _ _ _ _ _ _			_ _ _ _ _ _ _		_ _ _ _ _ _ _
_ _ _ _ _ _ _			_ _ _ _ _ _ _			_ _ _ _ _ _ _		_ _ _ _ _ _ _




Implementation
Upon the game program being started:
The mesh structure will be allocated and built to be a 7x7 in size
All elements of the array are initialized with a “empty” token
Mesh will be constructed as nodes are created/initialize and assigned position
The board graphic will be display.
The Gameplay loop will start:
Red player will always start first
Movement across the columns is looped until the player drops by pressing enter
Upon dropping, token is placed at the bottom row (array[6][x])
Graphic is updated to represent new token
connectFourPresent function is run to verify that four tokens of the same color are not in a row
starting at array[6][6] and moving backwards through the row then moving upwards to the next column
Node will be checked if it has a yellow or red token in it
If empty, will move on to next.
If token is present, will check if same color token is present to the left, up-left, up, upright 
If true will continue checking in that direction counting all the tokens encountered.
If false, counter set to 0 and checks other directions not checked or continues next token.
If counter reaches four, the game loop is exited and the game ends.
Upon game ending status bars change and a new game option is presented, reinitializing the mesh and entering the game loop again.
Game can be exited at any time using Ctrl-q, status bar will inform user that Ctrl-q must be pressed 3 more times to exit.



Timeline
Full completion: Two weeks
Mesh structure, node initialization 			            	1-2 days
Terminal and keypress			            		            	1-3 days
Graphics						                                  	1-2 days	
Linking of structure to Graphic			    	            	2-3 days
connectFourPresent function (and related sub functions)	1-3 days
Gameplay loop						                              	1 day


