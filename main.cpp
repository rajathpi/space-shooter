/* =========================================================================
 *  SPACE SHOOTER
 *  A two-player OpenGL/GLUT mini project. Two aliens in flying saucers face
 *  each other and shoot lasers until one of them runs out of LIFE.
 *
 *  -----------------------------------------------------------------------
 *  BUILDING AND RUNNING
 *  -----------------------------------------------------------------------
 *  You need OpenGL plus GLUT (freeglut is the usual modern version).
 *
 *    Code::Blocks : create a "GLUT project", replace the generated main file
 *                   with this one, and make sure freeglut's include folder,
 *                   lib folder and DLL are set up in the project options.
 *    g++          : g++ main.cpp -o space-shooter -lfreeglut -lopengl32 -lglu32
 *    Linux        : g++ main.cpp -o space-shooter -lglut -lGL -lGLU
 *    macOS        : g++ main.cpp -o space-shooter -framework OpenGL -framework GLUT
 *
 *  -----------------------------------------------------------------------
 *  CONTROLS
 *  -----------------------------------------------------------------------
 *    PLAYER 1 (left, amber)   w/a/s/d to move, c to shoot
 *    PLAYER 2 (right, jade)   i/j/k/l to move, m to shoot
 *  Holding an up/down key while shooting angles the laser. A laser that
 *  touches the other saucer takes 5 points off that player's LIFE.
 *
 *  -----------------------------------------------------------------------
 *  HOW THE PROGRAM IS ORGANISED
 *  -----------------------------------------------------------------------
 *  GLUT programs are "callback driven". We never write a loop that runs the
 *  game ourselves; instead we hand GLUT a set of functions in main() and it
 *  calls them when something happens:
 *
 *      display()            called whenever the window must be redrawn
 *      reshape()            called when the window changes size
 *      keyPressed/Released  called when a key goes down or comes back up
 *      mouseClick()         called on a mouse button
 *      passiveMotionFunc()  called when the mouse moves
 *      timer()              called by us on a schedule, to drive animation
 *
 *  glutMainLoop() at the bottom of main() never returns - from that point on
 *  the program is entirely driven by those callbacks.
 *
 *  The game has five screens, tracked by the `viewPage` variable:
 *
 *      INTRO  --ENTER-->  MENU  --click Start-->  GAME  --life hits 0-->  GAMEOVER
 *                          |  ^                                              |
 *                          |  +----------------- (menu is redrawn) ----------+
 *                          +--click Instructions--> INSTRUCTIONS --Back--> MENU
 *
 *  display() looks at viewPage every frame and calls the matching screen
 *  function, so adding a new screen means adding a value to the enum and a
 *  case to that switch.
 *
 *  -----------------------------------------------------------------------
 *  THE COORDINATE SYSTEM
 *  -----------------------------------------------------------------------
 *  init() calls gluOrtho2D(-1200, 1200, -700, 700). That sets up a flat 2D
 *  "world" measuring 2400 x 1400 units, with (0,0) dead centre, x growing to
 *  the right and y growing upwards. Those units are not pixels: the world is
 *  stretched to fit whatever size the window happens to be, so the same
 *  drawing code works at any resolution.
 *
 *  -----------------------------------------------------------------------
 *  A LITTLE OPENGL VOCABULARY
 *  -----------------------------------------------------------------------
 *  glBegin(GL_POLYGON) ... glEnd()
 *      Everything between the two is one shape; each glVertex2f() adds a
 *      corner. GL_LINE_STRIP joins the points with lines instead of filling
 *      them, which is how the black outlines are drawn.
 *
 *  glColor3f() / glColor3fv()
 *      Sets the colour used by every vertex that follows, until it is
 *      changed again. Colours are red/green/blue, each 0.0 to 1.0.
 *
 *  glTranslated() / glScalef() / glRotated()
 *      These do not move shapes that are already on screen. They change the
 *      coordinate system that later drawing happens in - so the alien is
 *      always drawn around (0,0) and then moved into place.
 *
 *  glPushMatrix() / glPopMatrix()
 *      Push saves the current coordinate system, pop restores it. Every
 *      push must have a matching pop. This is what lets us move the
 *      coordinate system to draw one saucer without disturbing the other.
 * ========================================================================= */

#ifdef _WIN32
	#include<windows.h>
#endif
#include<stdio.h>
#include<stdlib.h>
#include<GL/glut.h>
#include<math.h>
#define GL_SILENCE_DEPRECATION

#define XMAX 1200
#define YMAX 700
// How far a ship moves each frame. The game is capped at FRAMES_PER_SECOND
// below, so this works out at roughly 180 world units per second.
#define SPACESHIP_SPEED 3
#define FRAMES_PER_SECOND 60
#define FRAME_DELAY (1000/FRAMES_PER_SECOND)   //milliseconds between frames
#define HIT_INTERVAL 12                        //frames between two laser hits


// Filled in by reshape(): the width and height of the window in pixels.
// Needed to turn mouse pixel positions into world coordinates.
GLint m_viewport[4];
bool mButtonPressed = false;      //is the left mouse button down right now?
float mouseX, mouseY;             //cursor position, converted to world units
enum view {INTRO, MENU, INSTRUCTIONS, GAME, GAMEOVER};
view viewPage = INTRO; // initial value
// One slot per character key. keyPressed() sets a slot to true and
// keyReleased() sets it back to false, so keyOperations() can ask "is w
// being held right now?". Reacting to the key-down event alone would make
// the ships stutter, because the operating system repeats held keys slowly.
bool keyStates[256] = {false};
// Which way each laser points: slot 0 means up, slot 1 means down, and
// neither set means straight across at the opponent.
bool laser1Dir[2] = {false};
bool laser2Dir[2] = {false};

unsigned long frameCount = 0;      //how many frames have been drawn so far
unsigned long lastHitFrame1 = 0;   //when each player was last hit, so that a
unsigned long lastHitFrame2 = 0;   //held-down laser cannot drain them instantly
int alienLife1 = 100;
int alienLife2 = 100;
// Ship positions. Both start at x = 500 because ship two is drawn mirrored
// (see gameScreenDisplay), which puts it on the opposite side of the screen.
float xOne = 500, yOne = 0;       //the right-hand ship  - PLAYER 2
float xTwo = 500, yTwo = 0;       //the left-hand ship   - PLAYER 1
bool laser1 = false, laser2 = false;   //is each ship firing?
GLint CI=0;                       //offset into LightColor, to tint the running lights
GLfloat LightColor[][3]={{0.95,0.80,0.40}, {0.62,0.86,0.95}, {0.90,0.55,0.45}};	//running lights

/* ------------------------------------------------------------------------
 * COLOUR PALETTE
 *
 * Every colour in the game is named here instead of being written inline, so
 * the whole look can be re-themed by editing this one block. OpenGL colours
 * are three floats - red, green and blue - each from 0.0 (none) to 1.0 (full).
 * glColor3fv() takes such an array and makes it the "current" colour: every
 * vertex drawn afterwards uses it until the next glColor call.
 *
 * Each player owns one hue: PLAYER 1 is amber, PLAYER 2 is jade. That hue is
 * reused for their alien, their hull, their laser and their LIFE counter, so
 * you can always tell at a glance which half of the screen is yours.
 * ---------------------------------------------------------------------- */

//----- screen text -----
GLfloat colTitle[3]  = {0.95, 0.80, 0.40};		//"SPACE SHOOTER" heading - warm gold
GLfloat colAccent[3] = {0.60, 0.80, 0.95};		//section headings - ice blue
GLfloat colText[3]   = {0.90, 0.92, 0.96};		//body text - soft white
GLfloat colInfo[3]   = {0.58, 0.64, 0.76};		//secondary text - muted slate

//----- the two players -----
GLfloat colTeam1[3]  = {0.93, 0.72, 0.28};		//PLAYER 1 - amber
GLfloat colTeam2[3]  = {0.35, 0.80, 0.50};		//PLAYER 2 - jade
GLfloat colHull1[3]  = {0.52, 0.32, 0.12};		//their hulls: a darker shade of
GLfloat colHull2[3]  = {0.13, 0.40, 0.30};		//the same hue

//----- shared alien and spaceship parts -----
GLfloat colFace[3]   = {0.26, 0.31, 0.52};		//slate indigo
GLfloat colBeak[3]   = {0.88, 0.84, 0.74};		//ivory
GLfloat colEye[3]    = {0.80, 0.93, 1.00};		//pale ice
GLfloat colDome[3]   = {0.62, 0.78, 0.86};		//glass canopy
GLfloat colWheel[3]  = {0.26, 0.28, 0.34};		//graphite
GLfloat colOutline[3]= {0.05, 0.06, 0.09};		//outlines, near-black

//----- menu furniture -----
GLfloat colBorder[3] = {0.35, 0.45, 0.62};		//steel blue frame
GLfloat colButton[3] = {0.80, 0.64, 0.28};		//brass
GLfloat colButtonHot[3]={0.98, 0.82, 0.45};		//brass, lit up under the cursor
GLfloat colButtonText[3]={0.08, 0.08, 0.10};
GLfloat AlienBody[][2]={{-4,9}, {-6,0}, {0,0}, {0.5,9}, {0.15,12}, {-14,18}, {-19,10}, {-20,0},{-6,0}};
GLfloat AlienCollar[][2]={{-9,10.5}, {-6,11}, {-5,12}, {6,18}, {10,20}, {13,23}, {16,30}, {19,39}, {16,38},
						  {10,37}, {-13,39}, {-18,41}, {-20,43}, {-20.5,42}, {-21,30}, {-19.5,23}, {-19,20},
						  {-14,16}, {-15,17},{-13,13},  {-9,10.5}};
GLfloat ALienFace[][2]={{-6,11}, {-4.5,18}, {0.5,20}, {0.,20.5}, {0.1,19.5}, {1.8,19}, {5,20}, {7,23}, {9,29},
						{6,29.5}, {5,28}, {7,30}, {10,38},{11,38}, {11,40}, {11.5,48}, {10,50.5},{8.5,51}, {6,52},
						{1,51}, {-3,50},{-1,51}, {-3,52}, {-5,52.5}, {-6,52}, {-9,51}, {-10.5,50}, {-12,49}, {-12.5,47},
						{-12,43}, {-13,40}, {-12,38.5}, {-13.5,33},{-15,38},{-14.5,32},  {-14,28}, {-13.5,33}, {-14,28},
						{-13.8,24}, {-13,20}, {-11,19}, {-10.5,12}, {-6,11} } ;
GLfloat ALienBeak[][2]={{-6,21.5}, {-6.5,22}, {-9,21}, {-11,20.5}, {-20,20}, {-14,23}, {-9.5,28}, {-7,27}, {-6,26.5},
						{-4.5,23}, {-4,21}, {-6,19.5}, {-8.5,19}, {-10,19.5}, {-11,20.5} };


// The font used for all the on-screen text. GLUT ships a fixed set of fonts;
// change this single line to try another one. The alternatives are
// GLUT_BITMAP_TIMES_ROMAN_24, GLUT_BITMAP_TIMES_ROMAN_10, GLUT_BITMAP_HELVETICA_10,
// GLUT_BITMAP_HELVETICA_12, GLUT_BITMAP_9_BY_15 and GLUT_BITMAP_8_BY_13.
#define UI_FONT GLUT_BITMAP_HELVETICA_18

// Draws a line of text starting at world position (x,y). "Raster" text is made
// of ready-made bitmaps, so it is always the same size on screen no matter how
// the window is scaled or resized.
void displayRasterText(float x ,float y ,float z ,const char *stringToDisplay) {
	glRasterPos3f(x, y, z);
	for(const char* c = stringToDisplay; *c != '\0'; c++){
		glutBitmapCharacter(UI_FONT , *c);
	}
}

// How wide a string will be, measured in world units.
// glutBitmapWidth() answers in screen pixels, but we position everything in the
// world coordinates set up by gluOrtho2D(), so we convert between the two. The
// world is 2400 units wide and the window is m_viewport[2] pixels wide, which
// gives us the number of world units in one pixel. Measuring the text instead
// of hard-coding offsets means the layout stays correct if you change UI_FONT
// or resize the window.
float rasterTextWidth(const char *stringToDisplay) {
	int pixels = 0;
	for(const char* c = stringToDisplay; *c != '\0'; c++)
		pixels += glutBitmapWidth(UI_FONT, *c);
	return pixels * (2400.0f / m_viewport[2]);
}

// Text centred about x.
void displayCenteredRasterText(float x ,float y ,float z ,const char *stringToDisplay) {
	displayRasterText(x - rasterTextWidth(stringToDisplay)/2, y, z, stringToDisplay);
}

// Text that ends at x, used to keep the right-hand LIFE counter off the edge.
void displayRightAlignedRasterText(float x ,float y ,float z ,const char *stringToDisplay) {
	displayRasterText(x - rasterTextWidth(stringToDisplay), y, z, stringToDisplay);
}

// Bitmap fonts come in fixed sizes only, so headings use the stroke font
// instead - it is drawn with lines and can be scaled to any size.
void displayStrokeText(float x ,float y ,float scale ,float thickness ,const char *stringToDisplay) {
	glPushMatrix();
	glTranslatef(x, y, 0);
	glScalef(scale, scale, scale);
	glLineWidth(thickness);
	for(const char* c = stringToDisplay; *c != '\0'; c++){
		glutStrokeCharacter(GLUT_STROKE_ROMAN , *c);
	}
	glPopMatrix();
	glLineWidth(1);
}

// Same, but horizontally centred about x.
void displayCenteredStrokeText(float x ,float y ,float scale ,float thickness ,const char *stringToDisplay) {
	float width = glutStrokeLength(GLUT_STROKE_ROMAN, (const unsigned char*)stringToDisplay) * scale;
	displayStrokeText(x - width/2, y, scale, thickness, stringToDisplay);
}

/* =======================================================================
 * SET-UP
 * ======================================================================= */
// Runs once, before the first frame. Sets the background colour and defines
// the world coordinate system that every drawing function then works in.
void init()
{
	glClearColor(0.02,0.03,0.06,0);
	glColor3f(1.0,0.0,0.0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

    gluOrtho2D(-1200,1200,-700,700);                   //<-----CHANGE THIS TO GET EXTRA SPACE
//  gluOrtho2D(-200,200,-200,200);
	glMatrixMode(GL_MODELVIEW);
}

/* =======================================================================
 * SCREEN 1 - THE TITLE PAGE
 * ======================================================================= */
// The title page. Nothing here moves; it simply waits for ENTER, which is
// picked up by keyOperations().
void introScreen()
{
	glClear(GL_COLOR_BUFFER_BIT);

		glColor3fv(colAccent);
	displayCenteredRasterText(0, 490, 0.0,"NMAM INSTITUTE OF TECHNOLOGY");
		glColor3fv(colText);
	displayCenteredRasterText(0, 415, 0.0,"DEPARTMENT OF COMPUTER SCIENCE AND ENGINEERING");
		glColor3fv(colInfo);
	displayCenteredRasterText(0, 320, 0.0,"A MINI PROJECT ON");
		glColor3fv(colTitle);
	displayCenteredStrokeText(0, 170, 1.1, 5.0,"SPACE SHOOTER");
   		glColor3fv(colInfo);
	displayCenteredRasterText(0, 90, 0.0,"created by");
		glColor3fv(colText);
	displayCenteredRasterText(0, 20, 0.0,"SHOOTERS");
	 	glColor3fv(colAccent);
	displayRasterText(-800, -140, 0.0,"STUDENT NAMES");
	  	glColor3fv(colText);
	displayRasterText(-800, -230, 0.0,"Saurav N Shetty");
	displayRasterText(-800, -300, 0.0,"Rajath R Pai");
		glColor3fv(colAccent);
	displayRasterText(400, -140, 0.0,"Under the Guidance of");
		glColor3fv(colText);
	displayRasterText(400, -230, 0.0,"Prof X");
		glColor3fv(colAccent);
	displayCenteredRasterText(0, -430, 0.0,"Academic Year 2020-2021");
        glColor3fv(colText);
	displayCenteredRasterText(0, -560, 0.0,"Press ENTER to start the game");
}

/* =======================================================================
 * SCREEN 2 - THE MENU
 * ======================================================================= */
// The mouse position is stored in half the units used for drawing (see
// passiveMotionFunc), so a button drawn between y1 and y2 is hovered when the
// cursor lies between half those values.
bool isHovered(float x1 ,float y1 ,float x2 ,float y2) {
	return mouseX >= x1/2 && mouseX <= x2/2 && mouseY >= y1/2 && mouseY <= y2/2;
}

// A menu button is just a filled rectangle, drawn brighter when the cursor
// is over it so the player can see it is clickable.
void drawButton(float x1 ,float y1 ,float x2 ,float y2 ,bool hovered) {
	if(hovered)
		glColor3fv(colButtonHot);		//lit up while the cursor is over it
	else
		glColor3fv(colButton);
	glBegin(GL_POLYGON);
		glVertex2f(x1 ,y1);
		glVertex2f(x1 ,y2);
		glVertex2f(x2 ,y2);
		glVertex2f(x2 ,y1);
	glEnd();
}

// The menu. Note the order: we work out what is hovered first, then draw the
// buttons, then act on any click. Deciding before drawing is what lets a
// button change colour on the same frame the cursor arrives.
void startScreenDisplay()
{
	bool startHovered = isHovered(-200 ,300 ,200 ,400);
	bool instructionsHovered = isHovered(-200 ,50 ,200 ,150);
	bool quitHovered = isHovered(-200 ,-200 ,200 ,-100);

	glLineWidth(10);
	//SetDisplayMode(MENU_SCREEN);

	glColor3fv(colBorder);
	glBegin(GL_LINE_LOOP);               //Border
		glVertex2f(-750 ,-500);
		glVertex2f(-750 ,550);
		glVertex2f(750 ,550);
		glVertex2f(750 ,-500);
	glEnd();

	glLineWidth(1);

	glColor3fv(colTitle);
	displayCenteredStrokeText(0 ,440 ,0.8 ,5.0 ,"SPACE SHOOTER");

	drawButton(-200 ,300 ,200 ,400 ,startHovered);			//START GAME
	drawButton(-200 ,50 ,200 ,150 ,instructionsHovered);	//INSTRUCTIONS
	drawButton(-200 ,-200 ,200 ,-100 ,quitHovered);			//QUIT

	// The labels sit in the middle of each button. displayCenteredRasterText()
	// measures the text for us, so they stay centred whatever font is chosen.
	glColor3fv(colButtonText);
	if(startHovered && mButtonPressed){
		alienLife1 = alienLife2 = 100;
		viewPage = GAME;
		mButtonPressed = false;
	}
	displayCenteredRasterText(0 ,335 ,0.4 ,"Start Game");

	if(instructionsHovered && mButtonPressed){
		viewPage = INSTRUCTIONS;
		mButtonPressed = false;
	}
	displayCenteredRasterText(0 ,85 ,0.4 ,"Instructions");

	if(quitHovered && mButtonPressed){
		mButtonPressed = false;
		exit(0);
	}
	displayCenteredRasterText(0 ,-165 ,0.4 ,"Quit");
	glutPostRedisplay();
}

/* =======================================================================
 * SCREEN 3 - THE INSTRUCTIONS
 * ======================================================================= */
void backButton() {
	if(mouseX <= -450 && mouseX >= -500 && mouseY >= -275 && mouseY <= -250){
			glColor3fv(colAccent);
			if(mButtonPressed) {
				viewPage = MENU;
				mButtonPressed = false;
				//instructionsGame = false;
				glutPostRedisplay();
			}
	}
	else glColor3fv(colText);
	displayRasterText(-1000 ,-550 ,0, "Back");
}

void instructionsScreenDisplay()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//SetDisplayMode(MENU_SCREEN);
	//colorBackground();
	glColor3fv(colAccent);
	displayRasterText(-900 ,550 ,0.4 ,"INSTRUCTIONS");
	glColor3fv(colTeam1);
	displayRasterText(-1000 ,400 ,0.4 ,"PLAYER 1");
	glColor3fv(colTeam2);
	displayRasterText(200 ,400 ,0.4 ,"PLAYER 2");
	glColor3fv(colText);
	displayRasterText(-1100 ,300 ,0.4 ,"Press 'w' to move up.");
	displayRasterText(-1100 ,200 ,0.4 ,"Press 's' to move down.");
	displayRasterText(-1100 ,100 ,0.4 ,"Press 'd' to move right.");
	displayRasterText(-1100 ,0 ,0.4 ,"Press 'a' to move left.");
	displayRasterText(100 ,300 ,0.4 ,"Press 'i' to move up.");
    displayRasterText(100 ,200 ,0.4 ,"Press 'k' to move down.");
    displayRasterText(100 ,100 ,0.4 ,"Press 'l' to move right.");
    displayRasterText(100 ,0 ,0.4 ,"Press 'j' to move left.");
	displayRasterText(-1100 ,-100 ,0.4 ,"Press 'c' to shoot. Hold 'w' or 's' while shooting to aim.");
	displayRasterText(100 ,-100 ,0.4 ,"Press 'm' to shoot. Hold 'i' or 'k' while shooting to aim.");
	displayRasterText(-1100, -300,0.4,"The objective is to kill your opponent.");
	displayRasterText(-1100 ,-370 ,0.4 ,"Each time a player gets shot, LIFE decreases by 5 points.");
	backButton();
	//if(previousScreen)
	//	nextScreen = false ,previousScreen = false; //as set by backButton()
}

/* =======================================================================
 * DRAWING THE ALIEN, PIECE BY PIECE
 * ======================================================================= */
// The alien is built from four flat shapes stacked on top of each other:
// body, then collar, then face, then beak, and finally the eyes. Each one is
// a list of corner points stored in the arrays near the top of the file, and
// each is drawn twice - once filled with colour, once as an outline.
void DrawAlienBody(bool isPlayerTwo)
{
	if(isPlayerTwo)
		glColor3fv(colTeam2);
	else
		glColor3fv(colTeam1);	//BODY color
	glBegin(GL_POLYGON);
	for(int i=0;i<=8;i++)
		glVertex2fv(AlienBody[i]);
	glEnd();

	glColor3fv(colOutline);		//BODY Outline
	glLineWidth(1);
	glBegin(GL_LINE_STRIP);
	for(int i=0;i<=8;i++)
		glVertex2fv(AlienBody[i]);
	glEnd();

	glBegin(GL_LINES);                //BODY effect
		glVertex2f(-13,11);
		glVertex2f(-15,9);
	glEnd();
}
void DrawAlienCollar(bool isPlayerTwo)
{
	if(isPlayerTwo)
		glColor3fv(colTeam2);
	else
		glColor3fv(colTeam1);				//COLLAR
	glBegin(GL_POLYGON);
	for(int i=0;i<=20 ;i++)
		glVertex2fv(AlienCollar[i]);
	glEnd();

	glColor3fv(colOutline);				//COLLAR outline
	glBegin(GL_LINE_STRIP);
	for(int i=0;i<=20 ;i++)
		glVertex2fv(AlienCollar[i]);
	glEnd();
}
void DrawAlienFace()
{
	glColor3fv(colFace);				//FACE

	glBegin(GL_POLYGON);
	for(int i=0;i<=42 ;i++)
		glVertex2fv(ALienFace[i]);
	glEnd();

	glColor3fv(colOutline);				//FACE outline
	glBegin(GL_LINE_STRIP);
	for(int i=0;i<=42 ;i++)
		glVertex2fv(ALienFace[i]);
	glEnd();

	glBegin(GL_LINE_STRIP);      //EAR effect
		glVertex2f(3.3,22);
		glVertex2f(4.4,23.5);
		glVertex2f(6.3,26);
	glEnd();
}
void DrawAlienBeak()
{
	glColor3fv(colBeak);				//BEAK color
	glBegin(GL_POLYGON);
	for(int i=0;i<=14 ;i++)
		glVertex2fv(ALienBeak[i]);
	glEnd();

	glColor3fv(colOutline);				//BEAK outline
	glBegin(GL_LINE_STRIP);
	for(int i=0;i<=14 ;i++)
		glVertex2fv(ALienBeak[i]);
	glEnd();
}
void DrawAlienEyes()
{
	glColor3fv(colEye);				//EYES

	glPushMatrix();
	glRotated(-10,0,0,1);
	glTranslated(-6,32.5,0);      //Left eye
	glScalef(2.5,4,0);
	glutSolidSphere(1,20,30);
	glPopMatrix();

	glPushMatrix();
	glRotated(-1,0,0,1);
	glTranslated(-8,36,0);							//Right eye
	glScalef(2.5,4,0);
	glutSolidSphere(1,20,20);
	glPopMatrix();
}
void DrawAlien(bool isPlayerTwo)
{
	DrawAlienBody(isPlayerTwo);
	DrawAlienCollar(isPlayerTwo);
	DrawAlienFace();
	DrawAlienBeak();
	DrawAlienEyes();
}
/* =======================================================================
 * DRAWING THE SPACESHIP
 * ======================================================================= */
// The saucer. glScalef() squashes a circle into the flat oval hull, which is
// far less work than listing the points of an ellipse by hand.
void DrawSpaceshipBody(bool isPlayerTwo)
{
	if(isPlayerTwo)
		glColor3fv(colHull2);		//BASE
	else
		glColor3fv(colHull1);

	glPushMatrix();
	glScalef(70,20,1);
	glutSolidSphere(1,50,50);
	glPopMatrix();

	glPushMatrix();							//LIGHTS
	glScalef(3,3,1);
	glTranslated(-20,0,0);					//move to the first light
	// Nine lights in a row. Each glTranslated() shifts the drawing position
	// along, so the next sphere lands 5 units further to the right.
	for(int i=0; i<9; i++) {
		glColor3fv(LightColor[(CI+i)%3]);
		glutSolidSphere(1,20,20);
		glTranslated(5,0,0);
	}
	glPopMatrix();
}
void DrawSteeringWheel()
{
	glPushMatrix();
	glLineWidth(3);
	glColor3fv(colWheel);
	glScalef(7,4,1);
	glTranslated(-1.9,5.5,0);
	glutWireSphere(1,8,8);
	glPopMatrix();

}
// The glass dome over the pilot. ("Doom" here means dome.)
void DrawSpaceshipDoom()
{
	glColor3fv(colDome);
	glPushMatrix();
	glTranslated(0,30,0);
	glScalef(35,50,1);
	glutSolidSphere(1,50,50);
	glPopMatrix();
}

// The laser is one thick line running from the ship to the far edge of the
// world. dir[] decides whether it goes up, down, or straight across.
void DrawLaser(int x, int y, bool dir[], GLfloat *colour) {
	int xend = -XMAX, yend = y;
	if(dir[0])
		yend = YMAX;
	else if(dir[1])
		yend = -YMAX;
	glLineWidth(5);
	glColor3fv(colour);
	glBegin(GL_LINES);
		glVertex2f(x, y);
		glVertex2f(xend, yend);
	glEnd();
}

// Draws one complete saucer with its pilot at position (x,y).
// Everything inside is drawn around the origin, and the glTranslated() moves
// the coordinate system so it lands in the right place. The push/pop pair
// makes sure the move is undone afterwards and the next saucer is unaffected.
void SpaceshipCreate(int x, int y, bool isPlayerTwo){
	glPushMatrix();
	glTranslated(x,y,0);
	// if(!checkIfSpaceShipIsSafe() && alienLife1 ){
	// 	alienLife1-=10;
	// 	xStart -= 23;
	// }
	DrawSpaceshipDoom();
	glPushMatrix();
	glTranslated(4,19,0);
	DrawAlien(isPlayerTwo);
	glPopMatrix();
	DrawSteeringWheel();
	DrawSpaceshipBody(isPlayerTwo);
	// DrawSpaceShipLazer();
	// if(mButtonPressed) {
	// 	DrawLazerBeam();
	// }
	glEnd();
	glPopMatrix();
}

/* =======================================================================
 * SCREEN 4 - THE GAME ITSELF
 * ======================================================================= */
void DisplayHealthBar1() {
	char temp1[40];
	glColor3fv(colTeam1);
	snprintf(temp1 ,sizeof(temp1) ,"PLAYER 1   LIFE = %d",alienLife1);
	displayRasterText(-1100 ,600 ,0.4 ,temp1);
}

void DisplayHealthBar2() {
	char temp2[40];
	glColor3fv(colTeam2);
	snprintf(temp2 ,sizeof(temp2) ,"PLAYER 2   LIFE = %d",alienLife2);
	displayRightAlignedRasterText(1100 ,600 ,0.4 ,temp2);	//mirrors PLAYER 1 on the left
}

// Has a laser hit the other saucer?
//
// The laser is a straight line and the saucer is treated as a circle of
// radius r around (xp,yp). Substituting the line y = m*x + k into the circle
// equation gives a quadratic, and the discriminant b*b - 4*a*c of that
// quadratic tells us how many points the two share:
//      below zero -> no solutions  -> the laser misses
//      zero       -> one solution  -> it grazes the edge
//      above zero -> two solutions -> it passes through, so it is a hit
void checkLaserContact(int x, int y, bool dir[], int xp, int yp, bool player1) {
	int xend = -XMAX, yend = y;
	xp += 8; yp += 8; // moving circle slightly up to fix laser issue
	if(dir[0])
		yend = YMAX;
	else if(dir[1])
		yend = -YMAX;

	// Here we find out if the laser(line) intersects with spaceship(circle)
	// by solving the equations for the same and finding the discriminant of the
	// quadratic equation obtained
	float m = (float)(yend - y) / (float)(xend - x);
	float k = y - m * x ;
	int r = 50; // approx radius of the spaceship

	//calculating value of b, a, and c needed to find discriminant
	float b = 2 * xp - 2 * m * (k - yp);
	float a = 1 + m * m;
	float c = xp * xp + (k - yp) * (k - yp) - r * r;

	float d = (b * b - 4 * a * c); // discriminant for the equation

	// A negative discriminant means the line misses the circle entirely; zero or
	// more means it touches or passes through, so the shot has connected.
	//
	// This runs once per frame, and a held-down laser would otherwise take off 5
	// points sixty times a second. HIT_INTERVAL spaces the hits out, so a player
	// under continuous fire loses 5 points about every fifth of a second.
	if(d >= 0) {
		if(player1) {
			if(frameCount - lastHitFrame1 >= HIT_INTERVAL) {
				alienLife1 -= 5;
				lastHitFrame1 = frameCount;
			}
		}
		else {
			if(frameCount - lastHitFrame2 >= HIT_INTERVAL) {
				alienLife2 -= 5;
				lastHitFrame2 = frameCount;
			}
		}
	}
}

// The game itself: two saucers, their lasers, and the two LIFE counters.
//
// glScalef(-1, 1, 1) around the second saucer mirrors it left-to-right. That
// is why both ships share one set of drawing code and one starting x of 500,
// yet appear on opposite sides facing each other. It also means the second
// ship moves the opposite way along x, which is why its keys look reversed
// further down in keyOperations().
void gameScreenDisplay()
{
	DisplayHealthBar1();
	DisplayHealthBar2();
	glScalef(2, 2 ,0);

	if(alienLife1 > 0){
		SpaceshipCreate(xOne, yOne, true);
		if(laser1) {
			DrawLaser(xOne, yOne, laser1Dir, colTeam2);
			checkLaserContact(xOne, yOne, laser1Dir, -xTwo, yTwo, true);
		}
	}
	else {
		viewPage = GAMEOVER;
	}

	if(alienLife2 > 0) {
		glPushMatrix();
		glScalef(-1, 1, 1);
		SpaceshipCreate(xTwo, yTwo, false);
		if(laser2) {
			DrawLaser(xTwo, yTwo, laser2Dir, colTeam1);
			checkLaserContact(xTwo, yTwo, laser2Dir, -xOne, yOne, false);
		}
		glPopMatrix();
	}
	else {
		viewPage = GAMEOVER;
	}

	if(viewPage == GAMEOVER) {
		xOne = xTwo = 500;
		yOne = yTwo = 0;
	}
}

void displayGameOverMessage() {
	const char* message;
	if(alienLife1 > 0) {
		glColor3fv(colTeam1);
		message = "GAME OVER  -  PLAYER 1 WINS";
	}
	else {
		glColor3fv(colTeam2);
		message = "GAME OVER  -  PLAYER 2 WINS";
	}

	displayCenteredStrokeText(0 ,595 ,0.6 ,5.0 , message);
}

/* =======================================================================
 * INPUT - KEYBOARD AND MOUSE
 * ======================================================================= */
// Called once per frame, before anything is drawn. Rather than acting on key
// presses as they arrive, we look at which keys are currently held down. That
// gives smooth movement and lets both players move at the same time.
//
// Note that a ship cannot move while its shoot key is held - the movement
// checks sit in the else branch - so shooting holds you still.
void keyOperations() {
	if(keyStates[13] == true && viewPage == INTRO) {
		viewPage = MENU;
	}
	if(viewPage == GAME) {
		laser1Dir[0] = laser1Dir[1] = false;
		laser2Dir[0] = laser2Dir[1] = false;
		if(keyStates['c'] == true) {
			laser2 = true;
			if(keyStates['w'] == true) 	laser2Dir[0] = true;
			if(keyStates['s'] == true) 	laser2Dir[1] = true;
		}
		else {
			laser2 = false;
			if(keyStates['d'] == true) xTwo-=SPACESHIP_SPEED;
			if(keyStates['a'] == true) xTwo+=SPACESHIP_SPEED;
			if(keyStates['w'] == true) yTwo+=SPACESHIP_SPEED;
			if(keyStates['s'] == true) yTwo-=SPACESHIP_SPEED;
		}

		if(keyStates['m'] == true) {
			laser1 = true;
			if(keyStates['i'] == true) laser1Dir[0] = true;
			if(keyStates['k'] == true) laser1Dir[1] = true;
		}
		else {
			laser1 = false;
			if(keyStates['l'] == true) xOne+=SPACESHIP_SPEED;
			if(keyStates['j'] == true) xOne-=SPACESHIP_SPEED;
			if(keyStates['i'] == true) yOne+=SPACESHIP_SPEED;
			if(keyStates['k'] == true) yOne-=SPACESHIP_SPEED;
		}
	}
}

/* =======================================================================
 * THE MAIN DRAW FUNCTION - CALLED ONCE PER FRAME
 * ======================================================================= */
// Called for every frame. It clears the window, draws whichever screen is
// current, then shows the result.
//
// The window is double buffered: everything is drawn onto a hidden buffer and
// glutSwapBuffers() then puts it on screen in one go. Drawing straight to the
// visible window instead would let the player watch it being built up, which
// shows as flicker.
void display()
{
	frameCount++;
	keyOperations();
	glClear(GL_COLOR_BUFFER_BIT);

	switch (viewPage)
	{
		case INTRO:
			introScreen();
			break;
		case MENU:
			startScreenDisplay();
			break;
		case INSTRUCTIONS:
			instructionsScreenDisplay();
			break;
		case GAME:
			gameScreenDisplay();
			//reset scaling values
			glScalef(1/2 ,1/2 ,0);
			break;
		case GAMEOVER:
			displayGameOverMessage();
			startScreenDisplay();
			break;
	}

	glFlush();
	glLoadIdentity();
	glutSwapBuffers();
}

// Called by GLUT whenever the window is resized. The world coordinates set up
// in init() never change, so the drawing simply stretches to fill the new size.
// We must also refresh m_viewport, because passiveMotionFunc() uses it to
// convert mouse pixels into world coordinates - without this the menu buttons
// stop responding after the window is resized.
void reshape(GLint w, GLint h)
{
	glViewport(0, 0, w, h);
	m_viewport[2] = w;
	m_viewport[3] = h;
	glutPostRedisplay();
}

// Called by GLUT whenever the mouse moves. GLUT reports the position in
// pixels measured from the top-left corner, but our world has (0,0) in the
// middle and y growing upwards, so both axes need converting - hence the
// shift by half the size, and the minus sign on y.
void passiveMotionFunc(int x,int y) {

	//when mouse not clicked
	mouseX = float(x)/(m_viewport[2]/1200.0)-600.0;  //converting screen resolution to ortho 2d spec
	mouseY = -(float(y)/(m_viewport[3]/700.0)-350.0);

	//Do calculations to find value of LaserAngle
	//somethingMovedRecalculateLaserAngle();
	glutPostRedisplay();
}

void mouseClick(int buttonPressed ,int state ,int x, int y) {

	if(buttonPressed == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
		mButtonPressed = true;
	else
		mButtonPressed = false;
	glutPostRedisplay();
}

// GLUT hands us the character of the key that changed. We only record it;
// keyOperations() decides what it means, once per frame.
void keyPressed(unsigned char key, int x, int y)
{
	keyStates[key] = true;
	glutPostRedisplay();
}

// Asks for a redraw, then books the next one FRAME_DELAY milliseconds later.
// Using a timer rather than glutIdleFunc() keeps the game running at the same
// speed on every machine: with an idle loop a fast computer would redraw
// thousands of times a second and the ships would cross the screen instantly.
void timer(int value) {
	glutPostRedisplay();
	glutTimerFunc(FRAME_DELAY, timer, 0);
}

void keyReleased(unsigned char key, int x, int y) {
	keyStates[key] = false;
}

/* =======================================================================
 * PROGRAM ENTRY POINT
 * ======================================================================= */
// Creates the window, tells GLUT which function handles which event, and then
// hands control over to GLUT for good.
int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(1200, 600);
    glutCreateWindow("Space Shooter");
    init();
    glutReshapeFunc(reshape);
	glutTimerFunc(FRAME_DELAY, timer, 0);
    glutKeyboardFunc(keyPressed);
	glutKeyboardUpFunc(keyReleased);
	glutMouseFunc(mouseClick);
	glutPassiveMotionFunc(passiveMotionFunc);
	glGetIntegerv(GL_VIEWPORT ,m_viewport);
    glutDisplayFunc(display);
    glutMainLoop();
}
