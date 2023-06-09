//image = ambient + diffuse + specular
#include<vector>
#include <iostream>

#include"tgaImage.h"
#include"model.h"
#include"geometry.h"
#include"our_gl.h"

using std::cout;
using std::endl;

Model* model = NULL;
const int width = 800;
const int height = 800;

TGAImage shadowbuffer;

Vec3f light_dir(1, 1, 3);   // 光源位置
Vec3f center(0, 0, 0);
Vec3f eye(1, 1, 3);     //相机位置
Vec3f up(0, 1, 0);

struct GourauShader : public IShader {
	Vec3f varying_intensity;
	mat<2, 3, float> varying_uv; // triangle uv coordinates
	mat<4, 4, float> uniform_M;   //  Projection*ModelView
	mat<4, 4, float> uniform_MIT; // (Projection*ModelView).invert_transpose()
	virtual Vec4f vertex(int iface, int nthvert)
	{
		varying_intensity[nthvert] = std::max(0.f, model->normal(iface, nthvert) * light_dir);
		varying_uv.set_col(nthvert, model->uv(iface, nthvert));
		//the function of embed<4> is to add a 1 to the end of the vector, so that the vector can be multiplied by the matrix
		Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert)); 
		return viewPort * projection * modelView * gl_Vertex;
	}
	virtual bool fragment(Vec3f barycen, TGAColor& color)
	{
		float intensity = varying_intensity * barycen;
		Vec2f uv = varying_uv * barycen; //通过三个顶点的坐标插值得到的坐标
		Vec3f n = proj<3>(uniform_MIT * embed<4>(model->normal(uv))).normalize();
		Vec3f l = proj<3>(uniform_M * embed<4>(light_dir)).normalize();
		//r is the reflection of light
		//r.z is the z coordinate of the reflection of light
		Vec3f r = (n * (n * l * 2.f) - l).normalize();

		float diff = std::max(0.f, n * l);
		//the r.z is used for the specular, not x and y is because the light is from the top
		float spec = pow(std::max(r.z, 0.0f), model->specular(uv));
		TGAColor  c = model->diffuse(uv);
		color = c;
		//原来是color * diff     ambient 5/ diff 1/ spec 0.6
		for (int i = 0; i < 3; i++) color[i] = std::min<float>(5 + c[i] * (diff + .6 * spec), 255);
		return false;
	}
};
//test 11
struct PhongShader : public IShader {  
	mat<2, 3, float> varying_uv;
	mat<3, 3, float> varying_nrm;
	mat<4, 3, float> varying_tri;
	mat<3, 3, float> ndc_tri;
	//Darboux basis
	virtual Vec4f vertex(int iface, int nthvert)
	{
		varying_uv.set_col(nthvert, model->uv(iface, nthvert));
		varying_nrm.set_col(nthvert, proj<3>((projection * modelView).invert_transpose() * embed<4>(model->normal(iface, nthvert), 0.f)));
		Vec4f gl_Vertex = projection * modelView * embed<4>(model->vert(iface, nthvert));
		varying_tri.set_col(nthvert, gl_Vertex);
		ndc_tri.set_col(nthvert, proj<3>(gl_Vertex / gl_Vertex[3]));
		return gl_Vertex;
	}
	virtual bool fragment(Vec3f barc, TGAColor& color)
	{
		Vec3f bn = (varying_nrm * barc).normalize();
		Vec2f uv = varying_uv * barc;

		mat<3, 3, float> A;
		A[0] = ndc_tri.col(1) - ndc_tri.col(0);
		A[1] = ndc_tri.col(2) - ndc_tri.col(0);
		A[2] = bn;

		mat<3, 3, float> AI = A.invert();

		Vec3f i = AI * Vec3f(varying_uv[0][1] - varying_uv[0][0], varying_uv[0][2] - varying_uv[0][0], 0);
		Vec3f j = AI * Vec3f(varying_uv[1][1] - varying_uv[1][0], varying_uv[1][2] - varying_uv[1][0], 0);

		mat<3, 3, float> B;
		B.set_col(0, i.normalize());
		B.set_col(1, j.normalize());
		B.set_col(2, bn);

		Vec3f n = (B * model->normal(uv)).normalize();

		float diff = std::max(0.f, n * light_dir);
		//float diff = std::max(0.f, bn* light_dir);
		color = model->diffuse(uv) * diff;

		return false;
	}
};

struct DepthShader : public IShader {
	mat<3, 3, float> varying_tri;
	virtual Vec4f vertex(int iface, int nthvert)
	{
		Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert)); // read the vertex from .obj file
		gl_Vertex = viewPort * projection * modelView * gl_Vertex;          // transform it to screen coordinates
		varying_tri.set_col(nthvert, proj<3>(gl_Vertex / gl_Vertex[3]));
		return gl_Vertex;
	}
	virtual bool fragment(Vec3f barc, TGAColor& color)
	{
		Vec3f p = varying_tri * barc;
		float frag_depth = p.z;
		color = TGAColor(255, 255, 255) * (p.z / 255);
		return false;
	}
};

struct Shader : public IShader {
	Vec3f varying_intensity;
	mat<2, 3, float> varying_uv; // triangle uv coordinates
	mat<4, 4, float> uniform_M;   //  Projection*ModelView
	mat<4, 4, float> uniform_MIT; // (Projection*ModelView).invert_transpose()
	mat<3, 3, float> varying_tri;
	mat<4, 4, float> uniform_Mshadow;
	virtual Vec4f vertex(int iface, int nthvert)
	{
		varying_uv.set_col(nthvert, model->uv(iface, nthvert));
		//the function of embed<4> is to add a 1 to the end of the vector, so that the vector can be multiplied by the matrix
		Vec4f gl_Vertex = viewPort * projection * modelView * embed<4>(model->vert(iface, nthvert));
		varying_tri.set_col(nthvert, proj<3>(gl_Vertex / gl_Vertex[3]));
		return gl_Vertex;
	}
	virtual bool fragment(Vec3f barycen, TGAColor& color)
	{
		Vec4f sb_p = uniform_Mshadow * embed<4>(varying_tri * barycen);
		sb_p = sb_p / sb_p[3];
		float shadow = 0.3 + 0.7 * (shadowbuffer.get(sb_p[0], sb_p[1])[0] < sb_p[2]   + 43.34);
		float intensity = varying_intensity * barycen;
		Vec2f uv = varying_uv * barycen; //通过三个顶点的坐标插值得到的坐标
		Vec3f n = proj<3>(uniform_MIT * embed<4>(model->normal(uv))).normalize();
		Vec3f l = proj<3>(uniform_M * embed<4>(light_dir)).normalize();
		//r is the reflection of light
		//r.z is the z coordinate of the reflection of light
		Vec3f r = (n * (n * l * 2.f) - l).normalize();

		float diff = std::max(0.f, n * l);
		//the r.z is used for the specular, not x and y is because the light is from the top
		float spec = pow(std::max(r.z, 0.0f), model->specular(uv));
		TGAColor  c = model->diffuse(uv);
		color = c;
		//原来是color * diff     ambient 5/ diff 1/ spec 0.6
		for (int i = 0; i < 3; i++) color[i] = std::min<float>(5 + c[i] * shadow* (diff + .6 * spec), 255);
		return false;
	}
};

int main(int argc, char** argv)
{
	if (argc == 2)
	{
		model = new Model(argv[1]);
	}
	else
	{
		model = new Model("obj/african_head.obj");
		
	}
	TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);
	shadowbuffer = TGAImage(width, height, TGAImage::GRAYSCALE);
	//zbuffer he shadow buffer 对应的几个矩阵不同
	light_dir.normalize();
	

	//Projection(-1.f / (eye - center).norm());
	//light_dir = proj<3>((projection * modelView * embed<4>(light_dir, 0.f))).normalize();
	
	TGAImage image(width, height, TGAImage::RGB);
	LookAt(light_dir, center, up);
	ViewPort(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
	Projection(0);
	
	//PhongShader shader;
	DepthShader depthShader;
	//shader.uniform_M = projection * modelView;
	//shader.uniform_MIT = (projection * modelView).invert_transpose(); // uniform pass constant parameters 
	for (int i = 0; i < model->nfaces(); i++)
	{
		Vec4f screen_coords[3]; //3个顶点
		for (int j = 0; j < 3; j++)
		{
			screen_coords[j] =  depthShader.vertex(i, j);
		}
		triangle(screen_coords, depthShader, image, shadowbuffer);
	}
	image.flip_horizontally();
	//image.write_tga_file("output/output11.tga");

	Matrix M = viewPort * projection * modelView;

	{ // rendering the frame buffer
		TGAImage frame(width, height, TGAImage::RGB);
		LookAt(eye, center, up);
		ViewPort(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
		Projection(-1.f / (eye - center).norm());

		Shader shader;
		shader.uniform_M = projection * modelView;
		shader.uniform_MIT = (projection * modelView).transpose();
		shader.uniform_Mshadow = M * (viewPort * projection * modelView).invert(); // tail to transform frame buffer to object, and front transform object to shadow buffer
		//here build the relationship between frame buffer and shadow buffer//// frame screen -> shadow screen
		Vec4f screen_coords[3];
		for (int i = 0; i < model->nfaces(); i++) {
			for (int j = 0; j < 3; j++) {
				screen_coords[j] = shader.vertex(i, j);
			}
			triangle(screen_coords, shader, frame, zbuffer);
		}
		//frame.flip_vertically(); // to place the origin in the bottom left corner of the image
		frame.write_tga_file("output/output11.tga");
	}
	
	delete model;
	return 0;
}
//nothing
//test 

//main funciton is the shader program