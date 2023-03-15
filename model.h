#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "geometry.h"

class Model {
private:
	std::vector<Vec3f> verts_;
	std::vector<std::vector<int> > faces_;
	std::vector<std::vector<int>> faces_texture_;
	std::vector<Vec2f> textures_;
public:
	Model(const char* filename);
	~Model();
	int nverts();
	int nfaces();
	Vec3f vert(int i);
	Vec2f texture(int i);       //return every texture's coordinates
	std::vector<int> face(int idx);
	std::vector<int> face_texture(int idx);  //return every face's texture index, include 3
};

#endif //__MODEL_H__