#include<vector>
#include"tgaImage.h"
#include"geometry.h"

extern Matrix modelView;
extern Matrix viewPort;
extern Matrix projection;

void ViewPort(int x, int y, int w, int h);
void Projection(float coeff = 0.f); //coeff = -1/c
void LookAt(Vec3f eye, Vec3f center, Vec3f up);

struct IShader {
	virtual ~IShader();
	virtual Vec3f vertex(int iface, int nthvert) = 0;
	virtual bool fragment(Vec3f bar, TGAColor& color) = 0;
};

void triangle(Vec3f * pts, IShader& shader, TGAImage& image, float* zbuffer);