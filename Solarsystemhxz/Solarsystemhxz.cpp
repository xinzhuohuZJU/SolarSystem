// Solarsystemhxz.cpp: 定义控制台应用程序的入口点。
//

#include"HXZskybox.h"
#include"SOIL.h";                 //include了opengl简易图像库的头文件

#pragma comment(lib,"SOIL.lib")  //opengl简易图像库的动态链接库


//#define FileName "universe.bmp"   //背景贴图文件名
//static GLint ImageWidth;         //背景贴图宽度
//static GLint ImageHeight;       //背景贴图长度
//static GLint PixelLength;        
//static GLubyte* PixelData;      //像素数据指针

using namespace std;


GLint winWidth = 1000, winHeight = 1000;

const GLfloat PI = 3.141595654f;
GLdouble eyex = 0, eyey = 0.0, eyez = 300;              //相机在世界坐标的位置
GLdouble xref = 0.0, yref = 0.0, zref = 0.0;                //相机镜头对准的位置
GLdouble Vx = 0.0, Vy = 1.0, Vz = 0.0;                      //相机向上的方向在世界坐标的位置

GLint skybox_x = 0, skybox_y = 0, skybox_z = 0;

static GLfloat earthAngle = 0.0;             //地球旋转角
static GLfloat moonAngle = 0.0;              //月球旋转角
static GLfloat JupiterAngle = 0.0;           //木星旋转角
static GLfloat MarsAngle = 0.0;              //火星旋转角

static GLfloat earthstep = 12;               //地球旋转步长
static GLfloat moonstep = 60;                //月球旋转步长
static GLfloat jupiterstep = 3.6;            //木星旋转步长
static GLfloat Marsstep = 8.8;               //火星旋转步长  

//纹理部分
int sunHeight, sunWidth;    //纹理贴图的长与宽
int earthHeight, earthWidth;
int moonHeight, moonWidth;
int marsHeight, marsWidth;
int jupiterHeight, jupiterWidth;

GLuint texture_sun;             //纹理ID
GLuint texture_earth;
GLuint texture_moon;
GLuint texture_mars;
GLuint texture_jupiter;

GLUquadricObj *sunquadPlanet;      //初始化二次曲面并创建一个指向二次曲面的指针
GLUquadricObj *earthquadPlanet;
GLUquadricObj *moonquadPlanet;
GLUquadricObj *marsquadPlanet;
GLUquadricObj *jupiterquadPlanet;



//该solar system的光照模型采用Phong模型

//定义绘制行星公转轨道的函数,输入参数为轨道半径,以及垂直轨道平面的法向量n
void DrawOrbit(float r, float xn, float yn, float zn) {       

	int n = 3600;
	float xa, ya, za;       //空间向量a
	float xb, yb, zb;       //空间向量b
	float moda, modb;       //向量a,b的模长
	float Module(float x, float y, float z);        //求向量模长的函数，输入为向量的坐标值
	
	//n与(1,0,0)叉乘，求取向量a
	xa = 0;
	ya = zn;
	za = - yn;
	//如果a为零向量，n与(0,1,0)叉乘
	if (zn==0&&yn==0)
	{
		xa = -zn;
		ya = 0;
		za = xn;

	}
	//向量n与a叉乘求取向量b
	xb = yn * za - ya * zn;
	yb = zn * xa - za * xn;
	zb = xn * ya - yn * xa;
	//求取向量a,b的模长
	moda = Module(xa, ya, za);
	modb = Module(xb, yb, zb);
	//单位化a,b向量
	xa /= moda;
	ya /= moda;
	za /= moda;

	xb /= modb;
	yb /= modb;
	zb /= modb;
	//根据圆的参数方程绘制圆轨道
	glPushMatrix();
	glColor3f(1.0, 1.0, 1.0);
	glBegin(GL_LINE_LOOP);
	//设置轨道的材质
	{
		GLfloat orbit_mat_diffuse[] = { 0.0,0.0,0.0,1.0 };            //定义轨道的材质的漫反射和环境光颜色为0
		GLfloat orbit_mat_specular[] = { 0.0,0.0,0.0,1.0 };           //定义轨道的材质的镜面反射的颜色为0
		GLfloat orbit_mat_emission[] = { 0.7,0.7,0.7,1.0 };           //定义轨道的材质的辐射颜色为灰色
		GLfloat orbit_mat_shininess = 0.0f;                           //高光系数为0

		glMaterialfv(GL_FRONT, GL_EMISSION, orbit_mat_emission);
		glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, orbit_mat_diffuse);     //同时设定漫反射与环境光
		glMaterialfv(GL_FRONT, GL_SPECULAR, orbit_mat_specular);
		glMaterialf(GL_FRONT, GL_SHININESS, orbit_mat_shininess);
	}
	for (int i = 0; i < n; i++)
	{
		glVertex3d(r*cos(2*PI*i/n)*xa+r*sin(2*PI*i/n)*xb, r*cos(2 * PI*i / n)*ya + r * sin(2 * PI*i / n)*yb, r*cos(2 * PI*i / n)*za + r * sin(2 * PI*i / n)*zb);
	}
	glEnd();
	glPopMatrix();

}

//求向量模长
float Module(float x, float y, float z) {

	float mod;
	mod = sqrtf(x*x + y * y + z * z);

	return mod;
}

void display(void) {
	
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glDisable(GL_DEPTH_TEST);          //关闭深度测试
	//
	//glDrawPixels(ImageWidth, ImageHeight, GL_RGB, GL_UNSIGNED_BYTE, PixelData);    //绘制背景贴图
	//glEnable(GL_DEPTH_TEST);           //开启深度测试
		
	{
		GLfloat sun_light[] = { 0.0,0.0,0.0,1.0 };         //光源放置在世界坐标系中心,点光源没有衰减
		GLfloat sun_light_ambient[] = { 0.0,0.0,0.0,1.0 };  //没有环境光
		GLfloat sun_light_diffuse[] = { 1.0,1.0,1.0,1.0 }; //漫反射光为白色
		GLfloat sun_light_specular[] = { 1.0,1.0,1.0,1.0, };//镜面反射光为白色
		GLfloat globalAmbient[] = { 0.0,0.0,0.3,1.0 };         //背景光为低强度，暗蓝色

		glLightfv(GL_LIGHT0, GL_POSITION, sun_light);
		glLightfv(GL_LIGHT0, GL_AMBIENT, sun_light_ambient);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, sun_light_diffuse);
		glLightfv(GL_LIGHT0, GL_SPECULAR, sun_light_specular);
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);

		glEnable(GL_LIGHT0);
		glEnable(GL_LIGHTING);
		glEnable(GL_DEPTH_TEST);
	}
	

	//绘制天空盒
	{
		SkyBox HXZSkybox;
		glPushMatrix();
		HXZSkybox.MoveSkyBox(skybox_x, skybox_y, skybox_z);
		glPopMatrix();
	}

	glPushMatrix();

		
	{
		GLfloat sun_mat_diffuse[] = { 0.0,0.0,0.0,1.0 };           //定义太阳的材质的漫反射颜色为0
		GLfloat sun_mat_specular[] = { 0.0,0.0,0.0,1.0 };          //定义太阳的材质的镜面反射颜色为0 
		GLfloat sun_mat_emission[] = { 1.0,0.13,0.13,1.0 };        //设定太阳的材质的辐射颜色为红色
		GLfloat sun_mat_shininess = 0.0f;

		glMaterialfv(GL_FRONT, GL_EMISSION, sun_mat_emission);
		glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, sun_mat_diffuse);
		glMaterialfv(GL_FRONT, GL_SPECULAR, sun_mat_specular);
		glMaterialf(GL_FRONT, GL_SHININESS, sun_mat_shininess);
	}

	{
		unsigned char *sun = SOIL_load_image("texture/sun.bmp", &sunWidth, &sunHeight, 0, SOIL_LOAD_RGB);     //使用SOIL库的载入纹理图片的函数
		glGenTextures(1, &texture_sun);   //生成一个纹理对象
		glBindTexture(GL_TEXTURE_2D, texture_sun);  //绑定该纹理对象
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, sunWidth, sunHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, sun); //生成太阳的纹理
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		SOIL_free_image_data(sun);        //释放图像缓存
		sunquadPlanet = gluNewQuadric();  //返回一个指向二次曲面的指针;
				
	}
	{
		glEnable(GL_TEXTURE_2D);         //防止纹理只显示一次就被冲掉，开启2D纹理
		glGenerateMipmap(GL_TEXTURE_2D); //为target相关联的纹理生成一组完整的mipmap
		gluQuadricTexture(sunquadPlanet, GLU_TRUE); //设置自动计算纹理

		gluSphere(sunquadPlanet, 40, 1000, 1000);          //绘制太阳
	    //glutWireSphere(40, 1000.0, 1000.0); 
		gluDeleteQuadric(sunquadPlanet);   //释放指针
		glDisable(GL_TEXTURE_2D);     //关闭2D纹理
	}
	

	
	DrawOrbit(80, 0, 1, 0);       //绘制地球轨道
	glRotatef(earthAngle, 0.0, 1.0, 0.0);  //设置地球旋转的轨道平面

	glPushMatrix();
	glTranslatef(80, 0.0, 0.0);       //移动地球到相应位置
	
	{
		GLfloat earth_mat_ambient[] = { 0.0,0.0,0.5,1.0 };     //设定地球的材质的环境光颜色为偏蓝色
		GLfloat earth_mat_diffuse[] = { 0.0,0.0,1.0,1.0 };     //设定地球的材质的漫反射光为蓝色
		GLfloat earth_mat_specular[] = { 0.3,0.0,0.0,1.0 };    //设定地球的材质的镜面反射光为暗红色
		GLfloat earth_mat_emission[] = { 0.0,0.0,0.0,1.0 };    //地球不产生向外辐射光
		GLfloat earth_mat_shininess = 30.0f;

		glMaterialfv(GL_FRONT, GL_EMISSION, earth_mat_emission);
		glMaterialfv(GL_FRONT, GL_AMBIENT, earth_mat_ambient);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, earth_mat_diffuse);
		glMaterialfv(GL_FRONT, GL_SPECULAR, earth_mat_specular);
		glMaterialf(GL_FRONT, GL_SHININESS, earth_mat_shininess);
				
	
	}

	{
		//glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD);
		//glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		static GLfloat constColor[4] = { 0.0, 0.0, 0.0, 0.0 };
		constColor[3] = 0.4;
		glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, constColor);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
		//插值模式的混合函数
		glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_INTERPOLATE_ARB);
		//颜色来源于当前纹理
		glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_TEXTURE);
		glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, GL_SRC_COLOR);
		//颜色来源于当前片段（之前光照设置材质时的值）
		glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_PREVIOUS_ARB);
		glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND1_RGB_ARB, GL_SRC_COLOR);
		//使用GL_TEXTURE_ENV_COLOR设置的颜色值，即上文中的（0.4,0.4,0.4)
		glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE2_RGB_ARB, GL_CONSTANT_ARB);
		glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND2_RGB_ARB, GL_SRC_ALPHA);
		//上述混合操作的含义是:texColor*(0.4, 0.4, 0.4) + fragmentColor*(0.6, 0.6, 0.6);
	}
	
	
	{

		unsigned char *earth = SOIL_load_image("texture/earth.bmp", &earthWidth, &earthHeight, 0, SOIL_LOAD_RGB);
		glGenTextures(1, &texture_earth);
		glBindTexture(GL_TEXTURE_2D, texture_earth);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, earthWidth, earthHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, earth);//生成地球的纹理
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		SOIL_free_image_data(earth);
		earthquadPlanet = gluNewQuadric();

	}

	{
		glEnable(GL_TEXTURE_2D);
		glGenerateMipmap(GL_TEXTURE_2D); //为target相关联的纹理生成一组完整的mipmap
		gluQuadricTexture(earthquadPlanet, GLU_TRUE); //设置自动计算纹理
		gluSphere(earthquadPlanet, 10, 30, 30);
		//glutSolidSphere(10, 30, 30); //绘制地球
		gluDeleteQuadric(earthquadPlanet);

		glDisable(GL_TEXTURE_2D);
	}

	
	DrawOrbit(14.14213562, 0, 0, 1);
	glRotatef(moonAngle, 0.0, 0.0, 1.0);

	glPushMatrix();
	glTranslated(10.0, 10.0, 0.0);
	
	{
		GLfloat moon_mat_ambient[] = { 0.37,0.36,0.42,1.0 };           //月球的材质的环境光偏亮灰色 
		GLfloat moon_mat_diffuse[] = { 0.66,0.66,0.66,1.0 };           //月球的材质的漫反射光偏深灰色
		GLfloat moon_mat_specular[] = { 0.95,0.98,0.59,1.0 };          //月球的材质的镜面反射光偏亮黄色
		GLfloat moon_mat_emission[] = { 0.0,0.0,0.0,1.0 };
		GLfloat moon_mat_shininess = 30.0f;

		glMaterialfv(GL_FRONT, GL_EMISSION, moon_mat_emission);
		glMaterialfv(GL_FRONT, GL_AMBIENT, moon_mat_ambient);        
		glMaterialfv(GL_FRONT, GL_DIFFUSE, moon_mat_diffuse);
		glMaterialfv(GL_FRONT, GL_SPECULAR, moon_mat_specular);
		glMaterialf(GL_FRONT, GL_SHININESS, moon_mat_shininess);
	}

	{
		unsigned char *moon = SOIL_load_image("texture/moon.bmp", &moonWidth, &moonHeight, 0, SOIL_LOAD_RGB);     //使用SOIL库的载入纹理图片的函数
		glGenTextures(1, &texture_moon);   //生成一个纹理对象
		glBindTexture(GL_TEXTURE_2D, texture_moon);  //绑定该纹理对象
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, moonWidth, moonHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, moon); //生成月亮的纹理
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		SOIL_free_image_data(moon);        //释放图像缓存
		moonquadPlanet = gluNewQuadric();  //返回一个指向二次曲面的指针;
	}

	{
		//glutSolidSphere(5, 100, 100);//绘制月球；
		glEnable(GL_TEXTURE_2D);
		glGenerateMipmap(GL_TEXTURE_2D); //为target相关联的纹理生成一组完整的mipmap
		gluQuadricTexture(moonquadPlanet, GLU_TRUE); //设置自动计算纹理

		gluSphere(moonquadPlanet, 5, 100, 100);
		gluDeleteQuadric(moonquadPlanet);
		glDisable(GL_TEXTURE_2D);
	}
	
	glPopMatrix();


	glPopMatrix();
	

	glPopMatrix();

	glPushMatrix();

	DrawOrbit(101.9803903, 0.2, 1.0, 0);
	glRotatef(MarsAngle, 0.2, 1.0, 0.0);
	glPushMatrix();
	glTranslatef(100.0, -20.0, 0.0);

	
	{
		GLfloat mars_mat_ambient[] = { 0.8,0.21,0.24,1.0 };           //火星的材质的环境光偏黄红色
		GLfloat mars_mat_diffuse[] = { 1.0,0.37,0.24,1.0 };           //火星的材质的漫反射光偏铁锈色
		GLfloat mars_mat_specular[] = { 0.1,0.0,0.0,1.0 };
		GLfloat mars_mat_emission[] = { 0.0,0.0,0.0,1.0 };
		GLfloat mars_mat_shininess = 20.0f;

		glMaterialfv(GL_FRONT, GL_EMISSION, mars_mat_emission);
		glMaterialfv(GL_FRONT, GL_AMBIENT, mars_mat_ambient);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, mars_mat_diffuse);
		glMaterialfv(GL_FRONT, GL_SPECULAR, mars_mat_specular);
		glMaterialf(GL_FRONT, GL_SHININESS, mars_mat_shininess);
	}

	{
		unsigned char *mars = SOIL_load_image("texture/mars.bmp", &marsWidth, &marsHeight, 0, SOIL_LOAD_RGB);     //使用SOIL库的载入纹理图片的函数
		glGenTextures(1, &texture_mars);   //生成一个纹理对象
		glBindTexture(GL_TEXTURE_2D, texture_mars);  //绑定该纹理对象
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, marsWidth, marsHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, mars); //生成火星的纹理
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		SOIL_free_image_data(mars);        //释放图像缓存
		marsquadPlanet = gluNewQuadric();  //返回一个指向二次曲面的指针;
	}

	{
		//glutSolidSphere(15, 100, 100);   //绘制火星
		glEnable(GL_TEXTURE_2D);
		glGenerateMipmap(GL_TEXTURE_2D); //为target相关联的纹理生成一组完整的mipmap
		gluQuadricTexture(marsquadPlanet, GLU_TRUE); //设置自动计算纹理

		gluSphere(marsquadPlanet, 15, 100, 100);
		gluDeleteQuadric(marsquadPlanet);
		glDisable(GL_TEXTURE_2D);
	}
	
	glPopMatrix();

	glPopMatrix();

	
	glPushMatrix();

	DrawOrbit(158.113883, 0.3333, 1.0, 0);
	glRotatef(JupiterAngle, 0.3333, 1.0, 0.0);
	glPushMatrix();
	glTranslatef(150.0, -50.0, 0.0);

	{
		GLfloat jupiter_mat_diffuse[] = { 0.94,0.67,0.43,1.0 };           //木星的漫反射光偏土黄色
		GLfloat jupiter_mat_ambient[] = { 0.51,0.39,0.20,1.0 };           //木星的环境光偏暗黄色
		GLfloat jupiter_mat_specular[] = { 0.1,0.0,0.0,1.0 };
		GLfloat jupiter_mat_emission[] = { 0.0,0.0,0.0,1.0 };
		GLfloat jupiter_mat_shininess = 80.0f;                           //木星表面较亮,高光系数比较高

		glMaterialfv(GL_FRONT, GL_EMISSION, jupiter_mat_emission);
		glMaterialfv(GL_FRONT, GL_AMBIENT, jupiter_mat_ambient);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, jupiter_mat_diffuse);
		glMaterialfv(GL_FRONT, GL_SPECULAR, jupiter_mat_specular);
		glMaterialf(GL_FRONT, GL_SHININESS, jupiter_mat_shininess);

	}

	{
		unsigned char *jupiter = SOIL_load_image("texture/jupiter.bmp", &jupiterWidth, &jupiterHeight, 0, SOIL_LOAD_RGB);     //使用SOIL库的载入纹理图片的函数
		glGenTextures(1, &texture_jupiter);   //生成一个纹理对象
		glBindTexture(GL_TEXTURE_2D, texture_jupiter);  //绑定该纹理对象
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, jupiterWidth, jupiterHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, jupiter); //生成木星的纹理
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		SOIL_free_image_data(jupiter);        //释放图像缓存
		jupiterquadPlanet = gluNewQuadric();  //返回一个指向二次曲面的指针;
	}

	{
		//glutSolidSphere(20, 100, 100);   //绘制木星
		glEnable(GL_TEXTURE_2D);
		glGenerateMipmap(GL_TEXTURE_2D); //为target相关联的纹理生成一组完整的mipmap
		gluQuadricTexture(jupiterquadPlanet, GLU_TRUE); //设置自动计算纹理

		gluSphere(jupiterquadPlanet, 20, 100, 100);
		gluDeleteQuadric(jupiterquadPlanet);
		glDisable(GL_TEXTURE_2D);
	}
	

	glPopMatrix();

	glPopMatrix();

		
	glutSwapBuffers();
	//glFlush();
}

void init(void) {

	glewInit();
	glEnable(GL_DEPTH_TEST);
	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
	
	glShadeModel(GL_FLAT);       //选择FLAT模式着色
	

}

void reshapeFcn(GLint newWidth, GLint newHeight)
{
	glViewport(0, 0, newWidth, newHeight); //指定视口位置和大小

	winWidth = newWidth;
	winHeight = newHeight;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(90, GLfloat(newWidth*1.0 / newHeight), 1, 1000);     //观察的视景体大小

	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(eyex, eyey, eyez, xref, yref, zref, Vx, Vy, Vz);  //相机参数设置


}

//定时器函数,用于生成旋转动画
void timerFunc(int value) {
	if (earthAngle >= 360.0)
		earthAngle = 0;
	if (moonAngle >= 360.0)
		moonAngle = 0;
	if (JupiterAngle >= 360.0)
		JupiterAngle = 0;
	if (MarsAngle >= 360.0)
		MarsAngle = 0;

	earthAngle += earthstep;
	moonAngle += moonstep;
	JupiterAngle += jupiterstep;
	MarsAngle += Marsstep;

	glutPostRedisplay();

	glutTimerFunc(100, timerFunc, 1);

}


void processSpecialKeys(unsigned char key, int x, int y) {

	switch(key)
	{
	case 27:          //按ESC键推出程序
		exit(0);
		break;

	case 'w': //按w键，相机位置和天空盒沿z轴方向前进
		eyez -= 10;
		skybox_z -= 10;
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		break;

	case's': //按w键，相机位置和天空盒沿z轴方向后退
		eyez += 10;
		skybox_z += 10;
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		break;

	case'a': //按a键，相机位置和天空盒沿x轴方向左移
		eyex -= 10;
		skybox_x -= 10;
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		break;

	case'd': //按d键，相机位置和天空盒沿x轴方向右移
		eyex += 10;
		skybox_x += 10;
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		break;

	case'q': //按q键，相机位置和天空盒沿y轴方向上移
		eyey += 10;
		skybox_y += 10;
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		break;

	case'e': //按e键，相机位置和天空盒沿z轴方向下移
		eyey -= 10;
		skybox_y -= 10;
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		break;

	case'r': //按r键，相机位置和天空盒位置恢复初始设置值
		eyex = 0;
		eyey = 0;
		eyez = 300;
		skybox_x = skybox_y = skybox_z = 0;
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		break;
			
	}
	gluLookAt(eyex, eyey, eyez, xref, yref, zref, Vx, Vy, Vz);
	glutPostRedisplay();
}

//移动鼠标改变视角
void MouseMove(int x, int y) {
	
	const int x0 = 700;
	const int y0 = 300;

	int dx = x - x0;
	int dy = y0 - y;

	xref += 0.03*dx;
	yref += 0.03*dy;

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(eyex, eyey, eyez, xref, yref, zref, Vx, Vy, Vz);
	glutPostRedisplay();
}

void main(int argc, char** argv)
{
	////读写方式打开背景贴图的bmp文件
	//FILE* pFile = fopen(TestName, "rb");

	//if (pFile == 0)

	//{
	//	cout << "FILE open error!" << endl;
	//	Sleep(3000);
	//	exit(0);

	//}
	//// 移动到0x0012位置
	//fseek(pFile, 0x0012, SEEK_SET);
	////读取宽度
	//fread(&ImageWidth, sizeof(ImageWidth), 1, pFile);
	////读取长度
	//fread(&ImageHeight, sizeof(ImageHeight), 1, pFile);

	////每行数据长度
	//PixelLength = ImageWidth * 3;

	////修正Length使其为4的倍数
	//while (PixelLength % 4 != 0)

	//	++PixelLength;

	//PixelLength *= ImageHeight;
	//PixelData = (GLubyte*)malloc(PixelLength);
	//if (PixelData == 0)

	//	exit(0);

	////读取文件头后的数据
	//fseek(pFile, 54, SEEK_SET);
	//fread(PixelData, PixelLength, 1, pFile);
	////关闭文件
	//fclose(pFile);
	
	//初始化glut并运行
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowPosition(50, 50);
	glutInitWindowSize(winWidth, winHeight);
	glutCreateWindow("HXZ's solar system");

	init();
	glutDisplayFunc(display);
	glutReshapeFunc(reshapeFcn);
	glutTimerFunc(100, timerFunc, 1);

	glutKeyboardFunc(processSpecialKeys);
	glutPassiveMotionFunc(MouseMove);
	
	glutMainLoop();
}
