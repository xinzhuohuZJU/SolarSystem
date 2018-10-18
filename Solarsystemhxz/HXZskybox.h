#pragma once

#include<gl/glew.h>
#include<iostream>
#include<vector>
#include<gl/freeglut.h>
#include<GL/glut.h>
#include<cstdlib>
#include<time.h>
#include<cstdio>
#include<Windows.h>
#include<string.h>

#define GL_CLAMP_TO_EDGE    0x812F

using namespace std;

//定义地面网格
const unsigned int MAP_WIDTH = 1024;
const unsigned int CELL_WIDTH = 16;
const unsigned int MAP = MAP_WIDTH * CELL_WIDTH / 2;

class SkyBox {
private:
	GLuint skyboxtexture[6];
	vector<string> skypath;
	GLint width;
	GLint height;
public:
	SkyBox();
	~SkyBox();

	unsigned int LoadTexture(string filepath);

	void DrawSkyBox();

	void MoveSkyBox(GLint x, GLint y, GLint z);
};

SkyBox::SkyBox() {

	//bmp图片的宽和高
	/*width = 512;
	height = 512;*/

	string front = "skybox/bluecloud_ft.bmp";
	string back = "skybox/bluecloud_bk.bmp";
	string left = "skybox/bluecloud_lf.bmp";
	string right = "skybox/bluecloud_rt.bmp";
	string up = "skybox/bluecloud_up.bmp";
	string down = "skybox/bluecloud_dn.bmp";

	skypath.push_back(front);
	skypath.push_back(back);
	skypath.push_back(left);
	skypath.push_back(right);
	skypath.push_back(up);
	skypath.push_back(down);
}

SkyBox::~SkyBox(){}

unsigned int SkyBox::LoadTexture(string filepath) {

	unsigned int Texture;

	static GLint ImageWidth;         //背景贴图宽度
	static GLint ImageHeight;       //背景贴图长度
	static GLint PixelLength;
	static GLubyte* PixelData;      //像素数据指针

	//读写方式打开背景贴图的bmp文件
	FILE* pFile = fopen(filepath.c_str(), "rb");

	if (pFile == 0)

	{
		cout << "FILE open error!" << endl;
		Sleep(3000);
		exit(0);

	}
	// 移动到0x0012位置
	fseek(pFile, 0x0012, SEEK_SET);
	//读取宽度
	fread(&ImageWidth, sizeof(ImageWidth), 1, pFile);
	//读取长度
	fread(&ImageHeight, sizeof(ImageHeight), 1, pFile);

	//每行数据长度
	PixelLength = ImageWidth * 3;

	//修正Length使其为4的倍数
	while (PixelLength % 4 != 0)

		++PixelLength;

	PixelLength *= ImageHeight;
	PixelData = (GLubyte*)malloc(PixelLength);
	if (PixelData == 0)

		exit(0);

	//读取文件头后的数据
	fseek(pFile, 54, SEEK_SET);
	fread(PixelData, PixelLength, 1, pFile);
	//关闭文件
	fclose(pFile);

	glGenTextures(1, &Texture);
	glBindTexture(GL_TEXTURE_2D, Texture);
	gluBuild2DMipmaps(GL_TEXTURE_2D, 3, ImageWidth, ImageHeight, GL_BGR_EXT, GL_UNSIGNED_BYTE, PixelData);

	//控制滤波，采用GL_CLAMP_TO_EDGE来处理，这就消除了接缝处的细线，增强了天空盒的真实感
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	if (PixelData)
		free(PixelData);

	return Texture;

}

void SkyBox::DrawSkyBox() {

	float x, y, z;
	float box_width, box_height, box_length;

	x = 0;
	y = 0;
	z = 0;

	box_width = 1.0;
	box_height = 0.5;
	box_length = 1.0;

	/** 计算天空盒长 宽 高 */
	float width = MAP * box_width / 8;
	float height = MAP * box_height / 8;
	float length = MAP * box_length / 8;

	/** 计算天空盒中心位置 */
	x = x + MAP / 8 - width / 2;
	y = y + MAP / 24 - height / 2;
	z = z + MAP / 8 - length / 2;

	glEnable(GL_TEXTURE_2D);

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_INTERPOLATE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_PREVIOUS);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB, GL_CONSTANT);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB, GL_SRC_COLOR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	glPushMatrix();
	//移动天空盒的中心至世界坐标系原点
	glTranslatef(-x-width/2, -y-height/2, -z-length/2);


	//front
	skyboxtexture[0] = LoadTexture(skypath[0]);
	glBindTexture(GL_TEXTURE_2D, skyboxtexture[0]);

	glPushMatrix();
	//glTranslatef(0.0f, 0.0f, width / 2);
	glBegin(GL_QUADS);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(x, y, z + length);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(x, y + height, z + length);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(x + width, y + height, z + length);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(x + width, y, z + length);
	glEnd();
	glLoadIdentity();
	glPopMatrix();

	//back
	skyboxtexture[1] = LoadTexture(skypath[1]);
	glBindTexture(GL_TEXTURE_2D, skyboxtexture[1]);

	glPushMatrix();
	//glTranslatef(0.0f, 0.0f, -width / 2);
	glBegin(GL_QUADS);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(x + width, y, z);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(x + width, y + height, z);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(x, y + height, z);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(x, y, z);
	glEnd();
	glLoadIdentity();
	glPopMatrix();

	//left
	skyboxtexture[2] = LoadTexture(skypath[2]);
	glBindTexture(GL_TEXTURE_2D, skyboxtexture[2]);

	glPushMatrix();
	//glTranslatef(width / 2, 0.0f, 0.0f);
	glBegin(GL_QUADS);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(x, y + height, z);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(x, y + height, z + length);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(x, y, z + length);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(x, y, z);
	glEnd();
	glLoadIdentity();
	glPopMatrix();

	//right
	skyboxtexture[3] = LoadTexture(skypath[3]);
	glBindTexture(GL_TEXTURE_2D, skyboxtexture[3]);

	glPushMatrix();
	//glTranslatef(-width / 2, 0.0f, 0.0f);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(x + width, y, z);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(x + width, y, z + length);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(x + width, y + height, z + length);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(x + width, y + height, z);
	glEnd();
	glLoadIdentity();
	glPopMatrix();

	//up
	skyboxtexture[4] = LoadTexture(skypath[4]);
	glBindTexture(GL_TEXTURE_2D, skyboxtexture[4]);

	glPushMatrix();
	//glTranslatef(0.0f, width / 2, 0.0f);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(x + width, y + height, z);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(x + width, y + height, z + length);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(x, y + height, z + length);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(x, y + height, z);
	glEnd();
	glPopMatrix();
	

	//down
	skyboxtexture[5] = LoadTexture(skypath[5]);
	glBindTexture(GL_TEXTURE_2D, skyboxtexture[5]);

	glPushMatrix();
	//glTranslatef(0.0f, -width / 2, 0.0f);
	glBegin(GL_QUADS);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(x, y, z);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(x, y, z + length);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(x + width, y, z + length);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(x + width, y, z);
	glEnd();
	glPopMatrix();

	glPopMatrix();
	glDisable(GL_TEXTURE_2D);


}

void SkyBox::MoveSkyBox(GLint x, GLint y, GLint z) {
	glPushMatrix();
	glTranslatef(x, y, z);
	DrawSkyBox();
	glPopMatrix();
}