#include<windows.h>
#include<stdio.h>
#include<stdlib.h>
#include<GL/glut.h>
#include<math.h>

float  a = 0, b = 0, aa = 0, bb = 0, flag = 0, flag3 = 0;
float x = 100, y = 0, r = 0.5, y11 = 0, y21 = 0, y31 = 0, y41 = 0, y51 = 0, y61 = 0, y71 = 0, y81 = 0, y91 = 0, y10 = 0, y12 = 0, y13 = 0, y14 = 0, x0 = 0, xo = 0, angle = 0.0, pi = 3.142;
void create_menu(void);
void menu(int);

void drawstring(float x, float y, float z, char *string)
{
	char *c;
	glRasterPos3f(x, y, z);
	for(c = string;*c!='\0';c++)
	{
	 	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,*c);
	}
}

void init()
{
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, 500, 0, 500);
}

void screen()
{
	glClear(GL_COLOR_BUFFER_BIT);
		glColor3f(1.0, 0.0, 0.0);
	drawstring(320, 425, 0.0,"NMAM INSTITUTE OF TECHNOLOGY");
		glColor3f(1.0, 1.0, 1.0);
	drawstring(245, 385, 0.0,"DEPARTMENT OF COMPUTER SCIENCE AND ENGINEERING");
		glColor3f(0.0, 0.0, 1.0);
	drawstring(400, 350, 0.0,"A MINI PROJECT ON ");
		glColor3f(1.0, 0.0, 1.0);
	drawstring(435, 315, 0.0,"Space Shooter");
   		glColor3f(1.0, 0.7, 0.8);
	drawstring(445, 290, 0.0,"created by:");
		glColor3f(1.0, 1.0, 1.0);
	 drawstring(430, 260, 0.0,"SHOOTERS");
	 	glColor3f(1.0, 0.0, 0.0);
	  drawstring(135, 205, 0.0," STUDENT NAME");
	  	glColor3f(1.0, 1.0, 1.0);
	drawstring(100, 170, 0.0,"Name Saurav N Shetty");
	drawstring(100, 135, 0.0,"Name Rajath R Pai");
		glColor3f(1.0, 0.0, 0.0);
	drawstring(710, 220, 0.0,"Under the Guidance of");
		glColor3f(1.0, 1.0, 1.0);
	drawstring(720, 190, 0.0,"Prof Puneeth");
		glColor3f(1.0, 0.0, 0.0);
	drawstring(406, 95, 0.0,"Academic Year 2020-2021");
        glColor3f(1.0, 1.0, 1.0);
	drawstring(410, 45, 0.0,"Press 'p' to start the game");
	glFlush();
 	glutSwapBuffers();
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT);
	screen();
}

void mydisplay(void)
{
	glClear(GL_COLOR_BUFFER_BIT);
	if(flag3 == 0)
	{
		screen();
	}
	if(flag3 == 1)
	{
		display();
	}
}

void reshape(GLint w, GLint h)
{
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if(h>w)
	{
		gluOrtho2D(0, 500, ((float)h/(float)w)*(0), ((float)h/(float)w)*500);
	}
	else
	{
		gluOrtho2D(((float)w/(float)h)*(0), ((float)w/(float)h)*(500), 0, 500);
	}
	glMatrixMode(GL_MODELVIEW);
	glutPostRedisplay();
}

void speed()
{
	a = a - 9.0;
}

void slow()
{
	a = a - 0.0001;
}

void key(unsigned char k, int x, int y)
{
	if(k =='i')
        glutIdleFunc(speed);
	if(k =='d')
		glutIdleFunc(slow);
	if(k =='p')
        flag3 = 1;
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE|GLUT_RGB);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(1200, 600);
    glutCreateWindow("Space Shooter");
    init();
    glutReshapeFunc(reshape);
    glutKeyboardFunc(key);
    glutDisplayFunc(display);
    glutMainLoop();
}
