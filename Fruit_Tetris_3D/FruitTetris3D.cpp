#include "include/Angel.h"
#include <math.h>

typedef Angel::vec4  color4;
typedef Angel::vec4  point4;
using namespace std;

// ----------------------------------------------------------------------------
// parameters of the robot arm
const int NumVertices = 36; //(6 faces)(2 triangles/face)(3 vertices/triangle)

point4 points[NumVertices];
color4 colors[NumVertices];
point4 bpoints[NumVertices];
color4 bcolors[NumVertices];

point4 vertices[8] = {
    point4( -0.5, -0.5,  0.5, 1.0 ),
    point4( -0.5,  0.5,  0.5, 1.0 ),
    point4(  0.5,  0.5,  0.5, 1.0 ),
    point4(  0.5, -0.5,  0.5, 1.0 ),
    point4( -0.5, -0.5, -0.5, 1.0 ),
    point4( -0.5,  0.5, -0.5, 1.0 ),
    point4(  0.5,  0.5, -0.5, 1.0 ),
    point4(  0.5, -0.5, -0.5, 1.0 )
};

// RGBA olors
color4 vertex_colors[8] = {
    color4( 0.0, 0.0, 0.0, 1.0 ),  // black
    color4( 1.0, 0.0, 0.0, 1.0 ),  // red
    color4( 1.0, 1.0, 0.0, 1.0 ),  // yellow
    color4( 0.0, 1.0, 0.0, 1.0 ),  // green
    color4( 0.0, 0.0, 1.0, 1.0 ),  // blue
    color4( 1.0, 0.0, 1.0, 1.0 ),  // magenta
    color4( 1.0, 1.0, 1.0, 1.0 ),  // white
    color4( 0.0, 1.0, 1.0, 1.0 )   // cyan
};

GLuint vPosition;
GLuint vColor;

// Viewing transformation parameters
GLfloat pi = 3.14159;
GLfloat radius = 120.0;
GLfloat alpha = pi/6;

const GLfloat  unit = 10 * pi / 180;

GLuint  model_view;  // model-view matrix uniform shader variable location

// Projection transformation parameters
GLfloat  fovy = 45.0;  // Field-of-view in Y direction angle (in degrees)
GLfloat  aspect;       // Viewport aspect ratio
GLfloat  zNear = 1, zFar = 200.0;

GLuint  projection; // projection matrix uniform shader variable location

// angle of the robot arm
GLfloat theta = -20;
GLfloat phi = 100;

// VAO and VBO
GLuint vao,vao2;
GLuint vaoIDs[3]; // One VAO for each object: the grid, the board, the current piece
GLuint vboIDs[6]; // Two Vertex Buffer Objects for each VAO (specifying vertex positions and colours, respectively)

// Parameters controlling the size of the Robot's arm
const GLfloat BASE_HEIGHT      = 2.0;
const GLfloat BASE_WIDTH       = 5.0;
const GLfloat LOWER_ARM_HEIGHT = 5.0;
const GLfloat LOWER_ARM_WIDTH  = 0.5;
const GLfloat UPPER_ARM_HEIGHT = 5.0;
const GLfloat UPPER_ARM_WIDTH  = 0.5;

enum { Base = 0, LowerArm = 1, UpperArm = 2, NumAngles = 3 };
int      Axis = Base;
GLfloat  Theta[NumAngles] = { 0.0 };

mat4  model_v;

//----------------------------------------------------------------------------
// parameters of the tile
// current tile
vec2 tile[4]; // An array of 4 2d vectors representing displacement from a 'center' piece of the tile, on the grid
vec2 tilepos = vec2(5,19); // The position of the current tile using grid coordinates ((0,0) is the bottom left corner)
// current cells position
int cell[4][2] = { 0,0,0,0,0,0,0,0} ;

// all shapes
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
int currentShape = 0; // record shape for current tile

// colors
vec4 orange = vec4(1.0, 0.5, 0.0, 1.0); 
vec4 purple = vec4(0.5, 0.0, 1.0, 1.0); 
vec4 red    = vec4(1.0, 0.0, 0.0, 1.0); 
vec4 green  = vec4(0.0, 0.5, 0.0, 1.0);
vec4 yellow = vec4(1.0, 1.0, 0.0, 1.0);
vec4 white  = vec4(1.0, 1.0, 1.0, 1.0);
vec4 black  = vec4(0.0, 0.0, 0.0, 1.0);
vec4 blue   = vec4(0.0, 0.0, 1.0, 1.0);
vec4 grey   = vec4(0.5, 0.5, 0.5, 1.0);
vec4 brown  = vec4(0.627, 0.322, 0.176, 1.0);
vec4 black2 = vec4(0.0, 0.0, 0.0, 0.5);
vec4 colorlist[5] = { purple, red, yellow, green, orange};// 0 purple, 1 red, 2 yellow, 3 green, 4 orange
vec4 colorInTile[4];
vec4 colorInTileRestore[4];

//board[x][y] represents whether the cell (x,y) is occupied
bool board[10][20];
vec4 boardcolours[7200];
vec4 boardpoints[7200];

bool release = false;

// --------------------------------------------------------------------------
int Index = 0;

void quad( int a, int b, int c, int d ,int xa)
{
    colors[Index] = vertex_colors[xa]; points[Index] = vertices[a]; Index++;
    colors[Index] = vertex_colors[xa]; points[Index] = vertices[b]; Index++;
    colors[Index] = vertex_colors[xa]; points[Index] = vertices[c]; Index++;
    colors[Index] = vertex_colors[xa]; points[Index] = vertices[a]; Index++;
    colors[Index] = vertex_colors[xa]; points[Index] = vertices[c]; Index++;
    colors[Index] = vertex_colors[xa]; points[Index] = vertices[d]; Index++;
}

//----------------------------------------------------------------------------
// generate 12 triangles: 36 vertices and 36 colors
void colorcube()
{
    quad( 1, 0, 3, 2 , 1);
    quad( 2, 3, 7, 6 , 4);
    quad( 3, 0, 4, 7 , 1);
    quad( 6, 5, 1, 2 , 4);
    quad( 4, 5, 6, 7 , 1);
    quad( 5, 4, 0, 1 , 4);
}

// if there is collision, change tile into grey
bool tileAdjust()
{

    bool flag = true;
    vec4 newcolours[144];
    for(int i = 0; i < 4; i++)
    {
        // get cell coordinate
        int x = tilepos.x + tile[i].x;
        int y = tilepos.y + tile[i].y;
        cout << "x: " << x << " y:" << y <<endl;
        if(board[x][y] || x < 0 || x > 9 || y < 0 || y > 19) 
        {
            flag = false;
            break;
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);
    for(int n = 0; n < 4; n++)
    {
        // set boardcolor buffer
        for(int k = (cell[n][0]*6*6 + cell[n][1]*60*6);     k < (cell[n][0]*6*6 + cell[n][1]*60*6 + 36);      k++)
        {
            if(flag == false)
            {
                colorInTile[n]= grey;
                for(int j = 0; j < 36; j++)
                    newcolours[n*36 + j] = grey;

            }
            else
            {
                colorInTile[n] = colorInTileRestore[n];
                for(int j = 0; j < 36; j++)
                    newcolours[n*36 + j] = colorInTileRestore[n];
            }
        }

    }

    glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]); // Bind the VBO containing current tile vertex colours
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(newcolours), newcolours); // Put the colour data in the VBO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return flag;
}

// When the current tile is moved or rotated (or created), update the VBO containing its vertex position data
void updatetile()
{
    // Bind the VBO containing current tile vertex positions
    glBindBuffer(GL_ARRAY_BUFFER, vboIDs[4]);
    GLfloat xx,yy;
    GLuint xa,ya;
    if(release == false)
    {        
        // calculate (x, y) from theta and phi
        xx = -25 + 30*sin(theta*pi/180) + 40*sin((theta + phi)*pi/180);
        yy = 1 + 30*cos(theta*pi/180) + 40*cos((theta + phi)*pi/180);
        xa = xx/3 + 5;
        ya = yy/3;
        cout << "now position1 (" << tilepos.x << "," << tilepos.y << ")" << endl;
        tilepos = vec2(xa,ya);
    }

    
    // For each of the 4 'cells' of the tile,
    for (int i = 0; i < 4; i++) 
    {
        // Calculate the grid coordinates of the cell
        GLuint x,y;
        x = tilepos.x + tile[i].x; 
        y = tilepos.y + tile[i].y;
        cell[i][0] = int(x);
        cell[i][1] = int(y);
        //cout << "now position2 (" << x << "," << y << ")" << endl;

        // Create the 4 corners of the square - these vertices are using location in pixels
        // These vertices are later converted by the vertex shader
        vec4 p1 = vec4( 3*x -15, 3*y,       -1.5, 1); 
        vec4 p2 = vec4( 3*x -15, 3*y + 3,   -1.5, 1);
        vec4 p3 = vec4( 3*x -12, 3*y,       -1.5, 1);
        vec4 p4 = vec4( 3*x -12, 3*y + 3,   -1.5, 1);
        vec4 p5 = vec4( 3*x -15, 3*y,       1.5, 1); 
        vec4 p6 = vec4( 3*x -15, 3*y + 3,   1.5, 1);
        vec4 p7 = vec4( 3*x -12, 3*y,       1.5, 1);
        vec4 p8 = vec4( 3*x -12, 3*y + 3,   1.5, 1);

        // 36 vertices for one cell
        vec4 newpoints[36] = {
            p1, p2, p3, p2, p3, p4, 
            p1, p2, p5, p2, p5, p6,  
            p5, p6, p7, p6, p7, p8, 
            p3, p4, p7, p4, p7, p8, 
            p2, p4, p6, p4, p6, p8,
            p1, p3, p5, p3, p5, p7
        }; 

        // Put new data in the VBO
        glBufferSubData(GL_ARRAY_BUFFER, i*36*sizeof(vec4), 36*sizeof(vec4), newpoints); 
    }

    glBindVertexArray(0);
    tileAdjust();
}

//-------------------------------------------------------------------------------------------------------------------

// Called at the start of play and every time a tile is placed
void newtile()
{
    release = false;

    GLfloat xx,yy;
    int xa,ya;
    xx = -25 + 30*sin(theta*pi/180) + 40*sin((theta + phi)*pi/180);
    yy = 1 + 30*cos(theta*pi/180) + 40*cos((theta + phi)*pi/180);
    xa = xx/3 + 5;
    ya = yy/3;
    tilepos = vec2(xa,ya);
    cout << "intial position (" << tilepos.x << "," << tilepos.y << ")" << endl;

    // Update the geometry VBO of current tile
    //randomize the tile shape and orientation
    currentShape = rand() % 12;
    for (int i = 0; i < 4; i++)
        tile[i] = allRotationsShape[currentShape][i]; // Get the 4 pieces of the new tile
    // Update the color VBO of current tile
    vec4 newcolours[144];
    vec4 newtileColor;
    // randomize the tile color for four cells
    for (int i = 0; i < 4; i++)
    {
        int new_color_index = rand() % 5;
        newtileColor = colorlist[new_color_index];
        colorInTile[i] = newtileColor;
        colorInTileRestore[i] = newtileColor;
        for(int j = 0; j < 36; j++)
            newcolours[i*36 + j] = newtileColor;
    }
    updatetile(); 
    //cout << "checkpoint \n";    
    
    glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]); // Bind the VBO containing current tile vertex colours
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(newcolours), newcolours); // Put the colour data in the VBO
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);
}

//----------------------------------------------------------------------------
// No geometry for current tile initially
void initCurrentTile()
{
    glBindVertexArray(vaoIDs[2]);
    glGenBuffers(2, &vboIDs[4]);

    // Current tile vertex positions
    glBindBuffer(GL_ARRAY_BUFFER, vboIDs[4]);
    glBufferData(GL_ARRAY_BUFFER, 144*sizeof(vec4), NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(vPosition);

    // Current tile vertex colours
    glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]);
    glBufferData(GL_ARRAY_BUFFER, 144*sizeof(vec4), NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(vColor);
}


void initGrid()
{
    // ***Generate geometry data
    vec4 gridpoints[128]; // Array containing the 64 points of the 32 total lines to be later put in the VBO
    vec4 gridcolours[128]; // One colour per vertex
    // Vertical lines 
    for (int i = 0; i < 11; i++){
        gridpoints[2*i] =       vec4( 3*i - 15, 0,  -1.5,   1);
        gridpoints[2*i + 1] =   vec4( 3*i - 15, 60, -1.5,   1);
        gridpoints[2*i + 64] =  vec4( 3*i - 15, 0,  1.5,    1);
        gridpoints[2*i + 65] =  vec4( 3*i - 15, 60, 1.5,    1);
    }
    // Horizontal lines
    for (int i = 0; i < 21; i++){
        gridpoints[22 + 2*i] =      vec4( -15,  3*i , -1.5, 1);
        gridpoints[22 + 2*i + 1] =  vec4( 15,   3*i , -1.5, 1);
        gridpoints[22 + 2*i + 64] = vec4( -15,  3*i , 1.5,  1);
        gridpoints[22 + 2*i + 65] = vec4( 15,   3*i , 1.5,  1);
    }
    // Make all grid lines white
    for (int i = 0; i < 128; i++)
        gridcolours[i] = brown;

    
    // *** set up buffer objects
    // Set up first VAO (representing grid lines)
    glBindVertexArray(vaoIDs[0]); // Bind the first VAO
    glGenBuffers(2, vboIDs); // Create two Vertex Buffer Objects for this VAO (positions, colours)

    // Grid vertex positions
    glBindBuffer(GL_ARRAY_BUFFER, vboIDs[0]); // Bind the first grid VBO (vertex positions)
    glBufferData(GL_ARRAY_BUFFER, 128*sizeof(vec4), gridpoints, GL_STATIC_DRAW); // Put the grid points in the VBO
    glEnableVertexAttribArray(vPosition); // Enable the attribute
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0); 

    
    // Grid vertex colours
    glBindBuffer(GL_ARRAY_BUFFER, vboIDs[1]); // Bind the second grid VBO (vertex colours)
    glBufferData(GL_ARRAY_BUFFER, 128*sizeof(vec4), gridcolours, GL_STATIC_DRAW); // Put the grid colours in the VBO
    glEnableVertexAttribArray(vColor); // Enable the attribute
    glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
}

void initBoard()
{
    //initial board
    for(int i = 0; i < 10;i++)
        for(int j = 0; j < 20;j++)
        {
            board[i][j] = false;
        }
            
    // *** Generate the geometric data
    for (int i = 0; i < 7200; i++)
    {
        boardcolours[i] = black;
    }
    
    // Each cell is a square (12 triangles with 36 vertices)
    for (int i = 0; i < 20; i++){
        for (int j = 0; j < 10; j++)
        {       
            vec4 p1 = vec4( 3*j - 15, 3*i,      -1.4, 1);
            vec4 p2 = vec4( 3*j - 15, 3*i + 3,  -1.4, 1);
            vec4 p3 = vec4( 3*j - 12, 3*i,      -1.4, 1);
            vec4 p4 = vec4( 3*j - 12, 3*i + 3,  -1.4, 1);
            vec4 p5 = vec4( 3*j - 15, 3*i,      1.4, 1);
            vec4 p6 = vec4( 3*j - 15, 3*i + 3,  1.4, 1);
            vec4 p7 = vec4( 3*j - 12, 3*i,      1.4, 1);
            vec4 p8 = vec4( 3*j - 12, 3*i + 3,  1.4, 1);
            
            // 36 vertices for one cube
            boardpoints[36*(10*i + j)    ] = p1;
            boardpoints[36*(10*i + j)  +1] = p2;
            boardpoints[36*(10*i + j)  +2] = p3;
            boardpoints[36*(10*i + j)  +3] = p2;
            boardpoints[36*(10*i + j)  +4] = p3;
            boardpoints[36*(10*i + j)  +5] = p4;

            boardpoints[36*(10*i + j)  +6] = p1;
            boardpoints[36*(10*i + j)  +7] = p2;
            boardpoints[36*(10*i + j)  +8] = p5;
            boardpoints[36*(10*i + j)  +9] = p2;
            boardpoints[36*(10*i + j) +10] = p5;
            boardpoints[36*(10*i + j) +11] = p6;

            boardpoints[36*(10*i + j)  +12] = p5;
            boardpoints[36*(10*i + j)  +13] = p6;
            boardpoints[36*(10*i + j)  +14] = p7;
            boardpoints[36*(10*i + j)  +15] = p6;
            boardpoints[36*(10*i + j)  +16] = p7;
            boardpoints[36*(10*i + j)  +17] = p8;

            boardpoints[36*(10*i + j)  +18] = p3;
            boardpoints[36*(10*i + j)  +19] = p4;
            boardpoints[36*(10*i + j)  +20] = p7;
            boardpoints[36*(10*i + j)  +21] = p4;
            boardpoints[36*(10*i + j)  +22] = p7;
            boardpoints[36*(10*i + j)  +23] = p8;

            boardpoints[36*(10*i + j)  +24] = p2;
            boardpoints[36*(10*i + j)  +25] = p4;
            boardpoints[36*(10*i + j)  +26] = p6;
            boardpoints[36*(10*i + j)  +27] = p4;
            boardpoints[36*(10*i + j)  +28] = p6;
            boardpoints[36*(10*i + j)  +29] = p8;

            boardpoints[36*(10*i + j)  +30] = p1;
            boardpoints[36*(10*i + j)  +31] = p3;
            boardpoints[36*(10*i + j)  +32] = p5;
            boardpoints[36*(10*i + j)  +33] = p3;
            boardpoints[36*(10*i + j)  +34] = p5;
            boardpoints[36*(10*i + j)  +35] = p7;
        }
    }
    

    // *** set up buffer objects
    glBindVertexArray(vaoIDs[1]);
    glGenBuffers(2, &vboIDs[2]);

    // Grid cell vertex positions
    glBindBuffer(GL_ARRAY_BUFFER, vboIDs[2]);
    glBufferData(GL_ARRAY_BUFFER, 7200*sizeof(vec4), boardpoints, GL_STATIC_DRAW);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(vPosition);

    // Grid cell vertex colours
    glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);
    glBufferData(GL_ARRAY_BUFFER, 7200*sizeof(vec4), boardcolours, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(vColor);
}

// ---------------------------------------------------------------------------
void base()
{
    mat4 instance = ( Translate( -15, 0.5 * BASE_HEIGHT, 0.0 ) *
         Scale( BASE_WIDTH,
            BASE_HEIGHT,
            BASE_WIDTH ) );

    glBindVertexArray(vao);
    glUniformMatrix4fv( model_view, 1, GL_TRUE, model_v * instance );

    glDrawArrays( GL_TRIANGLES, 0, NumVertices );
}

//----------------------------------------------------------------------------
void upper_arm()
{
    mat4 instance = ( Translate( -15, 0.5 * UPPER_ARM_HEIGHT, 0.0 ) *
              Scale( UPPER_ARM_WIDTH,
                 UPPER_ARM_HEIGHT,
                 UPPER_ARM_WIDTH ) );
    
    glUniformMatrix4fv( model_view, 1, GL_TRUE, model_v * instance );
    glDrawArrays( GL_TRIANGLES, 0, NumVertices );
}

//----------------------------------------------------------------------------
void lower_arm()
{
    mat4 instance = ( Translate( -15, 0.5 * LOWER_ARM_HEIGHT, 0.0 ) *
              Scale( LOWER_ARM_WIDTH,
                 LOWER_ARM_HEIGHT,
                 LOWER_ARM_WIDTH ) );
    
    glUniformMatrix4fv( model_view, 1, GL_TRUE, model_v * instance );
    glDrawArrays( GL_TRIANGLES, 0, NumVertices );
}


// OpenGL initialization
void init()
{
    colorcube();

    // Create a vertex array object
    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );

    // Create and initialize a buffer object
    GLuint buffer1,buffer2;
    glGenBuffers( 1, &buffer1 );
    glBindBuffer( GL_ARRAY_BUFFER, buffer1 );
    glBufferData( GL_ARRAY_BUFFER, sizeof(points) + sizeof(colors),
          NULL, GL_STATIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(points), points );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors );

    // Load shaders and use the resulting shader program
    GLuint program = InitShader( "vshader.glsl", "fshader.glsl" );
    glUseProgram( program );

    // set up vertex arrays
    vPosition = glGetAttribLocation( program, "vPosition" );
    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
               BUFFER_OFFSET(0) );

    vColor = glGetAttribLocation( program, "vColor" ); 
    glEnableVertexAttribArray( vColor );
    glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0,
               BUFFER_OFFSET(sizeof(points)) );

    //
    glGenVertexArrays( 1, &vao2 );
    glBindVertexArray( vao2 );
    glGenBuffers( 1, &buffer2 );
    glBindBuffer( GL_ARRAY_BUFFER, buffer2 );
    glBufferData( GL_ARRAY_BUFFER, sizeof(points) + sizeof(colors),
          NULL, GL_STATIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(points), points );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors );
    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
               BUFFER_OFFSET(0) );
    glEnableVertexAttribArray( vColor );
    glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0,
               BUFFER_OFFSET(sizeof(points)) );


    model_view = glGetUniformLocation( program, "model_view" );
    projection = glGetUniformLocation( program, "projection" );

    glGenVertexArrays(3, &vaoIDs[0]);
    initGrid();
    initCurrentTile();
    initBoard();

    newtile(); // create new next tile
    
    glEnable( GL_DEPTH_TEST );
    glEnable( GL_BLEND );
    glClearColor( 0.0, 0.0, 0.0, 0.0 ); 
}

//----------------------------------------------------------------------------
void rotate()
{     
    //if(release == false)
      //  return;
    currentShape += 1;
    if(currentShape % 4 == 0)
        currentShape -= 4;
    // update tiles
    for(int i = 0; i < 4; i++){
        tile[i] = allRotationsShape[currentShape][i];
    }
    updatetile();   
    glutPostRedisplay();
}

//------------------------------------------------------------------------------------------------------------------
void settile()
{
    
    glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);

    // update colors
    for(int n = 0; n < 4; n++)
    {
        // set the cells as occupied
        board[cell[n][0]][cell[n][1]] = true;    
        // set boardcolor buffer
        for(int k = (cell[n][0]*6*6 + cell[n][1]*60*6);     k < (cell[n][0]*6*6 + cell[n][1]*60*6 + 36);      k++)
            boardcolours[k] = colorInTile[n];

    }

    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(boardcolours), boardcolours);
    glBindVertexArray(0);
    
    newtile();
}

void display( void )
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    point4  eye( radius*sin(alpha), 60, radius*cos(alpha), 1.0 );
    point4  center( 0, 60/2, 0, 1.0 );
    vec4    up( 0.0, 1.0, 0.0, 0.0 );

    mat4  lk_at = LookAt( eye, center, up );
    glUniformMatrix4fv( model_view, 1, GL_TRUE, lk_at );

    mat4  persp = Perspective( fovy, aspect, zNear, zFar );
    glUniformMatrix4fv( projection, 1, GL_TRUE, persp );
    glBindVertexArray(vaoIDs[1]); // Bind the VAO representing the board
    glDrawArrays( GL_TRIANGLES, 0, 7200);

    mat4 Modelv;
    // display the base
    Modelv = Translate( -25 , 1, 2 );
    mat4 instance1 = Modelv * Scale( 12, 2, 12);
    glUniformMatrix4fv( model_view, 1, GL_TRUE, lk_at * instance1 );
    glBindVertexArray(vao);
    glDrawArrays( GL_TRIANGLES, 0, NumVertices );

    // display the lower arm
    Modelv *= Translate( 0, 15, 0 );
    mat4 instance2 = Modelv * Translate( 0, -15, 0 ) * RotateZ(-theta) * Translate( 0, 15, 0 ) * Scale( 1,30,1 );    
    glUniformMatrix4fv( model_view, 1, GL_TRUE, lk_at*instance2 );
    glBindVertexArray(vao);
    glDrawArrays( GL_TRIANGLES, 0, NumVertices );

    // dislay the upper arm
    Modelv *= Translate( 0, -15, 0 ) * RotateZ(-theta) * Translate( 0, 15, 0 );
    Modelv *= Translate( 0, (40+30)/2, 0);
    mat4 instance3 = Modelv * Translate( 0, -20, 0 )* RotateZ( -phi) * Translate( 0, 20, 0 )* Scale( 1,40,1 );
    glUniformMatrix4fv( model_view, 1, GL_TRUE, lk_at*instance3 );
    glBindVertexArray(vao);
    glDrawArrays( GL_TRIANGLES, 0, NumVertices );

    // display the grid
    glBindVertexArray(vaoIDs[0]); // Bind the VAO representing the grid lines (to be drawn on top of everything else)
    glUniformMatrix4fv( model_view, 1, GL_TRUE, lk_at);
    glDrawArrays(GL_LINES, 0, 128); // Draw the grid lines (21+11 = 32 lines)

    glUniformMatrix4fv( model_view, 1, GL_TRUE, lk_at);

    // display current tile
    glBindVertexArray(vaoIDs[2]); // Bind the VAO representing the current tile (to be drawn on top of the board)
    glDrawArrays(GL_TRIANGLES, 0, 144); // Draw the current tile (8 triangles)

    glutPostRedisplay();
    glutSwapBuffers();
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

// -----------------------------------------------------------------------------------------------------
void down(int value)
{
    if(release == true)
        if(movetile(vec2(0,-1)) ==  false)
        {
            settile();
        }
    glutTimerFunc(200,down,1);
}

// ---------------------------------------------------------------------------
void restart()
{
    initBoard();
    newtile();
}

//---------------------------------------------------------------------------a-
void keyboard( unsigned char key, int x, int y )
{
    switch(key)
    {
        //quit
        case 033:case 'q':
            exit( EXIT_SUCCESS );
            break;
        //restart
        case 'r':
            restart();
            break;
        //drop a tile
        case ' ':
            if(tileAdjust()==true)
                release = true;
            break;
        //increase theta
        case 'd':
            theta += 15*unit;
            updatetile();
            glutPostRedisplay();
            break;
        //decrease theta
        case 'a':
            theta -= 10*unit;
            updatetile();
            glutPostRedisplay();
            break;
        //increase phi
        case 's':
            phi += 10*unit;
            updatetile();
            glutPostRedisplay();
            break;
        //decrease phi
        case 'w':
            phi -= 10*unit;
            updatetile();
            glutPostRedisplay();
            break;


    }
}

//----------------------------------------------------------------------------
void special(int key, int x, int y)
{
    switch( key ) {
        case GLUT_KEY_UP:
            rotate();
            break;

        case GLUT_KEY_LEFT:
            if(glutGetModifiers() == GLUT_ACTIVE_CTRL)
            {
                alpha += unit/5;
                updatetile();
            }
            break;

        case GLUT_KEY_RIGHT:
            if(glutGetModifiers() == GLUT_ACTIVE_CTRL)
            {
                alpha -= unit/5;
                updatetile();
            }
            break;
    }
    
    glutPostRedisplay();
}

//----------------------------------------------------------------------------
void reshape( int width, int height )
{
    glViewport( 0, 0, width, height );
    aspect = GLfloat(width)/height;
}

//----------------------------------------------------------------------------
void timer(int value)
{

}

//---------------------------------------------------------------------------
int main( int argc, char **argv )
{
    glutInit( &argc, argv );
    glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
    glutInitWindowSize( 800, 800 );
    glutCreateWindow( "Color Cube" );

    glewInit();

    init();

    glutDisplayFunc( display );
    glutSpecialFunc(special);
    glutKeyboardFunc( keyboard );
    glutReshapeFunc( reshape );
    glutTimerFunc(200,down,1);
    glutMainLoop();
    return 0;
}
