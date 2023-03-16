#include<vector>
#include"tgaImage.h"
#include"geometry.h"

extern Matrix modelView;  //外部声明，允许extern 到其他cpp文件
extern Matrix viewPort;
extern Matrix projection;

void ViewPort(int x, int y, int w, int h);
void Projection(float coeff = 0.f); //coeff = -1/c
void LookAt(Vec3f eye, Vec3f center, Vec3f up);

struct IShader {
	virtual ~IShader();
	virtual Vec4f vertex(int iface, int nthvert) = 0;
	virtual bool fragment(Vec3f bar, TGAColor& color) = 0;
};

void triangle(Vec4f* pts, IShader& shader, TGAImage& image, TGAImage& zbuffer);