//#include<windows.h>
#include<stdio.h>
#include<stdlib.h>
#define GL_SILENCE_DEPRECATION
#include<GLUT/glut.h>
#include<OpenGL/glu.h>
#include<OpenGL/gl.h>
#include<math.h>

GLint m_viewport[4];
bool mButtonPressed = false;
float mouseX, mouseY;
enum view {INTRO, MENU, INSTRUCTIONS};
view viewPage = INTRO; // initial value


void displayRasterText(float x ,float y ,float z ,char *stringToDisplay) {
	glRasterPos3f(x, y, z);
	for(char* c = stringToDisplay; *c != '\0'; c++){
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24 , *c); 
	}
}

void init()
{	
	glClearColor(0.0,0.0,0.0,0);
	glColor3f(1.0,0.0,0.0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
    gluOrtho2D(-1200,1200,-700,700);                   //<-----CHANGE THIS TO GET EXTRA SPACE
//  gluOrtho2D(-200,200,-200,200);
	glMatrixMode(GL_MODELVIEW);
}

void introScreen()
{
	glClear(GL_COLOR_BUFFER_BIT);

		glColor3f(1.0, 0.0, 0.0);
	displayRasterText(-425, 490, 0.0,"NMAM INSTITUTE OF TECHNOLOGY");
		glColor3f(1.0, 1.0, 1.0);
	displayRasterText(-700, 385, 0.0,"DEPARTMENT OF COMPUTER SCIENCE AND ENGINEERING");
		glColor3f(0.0, 0.0, 1.0);
	displayRasterText(-225, 300, 0.0,"A MINI PROJECT ON ");
		glColor3f(1.0, 0.0, 1.0);
	displayRasterText(-125, 225, 0.0,"Space Shooter");
   		glColor3f(1.0, 0.7, 0.8);
	displayRasterText(-100, 150, 0.0,"created by");
		glColor3f(1.0, 1.0, 1.0);
	 displayRasterText(-115, 80, 0.0,"SHOOTERS");
	 	glColor3f(1.0, 0.0, 0.0);
	  displayRasterText(-800, -100, 0.0," STUDENT NAMES");
	  	glColor3f(1.0, 1.0, 1.0);
	displayRasterText(-800, -200, 0.0," Saurav N Shetty");
	displayRasterText(-800, -285, 0.0," Rajath R Pai");
		glColor3f(1.0, 0.0, 0.0);
	displayRasterText(500, -100, 0.0,"Under the Guidance of");
		glColor3f(1.0, 1.0, 1.0);
	displayRasterText(500, -200, 0.0,"Prof Puneeth");
		glColor3f(1.0, 0.0, 0.0);
	displayRasterText(-250, -400, 0.0,"Academic Year 2020-2021");
        glColor3f(1.0, 1.0, 1.0);
	displayRasterText(-300, -550, 0.0,"Press ENTER to start the game");
	glFlush();
 	glutSwapBuffers();
}

void startScreenDisplay()
{	
	glLineWidth(10);
	//SetDisplayMode(MENU_SCREEN);
	printf("Start screen display called !\n");
	
	glColor3f(1,0,0);
	glBegin(GL_LINE_LOOP);               //Border
		glVertex2f(-750 ,-500);
		glVertex2f(-750 ,550);
		glVertex2f(750 ,550);
		glVertex2f(750 ,-500);
	glEnd();
	
	glLineWidth(1);

	glColor3f(1, 1, 0);
	glBegin(GL_POLYGON);				//START GAME PLOYGON
		glVertex2f(-200 ,300);
		glVertex2f(-200 ,400);
		glVertex2f(200 ,400);
		glVertex2f(200 ,300);
	glEnd();
	
	glBegin(GL_POLYGON);				//INSTRUCTIONS POLYGON
		glVertex2f(-200, 50);
		glVertex2f(-200 ,150);
		glVertex2f(200 ,150);
		glVertex2f(200 ,50);
	glEnd();

	glBegin(GL_POLYGON);				//QUIT POLYGON
		glVertex2f(-200 ,-200);
		glVertex2f(-200 ,-100);
		glVertex2f(200, -100);
		glVertex2f(200, -200);
	glEnd();

	if(mouseX>=-100 && mouseX<=100 && mouseY>=150 && mouseY<=200){
		glColor3f(0 ,0 ,1) ;
		if(mButtonPressed){
			//startGame = true ;
			//gameOver = false;
			mButtonPressed = false;
		}
	} else
		glColor3f(0 , 0, 0);

	displayRasterText(-100 ,340 ,0.4 ,"Start Game");
	
	if(mouseX>=-100 && mouseX<=100 && mouseY>=30 && mouseY<=80) {
		glColor3f(0 ,0 ,1);
		if(mButtonPressed){
			viewPage = INSTRUCTIONS;
			printf("button pressed bitch\n");
			mButtonPressed = false;
		}
	} else
		glColor3f(0 , 0, 0);
	displayRasterText(-120 ,80 ,0.4 ,"Instructions");
	
	if(mouseX>=-100 && mouseX<=100 && mouseY>=-90 && mouseY<=-40){
		glColor3f(0 ,0 ,1);
		if(mButtonPressed){
			//gameQuit = true ;
			mButtonPressed = false;
		}
	}
	else
		glColor3f(0 , 0, 0);
	displayRasterText(-100 ,-170 ,0.4 ,"    Quit");
	glutPostRedisplay();
}

void backButton() {
	if(mouseX <= -450 && mouseX >= -500 && mouseY >= -275 && mouseY <= -250){
			glColor3f(0, 0, 1);
			if(mButtonPressed) {
				viewPage = MENU;
				mButtonPressed = false;
				//instructionsGame = false;
				glutPostRedisplay();
			}
	}
	else glColor3f(1, 0, 0);
	displayRasterText(-1000 ,-550 ,0, "Back");
}

void instructionsScreenDisplay()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//SetDisplayMode(MENU_SCREEN);
	//colorBackground();
	glColor3f(1, 0, 0);
	displayRasterText(-900 ,400 ,0.4 ,"Key 'w' to move up.");
	displayRasterText(-900 ,300 ,0.4 ,"Key 's' to move down.");
	displayRasterText(-900 ,200 ,0.4 ,"Key 'd' to move right.");
	displayRasterText(-900 ,100 ,0.4 ,"Key 'a' to move left.");
	displayRasterText(-900 ,0.0 ,0.4 ,"Left mouse click to shoot laser");
	//displayRasterText(-900 ,-100 ,0.4 ,"The packet can be placed only when 's' is pressed before.");
	displayRasterText(-900 ,-200 ,0.4 ,"You Get 1 point for shooting each objet and 50 points for completing each lvl ");
	displayRasterText(-900, -270,0.4,"The Objective is to score maximum points");
	backButton();
	//if(previousScreen)
	//	nextScreen = false ,previousScreen = false; //as set by backButton()
}

void display()
{
	//glClearColor(, 0 , 0, 1);
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
	}

	glFlush();
	glutSwapBuffers();
}

// void reshape(GLint w, GLint h)
// {
// 	glViewport(0, 0, w, h);
// 	glMatrixMode(GL_PROJECTION);
// 	glLoadIdentity();
// 	if(h>w)
// 	{
// 		gluOrtho2D(0, 500, ((float)h/(float)w)*(0), ((float)h/(float)w)*500);
// 	}
// 	else
// 	{
// 		gluOrtho2D(((float)w/(float)h)*(0), ((float)w/(float)h)*(500), 0, 500);
// 	}
// 	glMatrixMode(GL_MODELVIEW);
// 	glutPostRedisplay();
// }

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

void keyPressed(unsigned char k, int x, int y)
{
	printf("key pressed\n");
	switch (k)
	{
		case 13: //on ENTER key pressed
			if(viewPage == INTRO) {
				viewPage = MENU;
				printf("view value changed to %d", viewPage);
			}
			printf("enter key pressed\n");
			break;
	}
	glutPostRedisplay();
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE|GLUT_RGB);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(1200, 600);
    glutCreateWindow("Space Shooter");
    init();
    //glutReshapeFunc(reshape);
    glutKeyboardFunc(keyPressed);
	glutMouseFunc(mouseClick);
	glutPassiveMotionFunc(passiveMotionFunc);
	glGetIntegerv(GL_VIEWPORT ,m_viewport);
    glutDisplayFunc(display);
    glutMainLoop();
}
