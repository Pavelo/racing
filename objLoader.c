#include <GLUT/glut.h>
#include <stdio.h>
#include <stdlib.h>


typedef struct
{

	float x;
	float y;
	float z;

} Vertex;

typedef struct
{

	float u;
	float v;

} VTexture;

typedef struct
{

	float x;
	float y;
	float z;

} VNormal;

typedef struct
{

	int v1;
	int t1;
	int n1;

	int v2;
	int t2;
	int n2;

	int v3;
	int t3;
	int n3;

	int v4;
	int t4;
	int n4;

} Face;

typedef struct
{

	int v1;
	int t1;
	int n1;

	int v2;
	int t2;
	int n2;

	int v3;
	int t3;
	int n3;

} Face3;


Vertex v[10][50000];
VTexture vt[10][50000];
VNormal vn[10][50000];
Face f[10][50000];
Face3 f3[10][50000];

int vCount;
int vtCount;
int vnCount;
int fCount;
int f3Count;

float ambient[10][3];
float diffuse[10][3];
float specular[10][3];
float shininess[10];
char textureName[10][32];
char texturePath[32];

int loadMTL(char* path, int id)
{
	int loaded;
	char line[100];

	// inizializzo le stringhe
	int i;
	for (i=0; i<32; i++)
	{
		texturePath[i] = '\0';
	}

	FILE *fp = fopen(path,"r");

	if (fp != NULL)
	{
		while (fgets(line, 99, fp))
		{
			if (strstr(line, "map_Kd") != NULL)
			{
				strncpy(textureName[id], line + 7, strlen(line)-8);
				printf(" [%s ", textureName[id]);
				sprintf(texturePath, "texture/%s", textureName[id]);
				printf("%d]", loadTGA(texturePath, id));

				// reinizializzo le stringhe
				for (i=0; i<32; i++)
				{
					texturePath[i] = '\0';
				}
			}
			else if (strstr(line, "Kd") != NULL)
			{
				sscanf(line, "%*c%*c %f %f %f", &diffuse[id][0], &diffuse[id][1], &diffuse[id][2]);
			}
			else if (strstr(line, "Ka") != NULL)
			{
				sscanf(line, "%*c%*c %f %f %f", &ambient[id][0], &ambient[id][1], &ambient[id][2]);
			}
			else if (strstr(line, "Ks") != NULL)
			{
				sscanf(line, "%*c%*c %f %f %f", &specular[id][0], &specular[id][1], &specular[id][2]);
			}
			else if (strstr(line, "Ns") != NULL)
			{
				sscanf(line, "%*c%*c %f", &shininess[id]);
			}
		}
		loaded = 1;
	}
	else
	{
		loaded = 0;
	}

	fclose(fp);

	return loaded;
}

int loadOBJ(char* path, int id)
{
	int loaded;
	char line[100];

	vCount = 0;
	vtCount = 0;
	vnCount = 0;
	fCount = 0;
	f3Count = 0;

	FILE *fp = fopen(path,"r");

	char mtllibName[32];
	char mtllibPath[32];

	// inizializzo le stringhe
	int i;
	for (i=0; i<32; i++)
	{
		mtllibName[i] = '\0';
		mtllibPath[i] = '\0';
	}

	if (fp != NULL)
	{
		while (fgets(line, 99, fp))
		{
			if (line[0] == 'v')
			{
				// texture vertex
				if (line[1] == 't')
				{
					sscanf(line, "%*c%*c %f %f", &vt[id][vtCount].u, &vt[id][vtCount].v);
					vtCount++;
				}
				// normal vertex
				else if (line[1] == 'n')
				{
					sscanf(line, "%*c%*c %f %f %f", &vn[id][vnCount].x, &vn[id][vnCount].y, &vn[id][vnCount].z);
					vnCount++;
				}
				// vertex
				else
				{
					sscanf(line, "%*c %f %f %f", &v[id][vCount].x, &v[id][vCount].y, &v[id][vCount].z);
					vCount++;
				}
			}
			// face
			else if (line[0] == 'f')
			{
				int n = 0;
				int spaceCount = 0;
				while (line[n] != '\0')
				{
					if (line[n]==' ')
						spaceCount++;
					n++;
				}
				if (spaceCount == 3)
				{
					sscanf(line, "%*c %d/%d/%d %d/%d/%d %d/%d/%d",
							&f3[id][f3Count].v1, &f3[id][f3Count].t1, &f3[id][f3Count].n1,
							&f3[id][f3Count].v2, &f3[id][f3Count].t2, &f3[id][f3Count].n2,
							&f3[id][f3Count].v3, &f3[id][f3Count].t3, &f3[id][f3Count].n3);
					f3Count++;
				}
				else if (spaceCount == 4)
				{
					sscanf(line, "%*c %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d",
							&f[id][fCount].v1, &f[id][fCount].t1, &f[id][fCount].n1,
							&f[id][fCount].v2, &f[id][fCount].t2, &f[id][fCount].n2,
							&f[id][fCount].v3, &f[id][fCount].t3, &f[id][fCount].n3,
							&f[id][fCount].v4, &f[id][fCount].t4, &f[id][fCount].n4);
					fCount++;
				}
			}
			else if (strstr(line, "mtllib") != NULL)
			{
				strncpy(mtllibName, line+7, strlen(line)-8);
				printf(mtllibName);
				sprintf(mtllibPath, "obj/%s", mtllibName);
				printf(" %d\n",loadMTL(mtllibPath, id));

				// reinizializzo le stringhe
				for (i=0; i<32; i++)
				{
					mtllibName[i] = '\0';
					mtllibPath[i] = '\0';
				}
			}
			//        	printf("v %f %f %f\n", v[id][vCount].x, v[id][vCount].y, v[id][vCount].z);
			//        	printf("%f %f %f\n", vn[id][f[id][fCount].n1-1].x, vn[id][f[id][fCount].n1-1].y, vn[id][f[id][fCount].n1-1].z);
		}
		loaded = 1;
	}
	else
	{
		loaded = 0;
	}

	fclose(fp);


	return loaded;
}

void drawOBJ(int id)
{
	//	printf("ambient %f %f %f\n", ambient[id][0], ambient[id][1], ambient[id][2]);
	//	printf("diffuse %f %f %f\n", diffuse[id][0], diffuse[id][1], diffuse[id][2]);
	//	printf("specular %f %f %f\n", specular[id][0], specular[id][1], specular[id][2]);
	//	printf("shininess %f\n", shininess[id]);

	glMaterialfv(GL_FRONT, GL_AMBIENT, ambient[id]);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse[id]);
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular[id]);
	glMaterialf(GL_FRONT, GL_SHININESS, shininess[id]);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, id);

	glBegin(GL_QUADS);

	int i;
	for (i=0; i<fCount; i++)
	{
		glNormal3f(vn[id][f[id][i].n1-1].x, vn[id][f[id][i].n1-1].y, vn[id][f[id][i].n1-1].z);
		glTexCoord2f(vt[id][f[id][i].t1-1].u, vt[id][f[id][i].t1-1].v);
		glVertex3f(v[id][f[id][i].v1-1].x, v[id][f[id][i].v1-1].y, v[id][f[id][i].v1-1].z);

		glNormal3f(vn[id][f[id][i].n2-1].x, vn[id][f[id][i].n2-1].y, vn[id][f[id][i].n2-1].z);
		glTexCoord2f(vt[id][f[id][i].t2-1].u, vt[id][f[id][i].t2-1].v);
		glVertex3f(v[id][f[id][i].v2-1].x, v[id][f[id][i].v2-1].y, v[id][f[id][i].v2-1].z);

		glNormal3f(vn[id][f[id][i].n3-1].x, vn[id][f[id][i].n3-1].y, vn[id][f[id][i].n3-1].z);
		glTexCoord2f(vt[id][f[id][i].t3-1].u, vt[id][f[id][i].t3-1].v);
		glVertex3f(v[id][f[id][i].v3-1].x, v[id][f[id][i].v3-1].y, v[id][f[id][i].v3-1].z);

		glNormal3f(vn[id][f[id][i].n4-1].x, vn[id][f[id][i].n4-1].y, vn[id][f[id][i].n4-1].z);
		glTexCoord2f(vt[id][f[id][i].t4-1].u, vt[id][f[id][i].t4-1].v);
		glVertex3f(v[id][f[id][i].v4-1].x, v[id][f[id][i].v4-1].y, v[id][f[id][i].v4-1].z);

		/*printf("v1 %f %f %f\n", v[id][f[id][i].v1-1].x, v[id][f[id][i].v1-1].y, v[id][f[id][i].v1-1].z);
            printf("v2 %f %f %f\n", v[id][f[id][i].v2-1].x, v[id][f[id][i].v2-1].y, v[id][f[id][i].v2-1].z);
            printf("v3 %f %f %f\n", v[id][f[id][i].v3-1].x, v[id][f[id][i].v3-1].y, v[id][f[id][i].v3-1].z);
            printf("v4 %f %f %f\n", v[id][f[id][i].v4-1].x, v[id][f[id][i].v4-1].y, v[id][f[id][i].v4-1].z);

            printf("vn1 %f %f %f\n", vn[id][f[id][i].n1-1].x, vn[id][f[id][i].n1-1].y, vn[id][f[id][i].n1-1].z);
            printf("vn2 %f %f %f\n", vn[id][f[id][i].n2-1].x, vn[id][f[id][i].n2-1].y, vn[id][f[id][i].n2-1].z);
            printf("vn3 %f %f %f\n", vn[id][f[id][i].n3-1].x, vn[id][f[id][i].n3-1].y, vn[id][f[id][i].n3-1].z);
            printf("vn4 %f %f %f\n", vn[id][f[id][i].n4-1].x, vn[id][f[id][i].n4-1].y, vn[id][f[id][i].n4-1].z);*/
	}

	glEnd();

	glBegin(GL_TRIANGLES);

	for (i=0; i<f3Count; i++)
	{
		glNormal3f(vn[id][f3[id][i].n1-1].x, vn[id][f3[id][i].n1-1].y, vn[id][f3[id][i].n1-1].z);
		glTexCoord2f(vt[id][f3[id][i].t1-1].u, vt[id][f3[id][i].t1-1].v);
		glVertex3f(v[id][f3[id][i].v1-1].x, v[id][f3[id][i].v1-1].y, v[id][f3[id][i].v1-1].z);

		glNormal3f(vn[id][f3[id][i].n2-1].x, vn[id][f3[id][i].n2-1].y, vn[id][f3[id][i].n2-1].z);
		glTexCoord2f(vt[id][f3[id][i].t2-1].u, vt[id][f3[id][i].t2-1].v);
		glVertex3f(v[id][f3[id][i].v2-1].x, v[id][f3[id][i].v2-1].y, v[id][f3[id][i].v2-1].z);

		glNormal3f(vn[id][f3[id][i].n3-1].x, vn[id][f3[id][i].n3-1].y, vn[id][f3[id][i].n3-1].z);
		glTexCoord2f(vt[id][f3[id][i].t3-1].u, vt[id][f3[id][i].t3-1].v);
		glVertex3f(v[id][f3[id][i].v3-1].x, v[id][f3[id][i].v3-1].y, v[id][f3[id][i].v3-1].z);

		/*            printf("v1 %f %f %f\n", v[id][f[id][i].v1-1].x, v[id][f[id][i].v1-1].y, v[id][f[id][i].v1-1].z);
            printf("v2 %f %f %f\n", v[id][f[id][i].v2-1].x, v[id][f[id][i].v2-1].y, v[id][f[id][i].v2-1].z);
            printf("v3 %f %f %f\n", v[id][f[id][i].v3-1].x, v[id][f[id][i].v3-1].y, v[id][f[id][i].v3-1].z);

            printf("vn1 %f %f %f\n", vn[id][f[id][i].n1-1].x, vn[id][f[id][i].n1-1].y, vn[id][f[id][i].n1-1].z);
            printf("vn2 %f %f %f\n", vn[id][f[id][i].n2-1].x, vn[id][f[id][i].n2-1].y, vn[id][f[id][i].n2-1].z);
            printf("vn3 %f %f %f\n", vn[id][f[id][i].n3-1].x, vn[id][f[id][i].n3-1].y, vn[id][f[id][i].n3-1].z);*/
	}

	glEnd();

	glDisable(GL_TEXTURE_2D);
}
