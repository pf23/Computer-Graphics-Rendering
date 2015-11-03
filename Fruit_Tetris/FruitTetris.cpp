/*
CMPT 361 Assignment 1 - FruitTetris implementation Sample Skeleton Code

- This is ONLY a skeleton code showing:
How to use multiple buffers to store different objects
An efficient scheme to represent the grids and blocks

- Compile and Run:
Type make in terminal, then type ./FruitTetris

This code is extracted from Connor MacLeod's (crmacleo@sfu.ca) assignment submission
by Rui Ma (ruim@sfu.ca) on 2014-03-04. 

Modified in Sep 2014 by Honghua Li (honghual@sfu.ca).
*/

#include "include/Angel.h"
#include <cstdlib>
#include <iostream>

using namespace std;


int delay = 0;
int speed = 50;


// xsize and ysize represent the window size - updated if window is reshaped to prevent stretching of the game
int xsize = 400; 
int ysize = 720;

// current tile
vec2 tile[4]; // An array of 4 2d vectors representing displacement from a 'center' piece of the tile, on the grid
vec2 tilepos = vec2(5, 19); // The position of the current tile using grid coordinates ((0,0) is the bottom left corner)

// current cells position
int cell[4][2];

// An array storing all possible orientations of all possible tiles
// The 'tile' array will always be some element [i][j] of this array (an array of vec2)
vec2 allRotationsShape[12][4] = 
{
	// I shapes, considering the difference of cell colors, there are 4
	{vec2(-2, 0), vec2(-1, 0), vec2(0, 0), vec2(1, 0)},		
	{vec2(0, -2), vec2(0, -1), vec2(0, 0), vec2(0, 1)},		
	{vec2(2, 0),  vec2(1, 0),  vec2(0, 0), vec2(-1,0)},		
	{vec2(0, 2),  vec2(0, 1),  vec2(0, 0), vec2(0,-1)},		

	// S shapes, considering the difference of cell colors, there are 4
	{vec2(-1, -1), vec2(0, -1), vec2(0, 0), vec2(1, 0)},	
	{vec2(1, -1),  vec2(1, 0),  vec2(0, 0), vec2(0, 1)},		
	{vec2(1, 1),   vec2(0, 1),  vec2(0, 0), vec2(-1,0)},		
	{vec2(-1, 1),  vec2(-1, 0), vec2(0, 0), vec2(0,-1)},

	// L shapes
	{vec2(-1, -1), vec2(-1, 0), vec2(0, 0), vec2(1, 0)},	
	{vec2(1, -1),  vec2(0, -1), vec2(0,0),  vec2(0, 1)},  	  
	{vec2(1, 1),   vec2(1,0),   vec2(0, 0), vec2(-1,0)},  	
	{vec2(-1,1),   vec2(0, 1),  vec2(0, 0), vec2(0,-1)},		
};	
int currentShape; // record shape for current tile

// color
vec4 orange = vec4(1.0, 0.5, 0.0, 1.0); 
vec4 purple = vec4(0.5, 0.0, 1.0, 1.0); 
vec4 red    = vec4(1.0, 0.0, 0.0, 1.0); 
vec4 green  = vec4(0.0, 1.0, 0.0, 1.0);
vec4 yellow = vec4(1.0, 1.0, 0.0, 1.0);
vec4 white  = vec4(1.0, 1.0, 1.0, 1.0);
vec4 black  = vec4(0.0, 0.0, 0.0, 1.0);
vec4 colorlist[5] = { purple, red, yellow, green, orange};
vec4 colorInTile[4];
int colorInCell[4]; // 0 purple, 1 red, 2 yellow, 3 green, 4 orange
 
//board[x][y] represents whether the cell (x,y) is occupied
bool board[10][20];

//An array containing the colour of each of the 10*20*2*3 vertices that make up the board
//Initially, all will be set to black. As tiles are placed, sets of 6 vertices (2 triangles; 1 square)
//will be set to the appropriate colour in this array before updating the corresponding VBO
vec4 boardcolours[1200];
int boardcellcol[10][20]; // 0 purple, 1 red, 2 yellow, 3 green, 4 orange

// location of vertex attributes in the shader program
GLuint vPosition;
GLuint vColor;

// locations of uniform variables in shader program
GLuint locxsize;
GLuint locysize;

// VAO and VBO
GLuint vaoIDs[3]; // One VAO for each object: the grid, the board, the current piece
GLuint vboIDs[6]; // Two Vertex Buffer Objects for each VAO (specifying vertex positions and colours, respectively)

// indicate game is on or over
bool gameover = false;

//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------

// When the current tile is moved or rotated (or created), update the VBO containing its vertex position data
void updatetile()
{
	// Bind the VBO containing current tile vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[4]); 

	// For each of the 4 'cells' of the tile,
	for (int i = 0; i < 4; i++) 
	{
		// Calculate the grid coordinates of the cell
		GLfloat x = tilepos.x + tile[i].x; 
		GLfloat y = tilepos.y + tile[i].y;
		cell[i][0] = (int)x;
		cell[i][1] = (int)y;

		// Create the 4 corners of the square - these vertices are using location in pixels
		// These vertices are later converted by the vertex shader
		vec4 p1 = vec4(33.0 + (x * 33.0), 33.0 + (y * 33.0), .4, 1); 
		vec4 p2 = vec4(33.0 + (x * 33.0), 66.0 + (y * 33.0), .4, 1);
		vec4 p3 = vec4(66.0 + (x * 33.0), 33.0 + (y * 33.0), .4, 1);
		vec4 p4 = vec4(66.0 + (x * 33.0), 66.0 + (y * 33.0), .4, 1);

		// Two points are used by two triangles each
		vec4 newpoints[6] = {p1, p2, p3, p2, p3, p4}; 

		// Put new data in the VBO
		glBufferSubData(GL_ARRAY_BUFFER, i*6*sizeof(vec4), 6*sizeof(vec4), newpoints); 
	}

	glBindVertexArray(0);
}

//-------------------------------------------------------------------------------------------------------------------
// check whether tile has exceeded the boundary when created and make adjustment
bool tileAdjust()
{
	for(int i = 0; i < 4; i++)
	{
		// get cell coordinate
		int	x = tilepos.x + tile[i].x;
		int y = tilepos.y + tile[i].y;
		while(x < 0)
		{
			tilepos.x += 1;
			x += 1;
		}
		while(x > 9)
		{
			tilepos.x -= 1;
			x -= 1;
		}
		while(y > 19)
		{
			tilepos.y -= 1;
			y -= 1;
		}
		if(board[x][y])
			return false;
	}
	return true;
}

//-------------------------------------------------------------------------------------------------------------------

// Called at the start of play and every time a tile is placed
void newtile()
{
	// reset speed when a neew tile is generated
	speed = 50;
	GLuint newtileX = rand() % 10; // choose a random starting position
	tilepos = vec2(newtileX , 19); // Put the tile at the top of the board

	// Update the geometry VBO of current tile
	//randomize the tile shape and orientation
	currentShape = rand() % 12;
	for (int i = 0; i < 4; i++)
		tile[i] = allRotationsShape[currentShape][i]; // Get the 4 pieces of the new tile
	if(tileAdjust()==false)
	{
		gameover = true;
		printf("Game Over, press [r] to restart, [q] or [ESC] to quit.\n");
		return;
	}
	tileAdjust();
	updatetile(); 

	// Update the color VBO of current tile
	vec4 newcolours[24];
	vec4 newtileColor;
	// randomize the tile color for four cells
	for (int i = 0; i < 4; i++)
	{
		colorInCell[i] = rand() % 5;
		newtileColor = colorlist[colorInCell[i]];
		colorInTile[i] = newtileColor;
		for(int j = 0; j < 6; j++)
			newcolours[i*6 + j] = newtileColor;
	}
	
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]); // Bind the VBO containing current tile vertex colours
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(newcolours), newcolours); // Put the colour data in the VBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}

//-------------------------------------------------------------------------------------------------------------------

void initGrid()
{
	// ***Generate geometry data
	vec4 gridpoints[64]; // Array containing the 64 points of the 32 total lines to be later put in the VBO
	vec4 gridcolours[64]; // One colour per vertex
	// Vertical lines 
	for (int i = 0; i < 11; i++){
		gridpoints[2*i] = vec4((33.0 + (33.0 * i)), 33.0, 0, 1);
		gridpoints[2*i + 1] = vec4((33.0 + (33.0 * i)), 693.0, 0, 1);
		
	}
	// Horizontal lines
	for (int i = 0; i < 21; i++){
		gridpoints[22 + 2*i] = vec4(33.0, (33.0 + (33.0 * i)), 0, 1);
		gridpoints[22 + 2*i + 1] = vec4(363.0, (33.0 + (33.0 * i)), 0, 1);
	}
	// Make all grid lines white
	for (int i = 0; i < 64; i++)
		gridcolours[i] = white;


	// *** set up buffer objects
	// Set up first VAO (representing grid lines)
	glBindVertexArray(vaoIDs[0]); // Bind the first VAO
	glGenBuffers(2, vboIDs); // Create two Vertex Buffer Objects for this VAO (positions, colours)

	// Grid vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[0]); // Bind the first grid VBO (vertex positions)
	glBufferData(GL_ARRAY_BUFFER, 64*sizeof(vec4), gridpoints, GL_STATIC_DRAW); // Put the grid points in the VBO
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0); 
	glEnableVertexAttribArray(vPosition); // Enable the attribute
	
	// Grid vertex colours
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[1]); // Bind the second grid VBO (vertex colours)
	glBufferData(GL_ARRAY_BUFFER, 64*sizeof(vec4), gridcolours, GL_STATIC_DRAW); // Put the grid colours in the VBO
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor); // Enable the attribute
}


void initBoard()
{
	gameover = false;
	// *** Generate the geometric data
	vec4 boardpoints[1200];
	for (int i = 0; i < 1200; i++)
		boardcolours[i] = black; // Let the empty cells on the board be black
	// Each cell is a square (2 triangles with 6 vertices)
	for (int i = 0; i < 20; i++){
		for (int j = 0; j < 10; j++)
		{		
			vec4 p1 = vec4(33.0 + (j * 33.0), 33.0 + (i * 33.0), .5, 1);
			vec4 p2 = vec4(33.0 + (j * 33.0), 66.0 + (i * 33.0), .5, 1);
			vec4 p3 = vec4(66.0 + (j * 33.0), 33.0 + (i * 33.0), .5, 1);
			vec4 p4 = vec4(66.0 + (j * 33.0), 66.0 + (i * 33.0), .5, 1);
			
			// Two points are reused
			boardpoints[6*(10*i + j)    ] = p1;
			boardpoints[6*(10*i + j) + 1] = p2;
			boardpoints[6*(10*i + j) + 2] = p3;
			boardpoints[6*(10*i + j) + 3] = p2;
			boardpoints[6*(10*i + j) + 4] = p3;
			boardpoints[6*(10*i + j) + 5] = p4;
		}
	}

	// Initially no cell is occupied
	for (int i = 0; i < 10; i++)
		for (int j = 0; j < 20; j++)
		{
			board[i][j] = false; 
			boardcellcol[i][j] = -1;
		}

	// initial cells
	for(int i = 0;i < 4; i++)
	{
		cell[i][0] = cell[i][1] = 0;
		colorInTile[i] = black;
		colorInCell[i] = -1;
	}

	// *** set up buffer objects
	glBindVertexArray(vaoIDs[1]);
	glGenBuffers(2, &vboIDs[2]);

	// Grid cell vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[2]);
	glBufferData(GL_ARRAY_BUFFER, 1200*sizeof(vec4), boardpoints, GL_STATIC_DRAW);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	// Grid cell vertex colours
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);
	glBufferData(GL_ARRAY_BUFFER, 1200*sizeof(vec4), boardcolours, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);
}

// No geometry for current tile initially
void initCurrentTile()
{
	glBindVertexArray(vaoIDs[2]);
	glGenBuffers(2, &vboIDs[4]);

	// Current tile vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[4]);
	glBufferData(GL_ARRAY_BUFFER, 24*sizeof(vec4), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	// Current tile vertex colours
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]);
	glBufferData(GL_ARRAY_BUFFER, 24*sizeof(vec4), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);
}

void init()
{
	// Load shaders and use the shader program
	GLuint program = InitShader("vshader.glsl", "fshader.glsl");
	glUseProgram(program);

	// Get the location of the attributes (for glVertexAttribPointer() calls)
	vPosition = glGetAttribLocation(program, "vPosition");
	vColor = glGetAttribLocation(program, "vColor");

	// Create 3 Vertex Array Objects, each representing one 'object'. Store the names in array vaoIDs
	glGenVertexArrays(3, &vaoIDs[0]);

	// Initialize the grid, the board, and the current tile
	initGrid();
	initBoard();
	initCurrentTile();

	// The location of the uniform variables in the shader program
	locxsize = glGetUniformLocation(program, "xsize"); 
	locysize = glGetUniformLocation(program, "ysize");

	// Game initialization
	newtile(); // create new next tile

	// set to default
	glBindVertexArray(0);
	glClearColor(0, 0, 0, 0);
}

//-------------------------------------------------------------------------------------------------------------------

// the space key will shullfle the fruits, which moves the fruits within the tile on the following orders:
// a->b, b->c, c->d, d->a
void shuffle()
{
	// shuffle the colors
	vec4  colorOfa = colorInTile[0];
	colorInTile[0] = colorInTile[3];
	colorInTile[3] = colorInTile[2];
	colorInTile[2] = colorInTile[1];
	colorInTile[1] = colorOfa;

	int colorOfa_num = colorInCell[0];
	colorInCell[0] = colorInCell[3];
	colorInCell[3] = colorInCell[2];
	colorInCell[2] = colorInCell[1];
	colorInCell[1] = colorOfa_num;

	// Update the color VBO of current tile
	vec4 newcolours[24];
	//assign every 6 verticies with the new color
	for(int i = 0; i < 4; i++)
		for(int j = 0; j < 6; j++)
			newcolours[(i*6) + j] = colorInTile[i];
						
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]); // Bind the VBO containing current tile vertex colours
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(newcolours), newcolours); // Put the colour data in the VBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}

//-------------------------------------------------------------------------------------------------------------------

// Rotates the current tile, if there is room
void rotate()
{      
	currentShape += 1;
	if(currentShape % 4 == 0)
		currentShape -= 4;
	// update tiles
	for(int i = 0; i < 4; i++){
		tile[i] = allRotationsShape[currentShape][i];
	}
	tileAdjust();
	updatetile();	
	glutPostRedisplay();
}

//-------------------------------------------------------------------------------------------------------------------
// given a cell, Check if there is three same fruits in a row or column around it
bool checkfruit()
//{
//	return true;
//}
{
	bool returnval = false;
	// search from bot left to top right
	for (int row = 0; row < 20; ++row)
	{
		for(int col = 0; col < 10; ++col)
		{
			int flagrow = 0; // indicate num of same color in a row
			int flagcol = 0; // indicate num of same color in a row

			// get cell color
			int cellcolor = boardcellcol[col][row];
			if(cellcolor == -1)
				continue;


			// check whether there are same fruits in a column
			for(int k = 1; k + row < 20; k++)
			{
				if(boardcellcol[col][k+row] == cellcolor)
					flagcol++;
				else
					break;
			}
			// check whether there are same fruits in a row
			for(int k = 1; k + col < 10;k++)
			{
				if(boardcellcol[k+col][row] == cellcolor)
					flagrow++;
				else
					break;
			}

			// clear if needed
			if(flagcol>=2)
			{
				printf("fruit\n");
				returnval = true;

				// shift down the tiles above
				glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);

				// everything above the cleared row shift down one row
				for(int i = row; i < 20; i++)
				{
					// update the boardcolours for the cell
					for(int j = col*6; j < (col + 1) * 6; j++)
						//if it is or exceeds the top row, clear it
						if((i + flagcol) >= 19)
							boardcolours[(i * 60) + j] = black;
						else
							boardcolours[(i * 60) + j] = boardcolours[(i + flagcol + 1) * 60 + j];

					// update the board occupation status and board cell color matrix
					//if it is the top row, clear it
					if((i + flagcol) >= 19)
					{
						board[col][i] = false;
						boardcellcol[col][i] = -1;
					}
					else
					{
						board[col][i] = board[col][i + flagcol + 1];
						boardcellcol[col][i] = boardcellcol[col][i + flagcol + 1];
					}
					
				}
				glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(boardcolours), boardcolours);
				glBindVertexArray(0);
			}

			if(flagrow>=2)
			{
				printf("fruit\n");
				returnval = true;

				// shift down the tiles above
				glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);

				int start_col = col;
				// if the cell is already removed in above operation
				if(flagcol>=2)
				{
					 start_col++;
					 flagrow--;
				}

				// everything above the cleared row shift down one row
				for(int i = row; i < 20; i++)
				{
					// update the boardcolours
					for(int j = start_col*6; j < (start_col + flagrow +1)*6; j++)
						//if it is the top row, clear it
						if(i == 19)
							boardcolours[1140 + j] = black;
						else
							boardcolours[(i * 60) + j] = boardcolours[(i * 60) + j + 60];
					// update the board occupation status and board cell color matrix

					for(int j = start_col; j < (start_col + flagrow + 1); j++)
					{
						// if it is the top row, clear it
						if(i == 19)
						{
							board[j][19] = false;
							boardcellcol[j][19] = -1;
						}
						else
						{
							board[j][i] = board[j][i+1];
							boardcellcol[j][i] = boardcellcol[j][i+1];
						}
					}
				}
				glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(boardcolours), boardcolours);
				glBindVertexArray(0);
				
			}// end of removal
		}// end of cell[col][row] judgment
	}
	return returnval;

}

//-------------------------------------------------------------------------------------------------------------------

// Checks if the specified row (0 is the bottom 19 the top) is full
// If every cell in the row is occupied, it will clear that cell and everything above it will shift down one row
bool checkfullrow(int row)
{
	for(int k = 0;k < 10;k++)
		if(!board[k][row])
			return false;

	// if there is a full row
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);

	// everything above the cleared row shift down one row
	for(int i = row; i < 20; i++)
	{
		// update the boardcolours
		for(int j = 0; j < 60; j++)
			//if it is the top row, clear it
			if(i == 19)
				boardcolours[1140 + j] = black;
			else
				boardcolours[(i * 60) + j] = boardcolours[(i * 60) + j + 60];
		// update the board occupation status and board cell color matrix
		for(int j = 0; j < 10; j++)
		{
			//if it is the top row, clear it
			if(i == 19)
			{
				board[j][19] = false;
				boardcellcol[j][19] = -1;
			}
			else
			{
				board[j][i] = board[j][i+1];
				boardcellcol[j][i] = boardcellcol[j][i+1];
			}
		}
	}
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(boardcolours), boardcolours);
	glBindVertexArray(0);
	
	return true;
}

//-------------------------------------------------------------------------------------------------------------------

// Places the current tile - update the board vertex colour VBO and the array maintaining occupied cells
void settile()
{
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);

	for(int i = 0; i < 4; i++)
	{
		// set the cells as occupied
		board[cell[i][0]][cell[i][1]] = true;
		boardcellcol[cell[i][0]][cell[i][1]] = colorInCell[i];			

		// set boardcolor buffer
		for(int j = (cell[i][0]*6 + cell[i][1]*60);    	j < (cell[i][0]*6 + cell[i][1]*60 + 6);      j++)
			boardcolours[j] = colorInTile[i];
	}
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(boardcolours), boardcolours);
	glBindVertexArray(0);
	for(int i = 0; i < 4; i++){
			checkfullrow(cell[i][1]);
	}
	for(;;)
	{
		if(checkfruit() == true)
			continue;
		else
			break;
	}
	newtile();
}

//-------------------------------------------------------------------------------------------------------------------

// Given (x,y), tries to move the tile x squares to the right and y squares down
// Returns true if the tile was successfully moved, or false if there was some issue
bool movetile(vec2 direction)
{
	vec2  temp = tilepos + direction;
	int temp_cell[4][2];
	for(int i = 0;i < 4;i++)
	{
		temp_cell[i][0] = (int)(temp.x + tile[i].x);
		temp_cell[i][1] = (int)(temp.y + tile[i].y);
		if(temp_cell[i][0] < 0 || temp_cell[i][0] > 9 || temp_cell[i][1] < 0 || temp_cell[i][1] > 19)
			return false;
		if(board[temp_cell[i][0]][temp_cell[i][1]])
			return false;
	}
	tilepos = temp;
	updatetile();
	glutPostRedisplay();
	return true;
}
//-------------------------------------------------------------------------------------------------------------------

// Starts the game over - empties the board, creates new tiles, resets line counters
void restart()
{
	gameover = false;
	initBoard();
	newtile();

}
//-------------------------------------------------------------------------------------------------------------------

// Draws the game
void display()
{

	glClear(GL_COLOR_BUFFER_BIT);

	glUniform1i(locxsize, xsize); // x and y sizes are passed to the shader program to maintain shape of the vertices on screen
	glUniform1i(locysize, ysize);

	glBindVertexArray(vaoIDs[1]); // Bind the VAO representing the grid cells (to be drawn first)
	glDrawArrays(GL_TRIANGLES, 0, 1200); // Draw the board (10*20*2 = 400 triangles)

	glBindVertexArray(vaoIDs[2]); // Bind the VAO representing the current tile (to be drawn on top of the board)
	glDrawArrays(GL_TRIANGLES, 0, 24); // Draw the current tile (8 triangles)

	glBindVertexArray(vaoIDs[0]); // Bind the VAO representing the grid lines (to be drawn on top of everything else)
	glDrawArrays(GL_LINES, 0, 64); // Draw the grid lines (21+11 = 32 lines)


	glutSwapBuffers();
}

//-------------------------------------------------------------------------------------------------------------------

// Reshape callback will simply change xsize and ysize variables, which are passed to the vertex shader
// to keep the game the same from stretching if the window is stretched
void reshape(GLsizei w, GLsizei h)
{
	xsize = w;
	ysize = h;
	glViewport(0, 0, w, h);
}

//-------------------------------------------------------------------------------------------------------------------

// Handle arrow key keypresses
void special(int key, int x, int y)
{
	vec2 dir;
	switch (key)
	{
		case GLUT_KEY_LEFT:
			dir = vec2(-1, 0);
			movetile(dir);
			break;
		case GLUT_KEY_RIGHT:
			dir = vec2(1, 0);
			movetile(dir);
			break;
		case GLUT_KEY_UP:
			rotate();
			break;
		case GLUT_KEY_DOWN:
			// accelerate falling speed
			speed -= 20;
			break;
	}
	glutPostRedisplay();

}

//-------------------------------------------------------------------------------------------------------------------

// Handles standard keypresses
void keyboard(unsigned char key, int x, int y)
{
	switch(key) 
	{
		case 033: // Both escape key and 'q' cause the game to exit
		    exit(EXIT_SUCCESS);
		    break;
		case 'q':
			exit (EXIT_SUCCESS);
			break;
		case 'r': // 'r' key restarts the game
			restart();
			break;
		case ' ': // ' ' to shuffle the tile
			shuffle();
			break;
	}
	glutPostRedisplay();
}

//-------------------------------------------------------------------------------------------------------------------
void down()
{
	if(movetile(vec2(0,-1)) ==  false)
	{
		settile();
	}
}
//-------------------------------------------------------------------------------------------------------------------

void idle(void)
{
	if(!gameover){		
		updatetile();
		delay ++;
		if(delay >= speed){
			down();
			delay = 0;
		}
	}
	glutPostRedisplay();
}

//-------------------------------------------------------------------------------------------------------------------

int main(int argc, char **argv)
{

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(xsize, ysize);
	glutInitWindowPosition(680, 178); // Center the game window (well, on a 1920x1080 display)
	glutCreateWindow("Fruit Tetris");
	glewInit();
	init();

	// Callback functions
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutSpecialFunc(special);
	glutKeyboardFunc(keyboard);
	glutIdleFunc(idle);

	glutMainLoop(); // Start main loop
	return 0;
}
