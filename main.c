#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <math.h>
#include <GLUT/glut.h>

#define WIDTH 800
#define HEIGHT 600

#define PPS 30
#define LIFETIME 2.0f
#define V 8.0f

typedef struct
{
	float radius;
	float pos[3];
	float v[3];
	float acc[3];
	float mass;
	float abs;
} mySphere;

struct _particles
{
	mySphere sphere;
	float lifetime;
	struct _particles *next;
};

typedef struct _particles particles;

float rot=0.0f, translateX=0.0f, translateZ=0.0f;
float myColor[] = {1.0, 1.0, 1.0, 1.0};
double deltaT = 0.0f, particleGenerator = 0.0f;
mySphere sphere;
struct timeval old;
particles *root;


void addParticle()
{
	particles *p;
	float d = 1.0f;

	p = malloc(sizeof(particles));

	p->sphere.radius = 0.05f;

	p->sphere.pos[0] = 0.0f;
	p->sphere.pos[1] = 1.0f;
	p->sphere.pos[2] = 0.0f;

	p->sphere.v[0] = ((float)rand() / (float)RAND_MAX) * d - d * 0.5f ;
	p->sphere.v[1] = V;
	p->sphere.v[2] = ((float)rand() / (float)RAND_MAX) * d - d * 0.5f;

	p->sphere.acc[0] = 0.0f;
	p->sphere.acc[1] = -9.81f;
	p->sphere.acc[2] = 0.0f;

	p->sphere.mass = 1.0f;
	p->sphere.abs = 0.1f;

	p->lifetime = 0.0f;

	p->next = root;
	root = p;
}

void drawCube(float size)
{
	float c = size*0.5f;
	glBegin(GL_QUADS);
		//faccia in alto
		glNormal3f(0.0f, 1.0f, 0.0f);
		glVertex3f(-c, c,-c);
		glVertex3f(-c, c, c);
		glVertex3f( c, c, c);
		glVertex3f( c, c,-c);

		//faccia in basso
		glNormal3f( 0.0f,-1.0f, 0.0f);
		glVertex3f(-c,-c,-c);
		glVertex3f( c,-c,-c);
		glVertex3f( c,-c, c);
		glVertex3f(-c,-c, c);

		//faccia davanti
		glNormal3f( 0.0f, 0.0f, 1.0f);
		glVertex3f(-c, c, c);
		glVertex3f( c, c, c);
		glVertex3f( c,-c, c);
		glVertex3f(-c,-c, c);

		//faccia dietro
		glNormal3f( 0.0f, 0.0f,-1.0f);
		glVertex3f(-c, c,-c);
		glVertex3f(-c,-c,-c);
		glVertex3f( c,-c,-c);
		glVertex3f( c, c,-c);

		//faccia destra
		glNormal3f( 1.0f, 0.0f, 0.0f);
		glVertex3f( c, c, c);
		glVertex3f( c,-c, c);
		glVertex3f( c,-c,-c);
		glVertex3f( c, c,-c);

		//faccia sinistra
		glNormal3f(-1.0f, 0.0f, 0.0f);
		glVertex3f(-c, c,-c);
		glVertex3f(-c,-c,-c);
		glVertex3f(-c,-c, c);
		glVertex3f(-c, c, c);
	glEnd();
}


void init(void)
{
	float ambient[] = {1.0f, 1.0f, 1.0f};
	float diffuse[] = {1.0f, 1.0f, 1.0f};
	float specular[] = {1.0f, 1.0f, 1.0f};
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

	glColorMaterial(GL_FRONT, GL_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);

	glEnable(GL_DEPTH_TEST);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	root = NULL;

	gettimeofday(&old, NULL);
	srand(old.tv_sec);
	return;
}

void display ( void )
{
	particles *p;
	float light0Position[] = {0.0f, 2.0f, 5.0f, 1.0f};
	float cubeMAmbient[] = {0.1f, 0.1f, 0.1f};
	float cubeMDiffuse[] = {0.5f, 0.5f, 0.5f};
	float cubeMSpecular[] = {0.5f, 0.5f, 0.5f};
	float cubeMShininess = 9.0f;

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	gluLookAt( 0.0f, 3.5f, 10.0f, //Eye
		   0.0f, 3.0f, 0.0f, //At
		   0.0f, 1.0f, 0.0f);//Up

	glLightfv(GL_LIGHT0, GL_POSITION, light0Position);

	glMaterialfv(GL_FRONT, GL_AMBIENT, cubeMAmbient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, cubeMDiffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, cubeMSpecular);
	glMaterialf(GL_FRONT, GL_SHININESS, cubeMShininess);

	glColor4fv(myColor);

	glPushMatrix();
		glTranslatef(0.0f, 0.5f, 0.0f);
		drawCube(1.0f);
	glPopMatrix();

	glTranslatef(translateX, 0.0f, translateZ);

	for(p=root; p!=NULL; p=p->next)
	{
		if (p->lifetime < LIFETIME)
		{
			glPushMatrix();
				glTranslatef(p->sphere.pos[0], p->sphere.pos[1], p->sphere.pos[2]);
				glutSolidSphere(p->sphere.radius, 20, 20);
			glPopMatrix();
		}
	}
	glColor3f(0.8f, 0.8f, 0.8f);
	glBegin(GL_QUADS);
		glNormal3f(0.0f, 1.0f, 0.0f);
		glVertex3f(-10.0f, 0.0f, -10.0f);
		glVertex3f(-10.0f, 0.0f,  10.0f);
		glVertex3f( 10.0f, 0.0f,  10.0f);
		glVertex3f( 10.0f, 0.0f, -10.0f);
	glEnd();

//	glPushMatrix();
//		glTranslatef(translateX, 0.5f, translateZ);
//		drawCube(1.0f);
//	glPopMatrix();

	glutSwapBuffers();
	return;
}

void reshape ( int w, int h)
{
	float ar;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glViewport(0, 0, w, h);
	ar = (float)w/(float)h;
	gluPerspective(45.0, ar, 0.2f, 100.0f);

	glMatrixMode(GL_MODELVIEW);
}

void keyb(unsigned char key, int x, int y)
{
	switch (key)
	{
		case 27:	exit(0);
				break;

		case 'r':	myColor[0]=1.0f;
				myColor[1]=0.0f;
				myColor[2]=0.0f;
			 	break;
		case 'g':	myColor[0]=0.0f;
				myColor[1]=1.0f;
				myColor[2]=0.0f;
			 	break;
		case 'b':	myColor[0]=0.0f;
				myColor[1]=0.0f;
				myColor[2]=1.0f;
			 	break;

		case 'w':	translateZ +=0.1;
				break;
		case 'd':	translateX -=0.1;
				break;
		case 's':	translateZ -=0.1;
				break;
		case 'a':	translateX +=0.1;
				break;

		case 'p':	glShadeModel(GL_FLAT);
				break;
		case 'P':	glShadeModel(GL_SMOOTH);
				break;


		default:	break;
	}
	glutPostRedisplay();
}

void idle(void)
{
	int i, nofp;
	struct timeval newTime;
	particles *p;

	gettimeofday(&newTime, NULL);

	deltaT = (((double)(newTime.tv_sec) + (double)(newTime.tv_usec)/1000000.0) -
		  ((double)(old.tv_sec) + (double)(old.tv_usec)/1000000.0));

// 	printf("%f\n", deltaT);

	particleGenerator += deltaT;

	nofp = (int)((float)PPS * (float)particleGenerator);

	if(nofp >= 1)
	{
		for (i=0; i<nofp; i++)
		{
			addParticle();
		}
		particleGenerator = 0.0f;
	}



	for(p=root; p!=NULL; p=p->next)
	{
		p->lifetime+=(float)deltaT;
	}

	//RIMUOVERE PARTICELLE MORTE  --> TODO

	for(p=root; p!=NULL; p=p->next)
	{
		//MOTORE COLLISIONI
		if ((p->sphere.pos[1] < p->sphere.radius) && (p->sphere.v[1]<0))
		{
			float e;
			e = p->sphere.mass * p->sphere.v[1] * p->sphere.v[1] * 0.5f;
			e = e - (e * p->sphere.abs);
			p->sphere.v[1] = sqrt(2.0f * e / p->sphere.mass);
		}

		//MOTORE FISICO
		for(i=0; i<3; i++)
		{
			p->sphere.pos[i] = p->sphere.pos[i] + p->sphere.v[i] * (float)deltaT;
			p->sphere.v[i] = p->sphere.v[i] + p->sphere.acc[i] * (float)deltaT;
	// 		printf("%f ", sphere.pos[i]);
		}

	// 	printf("\n");
	}


	old.tv_sec = newTime.tv_sec;
	old.tv_usec = newTime.tv_usec;
	glutPostRedisplay();
}

int main (int argc, char** argv)
{
	glutInit( &argc, argv );
	glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );

	glutInitWindowSize( WIDTH, HEIGHT );
	glutCreateWindow("Glut_1");

	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyb);

	init();

	glutMainLoop();
	return 0;
}
