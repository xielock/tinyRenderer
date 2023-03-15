#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "geometry.h"
#include "tgaImage.h"

class Model {
private:

	std::vector<std::vector<Vec3i> > faces_;   //for vertex/ux/normal index of every face
	std::vector<Vec3f> verts_;
	std::vector<Vec2f> uv_;
	std::vector<Vec3f> norms_;
	TGAImage diffusemap_;
	TGAImage normalmap_;
	TGAImage specularmap_;
	void load_texture(std::string filename, const char* suffix, TGAImage& img);

public:
	Model(const char* filename);
	~Model();
	int nverts();
	int nfaces();
	std::vector<int> face(int idx);
	Vec3f normal(int iface, int nthvert); //specific face and specific vertex
	Vec3f normal(Vec2f uv);
	Vec3f vert(int iface, int nthvert);
	Vec3f vert(int i);
	Vec2f uv(int iface, int nthvert);
	Vec2f uv(int i);
	TGAColor diffuse(Vec2f uv);  //return color from u\v position
	float specular(Vec2f uv);
};

#endif //__MODEL_H__