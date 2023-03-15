#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "model.h"

Model::Model(const char* filename) : verts_(), faces_(), faces_texture_(), textures_(){
    std::ifstream in;
    in.open(filename, std::ifstream::in);
    if (in.fail()) return;
    std::string line;
    while (!in.eof()) {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char trash;
        if (!line.compare(0, 2, "v ")) {
            iss >> trash;
            Vec3f v;
            for (int i = 0; i < 3; i++) iss >> v[i];
            verts_.push_back(v);
        }
        else if (!line.compare(0, 4, "vt  "))
        {
			iss >> trash>> trash;//test
			Vec2f vt;
			for (int i = 0; i < 2; i++) iss >> vt[i];
			textures_.push_back(vt);
		}
		else if (!line.compare(0, 2, "f ")) {
			std::vector<int> f;
			std::vector<int> f2;
			int itrash, idx, idx_texture;
			iss >> trash;
			while (iss >> idx >> trash >> idx_texture >> trash >> itrash) {
				idx--; // in wavefront obj all indices start at 1, not zero
				idx_texture--;
				f.push_back(idx);
				f2.push_back(idx_texture);
			}
			faces_.push_back(f);
			faces_texture_.push_back(f2);
        }
    }
    std::cerr << "# v# " << verts_.size() << " f# " << faces_.size() << std::endl;
}

Model::~Model() {
}

int Model::nverts() {
    return (int)verts_.size();
}

int Model::nfaces() {
    return (int)faces_.size();
}

std::vector<int> Model::face(int idx) {
    return faces_[idx];
}
std::vector<int> Model::face_texture(int idx)
{
	return faces_texture_[idx];  //texture µÄ3¸öindex
}
Vec3f Model::vert(int i) {
    return verts_[i];
}
Vec2f Model::texture(int i)
{
	return textures_[i];
}

