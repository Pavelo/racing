//#include <GLUT/gl.h>
#include <GLUT/glut.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include "tga.h"

//COSTANTI
#define WIDTH 800
#define HEIGHT 600
#define LIFETIME 2.0f
#define PPS 100

//STRUTTURE
//struttura particella
typedef struct
{
	float radius;
	float pos[3];
	float v[3];
	float a[3];
	float mass;
} mySphere;

//struttura particelle
struct _particles
{
	mySphere sphere;
	float lifetime;
	struct _particles *next;
};
typedef struct _particles particles;

//struttura ruota
struct _wheel
{
	float width;
	float length;
	float height;
	float pos[3];
	float v[3];
	float a[3];
	float rot;
};
typedef struct _wheel wheel;

//struttura automobile
struct _car
{
	float width;
	float length;
	float height;
	float pos[3];
	float v[3];
	float a[3];
	float rot;
	wheel forWheelL;
	wheel forWheelR;
	wheel backWheelL;
	wheel backWheelR;
	float throttle;
	float radius;
	float mass;
	float iperBoostAccumul;
	float iperBoost;
};
typedef struct _car car;

//VARIABILI
//particelle
particles *root = NULL;
int partsNumber = 10;

//automobile
car userCar;
float translateX=0.0f, translateZ=0.0f;
float distanceFromCamera = 10.0f;
//ruote
wheel userWheel1; // Back L
wheel userWheel2; // Back R
wheel userWheel3; // For L
wheel userWheel4; // For R

//albero
float treeRot = 0.0f;

//colori
float blue[]      = {0.0, 0.0, 1.0, 1.0};
float carColor[]  = {0.8, 0.1, 0.1, 1.0};
float roadColor[] = {0.0, 0.1, 0.9, 1.0};
float treeColor[] = {0.2, 0.8, 0.2, 1.0};
float wheelColor[] = {1.0, 1.0, 0.3, 1.0};

//modalità blur
float blur = 0.2f;
int blurActive = 0;

//zbuffer
GLubyte *pixels;

//stampe a video
char stampe[80];
char stampe2[80];

//tempi e frame
double deltaT = 0.0f;
struct timeval old;

//finestra
float ar = 1.0f;


//FUNZIONI

//Abilita l'illuminazione della scena
void lightOn(void)
{
//	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
}

//Disabilita l'illuminazione della scena
void lightOff(void)
{
//	glDisable(GL_COLOR_MATERIAL);
	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);
}

//Disabilita l'illuminzione e sposta in proiezione ortogonale
void orthogonalStart (void) {
	lightOff();

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, WIDTH, 0, HEIGHT);
	glScalef(1, -1, 1);
	glTranslatef(0, -HEIGHT, 0);
	glMatrixMode(GL_MODELVIEW);
}

//Ritorna in proiezione precedente e riabilita l'illuminazione
void orthogonalEnd (void) {
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	lightOn();
}

//inserisce una particella all'inizio della lista.
//inizialmente root è inizializzato a null.
//la prima ottiene null come next, e diventa root
//le successive mettono root come successivo elemento della lista
//e diventano root
void addParticle(float r, particles **root)
{
	particles *p;
	float range = 2.0f;
	p = malloc(sizeof(particles));
	p->sphere.radius = r;
	p->sphere.pos[0] = 0.0f;
	p->sphere.pos[1] = 0.0f;
	p->sphere.pos[2] = 0.0f;
	p->sphere.v[0] = ((float)rand()/(float)RAND_MAX)*range - range * 0.5f;
	p->sphere.v[1] = 10.0f;
	p->sphere.v[2] = ((float)rand()/(float)RAND_MAX)*range - range * 0.5f;
	p->sphere.a[0] = 0.0f;
	p->sphere.a[1] = -9.81f;
	p->sphere.a[2] = 0.0f;
	p->sphere.mass = 1.0f;

	p->lifetime = 0.0f;

	p->next = *root;
	*root = p;

}
void removeFirst(particles **root)
{
	particles *tmp;

	if(*root != NULL)
	{
		tmp = *root;
		*root = tmp->next;
		free(tmp);
	}
}

void removeFromLifetime(float lt, particles **root)
{
	particles *p;

	if(*root != NULL)
	{
		p = *root;
		while(p != NULL)
		{
			particles *tmp;

			while (p->lifetime > lt)
			{
				particles *tmp2;
				tmp2 = *root;
				*root = tmp2->next;
				p = *root;


				free(tmp2);
			}

			tmp = p->next;

			while(tmp != NULL && tmp->lifetime > lt)
			{
				if(tmp->next != NULL)
				{
//					printf("\nraggio = %f cambia next\n", p->lifetime);
//					printf("raggio da eliminare = %f\n", tmp->lifetime);
					p->next = tmp->next;
					free(tmp);
					tmp = tmp->next;
				} else
				{
//					printf("sono da eliminare e punto a null\n\n");
					p->next = NULL;
					free(tmp);
					tmp = NULL;

				}
				//free(tmp);
			}//end if

			p=p->next;
		}//end while
	}//end if

}

void drawCube(float width, float height, float length) {

	float w = width *0.5f;
	float h = height*0.5f;
	float l = length*0.5f;


	glBegin(GL_QUADS);

	//faccia in alto
	glNormal3f(0.0f,1.0f,0.0f);
	glVertex3f(-w,h,-l);
	glVertex3f(-w,h,l);
	glVertex3f(w,h,l);
	glVertex3f(w,h,-l);
	//faccia in basso
	glNormal3f(0.0f,-1.0f,0.0f);
	glVertex3f(-w,-h,-l);
	glVertex3f(w,-h,-l);
	glVertex3f(w,-h,l);
	glVertex3f(-w,-h,l);
	//faccia dietro
	glNormal3f(0.0f,0.0f,-1.0f);
	glVertex3f(-w,-h,-l);
	glVertex3f(-w,h,-l);
	glVertex3f(w,h,-l);
	glVertex3f(w,-h,-l);
	//faccia avanti
	glNormal3f(0.0f,0.0f,1.0f);
	glVertex3f(-w,-h,l);
	glVertex3f(-w,h,l);
	glVertex3f(w,h,l);
	glVertex3f(w,-h,l);
	//faccia sx
	glNormal3f(-1.0f,0.0f,0.0f);
	glVertex3f(-w,-h,l);
	glVertex3f(-w,-h,-l);
	glVertex3f(-w,h,-l);
	glVertex3f(-w,h,l);
	//faccia dx
	glNormal3f(1.0f,0.0f,0.0f);
	glVertex3f(w,-h,l);
	glVertex3f(w,-h,-l);
	glVertex3f(w,h,-l);
	glVertex3f(w,h,l);

	glEnd();
}

void makeRoad(float lenght, float width, float nol)
{
	float l = lenght * 0.5;
	float w = width  * 0.5;
	float stepl = lenght/nol;
	float stepw = width/nol;
	int i;

	for (i=0; i<=nol; i++)
	{
		glBegin(GL_LINES);
		glNormal3f(0.0f,1.0f,0.0f);
		glVertex3f(-w+(stepw*i),0.0f,-l);
		glVertex3f(-w+(stepw*i),0.0f,l);
		glEnd();
		glBegin(GL_LINES);
		glNormal3f(0.0f,1.0f,0.0f);
		glVertex3f(-w,0.0f,-l+(stepl*i));
		glVertex3f(w,0.0f,-l+(stepl*i));
		glEnd();
	}
}

void reshape ( int w, int h)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glViewport(0, 0, w, h);
	ar = (float)w/(float)h;
	gluPerspective(45.0, ar, 0.2f, 100.0f);


	glMatrixMode(GL_MODELVIEW);

	//zbuffer
	//free(pixels);
	//pixels = malloc(sizeof(GLubyte)*w*h);
}

void renderBitmapString(float x, float y, void *font, char *string)
{
	orthogonalStart();
	char *c;
	glRasterPos2f(x,y);
	for (c=string; *c != '\0'; c++)
	{
		glutBitmapCharacter(font, *c);
	}
	orthogonalEnd();
}

void keyb(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 27:	exit(0);
	break;

	case 's':	//if(userCar.throttle < 1.0f)
		userCar.throttle = 20.0f;
		break;

	case 'd':   if(userWheel3.rot > -45.0f) {
		userWheel3.rot  -=  2.5f;
		userWheel4.rot  -=  2.5f;
	}
	break;

	case 'w':	//if(userCar.throttle > -1.0f)
		userCar.throttle = -10.0f - userCar.iperBoost*blurActive;
		break;

	case 'a':   if(userWheel3.rot < 45.0f) {
		userWheel3.rot  +=  2.5f;
		userWheel4.rot  +=  2.5f;
	}
	break;

	case 32:    userCar.v[0] = userCar.a[0]/2;
	break;

	//reset
	case 'r':	userCar.width = 1.0f;
	userCar.height = 1.0f;
	userCar.length = 2.0f;
	userCar.pos[0] = 0.0f;
	userCar.pos[1] = 0.5f;
	userCar.pos[2] = 0.0f;
	userCar.v[0] = 0.0f;
	userCar.v[1] = 0.0f;
	userCar.v[2] = 0.0f;
	userCar.a[0] = 0.0f;
	userCar.a[1] = 0.0f;
	userCar.a[2] = 0.0f;
	userCar.rot  = 0.0f;
	userCar.throttle  = 0.0f;
	userCar.mass = 1.0f;
	break;
	case 'b':	if(userCar.iperBoostAccumul>=1.0f) {
		if(blurActive==1)
			blurActive = 0;
		else
			blurActive = 1;
		sprintf(stampe2,"IPERBOOST activated! \n");
	}
	break;

	default:	break;
	}
	glutPostRedisplay();
}

float scalProd(float v1[3], float v2[3])
{
	return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
}

void init(void)
{
	float ambient[] = {1.0f, 1.0f, 1.0f};
	float diffuse[] = {1.0f, 1.0f, 1.0f};
	float specular[] = {1.0f, 1.0f, 1.0f};

    glEnable(GL_LIGHTING);

    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
	glEnable(GL_LIGHT0);

	glLightfv(GL_LIGHT1, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, specular);
	glEnable(GL_LIGHT1);

	glLightfv(GL_LIGHT2, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT2, GL_SPECULAR, specular);
	glEnable(GL_LIGHT2);

	// da usare solo insieme a glColor per definire un colore indipendente dalle sorgenti di luce!
//	glColorMaterial(GL_FRONT, GL_DIFFUSE);
//	glEnable(GL_COLOR_MATERIAL);

	glEnable(GL_DEPTH_TEST);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	if(blurActive==1)
	{
		glClearAccum(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_ACCUM_BUFFER_BIT);
		glReadBuffer(GL_BACK);
		glDrawBuffer(GL_BACK);
	}

	pixels = malloc(sizeof(GLubyte)*WIDTH*HEIGHT);

	gettimeofday(&old, NULL);
	srand(old.tv_sec);

	// BACKWARD LEFT
	userWheel1.width = 0.4f;
	userWheel1.height = 0.4f;
	userWheel1.length = 0.4f;
	userWheel1.pos[0] = -0.5f; // X
	userWheel1.pos[1] = -0.5f; // Y
	userWheel1.pos[2] = 1.0f;  // Z
	userWheel1.v[0] = 0.0f;
	userWheel1.v[1] = 0.0f;
	userWheel1.v[2] = 0.0f;
	userWheel1.a[0] = 0.0f;
	userWheel1.a[1] = 0.0f;
	userWheel1.a[2] = 0.0f;
	userWheel1.rot  = 0.0f;

	//BACKWARD RIGHT
	userWheel2.width = 0.4f;
	userWheel2.height = 0.4f;
	userWheel2.length = 0.4f;
	userWheel2.pos[0] = 0.5f;
	userWheel2.pos[1] = -0.5f;
	userWheel2.pos[2] = 1.0f;
	userWheel2.v[0] = 0.0f;
	userWheel2.v[1] = 0.0f;
	userWheel2.v[2] = 0.0f;
	userWheel2.a[0] = 0.0f;
	userWheel2.a[1] = 0.0f;
	userWheel2.a[2] = 0.0f;
	userWheel2.rot  = 0.0f;

	//FORWARD LEFT
	userWheel3.width = 0.4f;
	userWheel3.height = 0.4f;
	userWheel3.length = 0.4f;
	userWheel3.pos[0] = -0.5f;
	userWheel3.pos[1] = -0.5f;
	userWheel3.pos[2] = 0.1f;
	userWheel3.v[0] = 0.0f;
	userWheel3.v[1] = 0.0f;
	userWheel3.v[2] = 0.0f;
	userWheel3.a[0] = 0.0f;
	userWheel3.a[1] = 0.0f;
	userWheel3.a[2] = 0.0f;
	userWheel3.rot  = 0.0f;


	//FORWARD RIGHT
	userWheel4.width = 0.4f;
	userWheel4.height = 0.4f;
	userWheel4.length = 0.4f;
	userWheel4.pos[0] = 0.5f;
	userWheel4.pos[1] = -0.5f;
	userWheel4.pos[2] = 0.1f;
	userWheel4.v[0] = 0.0f;
	userWheel4.v[1] = 0.0f;
	userWheel4.v[2] = 0.0f;
	userWheel4.a[0] = 0.0f;
	userWheel4.a[1] = 0.0f;
	userWheel4.a[2] = 0.0f;
	userWheel4.rot  = 0.0f;

	userCar.width = 1.0f;
	userCar.height = 1.0f;
	userCar.length = 2.0f;
	userCar.pos[0] = 0.0f;
	userCar.pos[1] = userCar.height*0.5f + userWheel1.height*0.5;
	userCar.pos[2] = 0.0f;
	userCar.v[0] = 0.0f;
	userCar.v[1] = 0.0f;
	userCar.v[2] = 0.0f;
	userCar.a[0] = 0.0f;
	userCar.a[1] = 0.0f;
	userCar.a[2] = 0.0f;
	userCar.rot  = 0.0f;
	userCar.forWheelL = userWheel1;
	userCar.forWheelR = userWheel2;
	userCar.backWheelL = userWheel3;
	userCar.backWheelR = userWheel4;
	userCar.radius = 5.0f;
	userCar.throttle = 0.0f;
	userCar.mass = 1.0f;
	userCar.iperBoost = 20.0f;
	userCar.iperBoostAccumul = 0.0f;

	sprintf(stampe,"Accelerazione: %f.\n Velocità: %f.\n",userCar.throttle,sqrt(userCar.v[0]*userCar.v[0]+userCar.v[2]*userCar.v[2]));

	//carico i modelli
	loadOBJ("obj/tank_camo.obj", 0);

}

void display(void)
{
	particles *p;
	float light0Position[] = {0.0f, 10.0f, 20.0f, 1.0f};
	float light1Position[] = {20.0f, -10.0f, 0.0f, 1.0f};
	float light2Position[] = {-20.0f, 0.0f, -20.0f, 1.0f};
	float cubeMAmbient[] = {0.1f, 0.1f, 0.1f};
	float cubeMDiffuse[] = {0.5f, 0.5f, 0.5f};
	float cubeMSpecular[] = {0.0f, 0.0f, 0.0f};
	float cubeMShininess = 0.0f;

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

//	glColor4fv(wheelColor);
	renderBitmapString(100,100,GLUT_BITMAP_HELVETICA_18,stampe);
	renderBitmapString(500,500,GLUT_BITMAP_HELVETICA_18,stampe2);


	//Sposta la telecamera nella posizione della macchina, quindi si allontana
	//in relazione all'angolo di rotazione e guarda verso la macchina
	//PROVATO A SEGUIRE LE RUOTE DAVANTI CONTROLLARE BENE!!!ANCHE POS[2]
	gluLookAt(userCar.pos[0]+(userWheel3.pos[0]+userWheel4.pos[0])/2.0f+ distanceFromCamera*sin(userCar.rot*M_PI/180.0f), //Eye x
			3.5f,                                                            //Eye y
			userCar.pos[2]+ distanceFromCamera*cos(userCar.rot*M_PI/180.0f), //Eye z
			userCar.pos[0]+(userWheel3.pos[0]+userWheel4.pos[0])/2.0f, 3.0f,userCar.pos[2]+(userWheel3.pos[2]+userWheel4.pos[2])/2.0f, //At
			0.0f, 1.0f, 0.0f);//Up

	glLightfv(GL_LIGHT0, GL_POSITION, light0Position);
	glLightfv(GL_LIGHT1, GL_POSITION, light1Position);
	glLightfv(GL_LIGHT2, GL_POSITION, light2Position);

/*	glMaterialfv(GL_FRONT, GL_AMBIENT, cubeMAmbient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, cubeMDiffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, cubeMSpecular);
	glMaterialf(GL_FRONT, GL_SHININESS, cubeMShininess);*/


	//DISEGNO


	glPushMatrix();
//		glRotatef(20, 1.0f, 0.0f, 0.0f);
		glTranslatef(0.0f, 5.0f, 0.0f);
//		glTranslatef(0.0f, 2.0f, 0.0f);
		drawOBJ(0);
//		drawCube(1.0f, 1.0f, 1.0f);
	glPopMatrix();

	glPushMatrix();
//		glRotatef(45, 1.0f, 0.0f, 0.0f);
		glTranslatef(0.0f, 2.0f, 0.0f);
//		drawOBJ(1);
	glPopMatrix();

	glPushMatrix();
//		glRotatef(90, 1.0f, 0.0f, 0.0f);
		glTranslatef(3.0f, 0.0f, 0.0f);
//		drawOBJ(2);
	glPopMatrix();

	//tree
/*	glPushMatrix();
		glTranslatef(1.0f, 0.0f, 3.0f);
		drawOBJ(3);
	glPopMatrix();

	//torus
	glPushMatrix();
		glTranslatef(0.0f, 0.0f, -3.0f);
		drawOBJ(2);
	glPopMatrix();*/

	//STRADA
	lightOff();
//	glColor4fv(roadColor);
	makeRoad(100.0f,100.0f,20.0f);
	lightOn();

	//ALBERO
	/*
     //AT  -> 2 6 0.2
     //EYE -> userCar.pos[0], 3.5f, userCar.pos[2] + 10.0f
     glPushMatrix();
         glColor4fv(treeColor);
         //glTranslatef(-2.0f, 3.0f, -4.0f);
         glTranslatef(0.0f, 3.0f, 0.0f);
         glRotatef(treeRot, 0.0f, 1.0f, 0.0f);
         drawCube(2.0f,6.0f,0.2f);
     glPopMatrix();
	 */

	//MACCHINA
	glPushMatrix();
//	glColor4fv(carColor);
	glTranslatef(userCar.pos[0], userCar.pos[1], userCar.pos[2]);
	glRotatef(userCar.rot, 0.0f, 1.0f, 0.0f);
	//drawCube(userCar.width,userCar.height,userCar.length);
//	glColor4fv(wheelColor);
	glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
	drawOBJ(0);
	glPushMatrix();
	glTranslatef(userWheel1.pos[0], userWheel1.pos[1], userWheel1.pos[2]);
	drawCube(userWheel1.width,userWheel1.height,userWheel1.length);
	glPopMatrix();
	glPushMatrix();
	glTranslatef(userWheel2.pos[0], userWheel2.pos[1], userWheel2.pos[2]);
	drawCube(userWheel2.width,userWheel2.height,userWheel2.length);
	glPopMatrix();
	glPushMatrix();
	glTranslatef(userWheel3.pos[0], userWheel3.pos[1], userWheel3.pos[2]);
	glRotatef(userWheel3.rot, 0.0f, 1.0f, 0.0f);
	drawCube(userWheel3.width,userWheel3.height,userWheel3.length);
	glPopMatrix();
	glPushMatrix();
	glTranslatef(userWheel4.pos[0], userWheel4.pos[1], userWheel4.pos[2]);
	glRotatef(userWheel4.rot, 0.0f, 1.0f, 0.0f);
	drawCube(userWheel4.width,userWheel4.height,userWheel4.length);
	glPopMatrix();
	glPopMatrix();

	if(blurActive==1)
	{
		glAccum(GL_MULT, 1.0f - blur);
		glAccum(GL_ACCUM,blur);
		glAccum(GL_RETURN, 1.0f);
	}
	//PARTICELLE
	//enable blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//draw particles
	for(p=root; p!=NULL; p=p->next)
	{
		if (p->lifetime < LIFETIME)
		{
			//cambia colore a seconda del tempo
			blue[0] = (float)((p->lifetime/LIFETIME));
			blue[1] = (float)((p->lifetime/LIFETIME));
			blue[2] = (float)(1.0f - (p->lifetime/LIFETIME));
			blue[3] = (float)(1.0f - (p->lifetime/LIFETIME));

			glPushMatrix();
/*			glColor4fv(blue);
			glTranslatef(2.0f,0.0f,0.0f);
			glTranslatef(p->sphere.pos[0], p->sphere.pos[1], p->sphere.pos[2]);*/
			glutSolidSphere(p->sphere.radius, 20, 20);
			glPopMatrix();
			glPushMatrix();
/*			glColor4fv(blue);
			glTranslatef(-2.0f,0.0f,0.0f);
			glTranslatef(p->sphere.pos[0], p->sphere.pos[1], p->sphere.pos[2]);
			glutSolidSphere(p->sphere.radius, 20, 20);*/
			glPopMatrix();
		}
	}
	glDisable(GL_BLEND);






	/*
     glDisable(GL_TEXTURE_2D);
     glOrtho(0, 100, 0, 100, -1, 1);
     glDisable(GL_DEPTH_TEST);
     glDepthMask(GL_FALSE);
     glColor4fv(wheelColor);
     renderBitmapString(100,100,GLUT_BITMAP_TIMES_ROMAN_24,stampe);
     glEnable(GL_TEXTURE_2D);
     glEnable(GL_DEPTH_TEST);
     glDepthMask(GL_TRUE);
	 */
	//zbuffer
	//glReadPixels(0.0,0.0,WIDTH,HEIGHT, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, pixels);
	//glDrawPixels(WIDTH,HEIGHT,GL_LUMINANCE, GL_UNSIGNED_BYTE, pixels);
	glutSwapBuffers();

	//finito il display riazzera i valori di rotazione e accelerazione
	if(userWheel3.rot<0.0f){
		userWheel3.rot +=1.0f;
		userWheel4.rot +=1.0f;
	}
	if(userWheel4.rot>0.0f){
		userWheel3.rot -=1.0f;
		userWheel4.rot -=1.0f;
	}
	userCar.throttle=0.0f;
}

void idle(void)
{
	//se è attivo il boost stacca la telecamera
	//1-boostaccumul da il grado di boost che diminuisce tra 1 e 0
	//moltiplicato per pigreco modella un seno su 0-180 x l'effetto
	//andata e ritorno
	if(blurActive==1)
	{
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glViewport(0, 0, WIDTH, HEIGHT);
		ar = (float)WIDTH/(float)HEIGHT;
		gluPerspective(45.0 + blurActive*sin((1-userCar.iperBoostAccumul)*M_PI)*3.0f, ar, 0.2f, 100.0f);
		glMatrixMode(GL_MODELVIEW);
	}

	int i,nop;
	particles *p;
	float ppi;
	float raggioCurvatura;
	float deltaTeta;
	struct timeval newTime;

	gettimeofday(&newTime, NULL);

	deltaT = (((double)(newTime.tv_sec) + (double)(newTime.tv_usec)/1000000.0) -
			((double)(old.tv_sec) + (double)(old.tv_usec)/1000000.0));

	ppi += deltaT;
	nop = (int)((float)PPS * (float)ppi);

	//PARTICELLE
	if(nop >= 1)
	{
		for (i=0; i<nop; i++)
		{
			addParticle(0.02f, &root);
		}
		ppi = 0.0f;
	}

	for(p=root; p!=NULL; p=p->next)
	{
		p->lifetime+=(float)deltaT;
	}

	removeFromLifetime(LIFETIME, &root);

	for(p=root; p!=NULL; p=p->next)
	{
		for(i=0;i<3;i++)
		{
			//MOTO UNIF ACC -> S = S' + v*t + 1/2*a*t^
			p->sphere.pos[i] = p->sphere.pos[i] + p->sphere.v[i]*deltaT;
			p->sphere.v[i] = p->sphere.v[i] + p->sphere.a[i]*deltaT;
		}
	}

	//IPERBOOST
	//carica il boost se non è attivo o già carico
	if(blurActive==0 && userCar.iperBoostAccumul<1.0f)
		userCar.iperBoostAccumul +=0.01;
	else
		sprintf(stampe2,"IPERBOOST ready! \n");
	//decrementa il boost se è attivo
	if(blurActive==1 && userCar.iperBoostAccumul>0.0f)
		userCar.iperBoostAccumul -=0.01;
	else
	{
		blurActive = 0;
	}
	//MACCHINA
	userCar.a[0] = userCar.throttle;
	userCar.v[0] = ((userCar.v[0] + userCar.a[0]*deltaT)+(userCar.v[0]))/2;

	// A = (VF-VO)/deltaT
	// A*deltaT = VF - VO
	// VF = VO + A*deltaT
	// VM = (VF+VO)/2

	float dist = userCar.pos[0];
	float dist2 = userCar.pos[2];
	userCar.pos[0] = userCar.pos[0] + userCar.v[0]*sin(userCar.rot*3.14f/180.0f)*deltaT;
	userCar.pos[2] = userCar.pos[2] + userCar.v[0]*cos(userCar.rot*3.14f/180.0f)*deltaT;
	userCar.rot -= (userWheel3.rot*userCar.v[0]*deltaT)/(5.0f+userCar.v[0]*deltaT);
	dist = dist - userCar.pos[0];
	dist2 = dist2 - userCar.pos[2];

	//raggioCurvatura = (userCar.v[0]*deltaT)/userWheel3.rot;
	//deltaTeta = (userCar.v[0]*deltaT*userWheel3.rot);
	//userCar.rot = userCar.rot + userCar.v[0]*deltaT/userCar.radius;

	//SPAZIO
	// VM = (SF-SO)/deltaT
	// VM*deltaT = SF-SO
	// SF = SO + VM*deltaT
	//userCar.pos[2] = userCar.pos[2] + userCar.v[0]*deltaT;

	//ALBERO
	//AT  -> 2 6 0.2
	//EYE -> userCar.pos[0], 3.5f, userCar.pos[2] + 10.0f
	float normal[3];
	float o2v[3];
	normal[0] = 0.0f;
	normal[1] = 0.0f;
	normal[2] = 1.0f;
	o2v[0] = userCar.pos[0];
	o2v[1] = 3.5f;
	o2v[2] = userCar.pos[2] + 10.0f;
	o2v[0] /= sqrt(scalProd(o2v,o2v));
	o2v[1] /= sqrt(scalProd(o2v,o2v));
	o2v[2] /= sqrt(scalProd(o2v,o2v));

	treeRot = 180.0f * acos(scalProd(o2v, normal)) / M_PI;

	old.tv_sec = newTime.tv_sec;
	old.tv_usec = newTime.tv_usec;

	//AGGIORNA VALORI DA STAMPARE A VIDEO
	sprintf(stampe,"Throttle: %f.\n Speed: %f.\n IperBoost: %f.\n",userCar.throttle,sqrt(userCar.v[0]*userCar.v[0]+userCar.v[2]*userCar.v[2]),userCar.iperBoostAccumul);

	glutPostRedisplay();
}

int main (int argc, char** argv)
{

	glutInit( &argc, argv );
	glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_ACCUM);

	glutInitWindowSize( WIDTH, HEIGHT );
	glutCreateWindow("Waterfall");

	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyb);


	init();

	glutMainLoop();
	return 0;
}

//ALPHA BLENDING
/*

mix(color1,color2,alpha) = color1

255 opaca
0 trasparente
source = da sovrapporre
destination = quella a cui sovrapporla
no zeta test ma alpha blending, mischiato con poligono dietro

es: staccionata -> rettangolo grande con texture con disegnati paletti staccionata
con alpha a zero nei 'buchi'

 */

//BILLBOARDING
/*

V vettore da centro oggetto a eye
n normale oggetto
V/||V|| X n = cos(teta) trovo teta

v = V/||V||
                          |i j k|
prodotto vett v x n = det |v v v| = A
                          |n n n|

a = A/||A|| vettore sul quale far ruotare

glRotatef(teta,a,a,a)

 */

//GLREADPIXELS
/*

glReadPixels(x,y,w,h,GLRGB,GL_U_BYTE,*data)

PIPELINE = FRAMEBUFFER(scheda video) -> (read) -> RAM(cpu) -> (rendering) -> FRAMEBUFFER

glDrawPixels scrive

 */
