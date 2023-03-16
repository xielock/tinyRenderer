#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "model.h"

Model::Model(const char* filename) : faces_(), verts_(), norms_(), uv_(), diffusemap_(), normalmap_(), specularmap_() {
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
        else if (!line.compare(0, 3, "vn ")) {
			iss >> trash >> trash;
            Vec3f n;
            for (int i = 0; i < 3; i++) iss >> n[i];
            norms_.push_back(n);
        }
        else if (!line.compare(0, 3, "vt "))
        {
			iss >> trash>> trash;//test
			Vec2f uv;
			for (int i = 0; i < 2; i++) iss >> uv[i];
            uv_.push_back(uv);
		}
		else if (!line.compare(0, 2, "f ")) {
            std::vector<Vec3i>f;
            Vec3i temp;
			iss >> trash;
			while (iss >> temp[0] >> trash >> temp[1] >> trash >> temp[2]) {
                for (int i = 0; i < 3; i++) temp[i] --;
                f.push_back(temp);
			}
			faces_.push_back(f);
        }
    }
    std::cerr << "# v# " << verts_.size() << " f# " << faces_.size() <<" vt# "<<uv_.size() <<" vn# "<< norms_.size()<< std::endl;
    load_texture(filename, "_diffuse.tga", diffusemap_);
    load_texture(filename, "_nm.tga", normalmap_);
    load_texture(filename, "_spec.tga", specularmap_);
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
    std::vector<int>vec;                                           //第idx个面的第i个顶点，共3个
    for (int i = 0; i < (int)faces_[i].size(); i++) vec.push_back(faces_[idx][i][0]);
    return vec;
}

Vec3f Model::normal(int iface, int nthvert) {
	return norms_[faces_[iface][nthvert][2]];
}
Vec3f Model::normal(Vec2f uvf)
{
    Vec2i uv(uvf[0] * normalmap_.get_width(), uvf[1] * normalmap_.get_height());
    TGAColor c = normalmap_.get(uv[0], uv[1]);
    Vec3f res;
    for (int i = 0; i < 3; i++)
    {
        res[2 - i] = (float)c[i] / 255.f * 2.f - 1.f;
    }
    return res;
}
Vec3f Model::vert(int iface, int nthvert) {
	return verts_[faces_[iface][nthvert][0]];
}
Vec3f Model::vert(int i) {
    return verts_[i];
}
Vec2f Model::uv(int iface, int nthvert)
{
	return uv_[faces_[iface][nthvert][1]];
}
Vec2f Model::uv(int i)
{
    return uv_[i];
}
void Model::load_texture(std::string filename, const char* suffix, TGAImage& img)
{
	//read one filename and add the suffix，then load file to img
    std::string texfile(filename);
    size_t dot = texfile.find_last_of(".");
    if (dot != std::string::npos) {
        texfile = texfile.substr(0, dot) + std::string(suffix);
        std::cerr << "texture file " << texfile << " loading " << (img.read_tga_file(texfile.c_str()) ? "ok" : "failed") << std::endl;
        img.flip_vertically();
    }
}
TGAColor Model::diffuse(Vec2f uv)
{
	Vec2i uv1(uv[0] * diffusemap_.get_width(), uv[1] * diffusemap_.get_height());
	return diffusemap_.get(uv1[0], uv1[1]);
}
float Model::specular(Vec2f uv)
{
    Vec2i uv1(uv[0] * specularmap_.get_width(), uv[1] * specularmap_.get_height());
    return specularmap_.get(uv1[0], uv1[1])[0] / 1.0f;
}