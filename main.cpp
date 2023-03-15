#include<vector>
#include<cmath>
#include"tgaImage.h"
#include"model.h"
#include"geometry.h"
#include <iostream>

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
Model* model = NULL;
TGAImage textureImage;
const int width = 800;
const int height = 800;
const int depth = 255;

Vec3f light_dir(0, 0, -1);
Vec3f center(0, 0, 0);
Vec3f eye(1, 1, 3);

void line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color)
{
	bool steep = false;
	if (abs(x0 - x1) < abs(y0 - y1))
	{
		std::swap(x0, y0);
		std::swap(x1, y1);
		steep = true;
	}
	if (x0 > x1)
	{
		std::swap(x0, x1);
		std::swap(y0, y1);
	}

	for (int x = x0; x <= x1; x++)
	{
		float t = (x - x0) / (float)(x1 - x0);
		//int y = y0 * (1. - t) + y1 * t;
		int y = y0 + (y0 - y1) * t;
		if (steep)
		{
			image.set(y, x, color);
		}
		else
		{
			image.set(x, y, color);
		}
	}
}
void triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage& image, TGAColor color)
{
	if (t0.y == t1.y && t0.y == t2.y) return;
	// max 1 : t2   mid t1 : min t0

	if (t0.y > t1.y) std::swap(t0, t1);
	if (t0.y > t2.y) std::swap(t0, t2);
	if (t1.y > t2.y) std::swap(t1, t2);
	int total_height = t2.y - t0.y;
	for (int i = 0; i < total_height; i++)
	{
		bool second_half = i > t1.y - t0.y || t1.y == t0.y;
		int segment_height = second_half ? t2.y - t1.y : t1.y - t0.y;
		float alpha = (float)i / total_height;
		float beta = (float)(i - (second_half ? t1.y - t0.y : 0)) / segment_height;
		Vec2i A = t0 + (t2 - t0) * alpha;
		Vec2i B = second_half ? t1 + (t2 - t1) * beta : t0 + (t1 - t0) * beta;
		if (A.x > B.x) std::swap(A, B);
		for (int j = A.x; j <= B.x; j++)
			image.set(j, t0.y + i, color);
	}
	
}
//重心坐标 baricentric coordinates
Vec3f barycentric(Vec2i* pts, Vec2i P) {
	//ABx, ACx, PAx           ABy, ACy, PAy
	Vec3f u = Vec3f(pts[2].x - pts[0].x, pts[1].x - pts[0].x, pts[0].x - P.x) ^ Vec3f(pts[2].y - pts[0].y,  pts[1].y - pts[0].y, pts[0].y - P.y);
	/* `pts` and `P` has integer value as coordinates
	   so `abs(u[2])` < 1 means `u[2]` is 0, that means
	   triangle is degenerate, in this case return something with negative coordinates */
	if (std::abs(u.z) < 1) return Vec3f(-1, 1, 1);
	return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
}
Vec3f barycentric(Vec3f * pts, Vec3f P) {
	//ABx, ACx, PAx           ABy, ACy, PAy
	Vec3f u = Vec3f(pts[2].x - pts[0].x, pts[1].x - pts[0].x, pts[0].x - P.x) ^ Vec3f(pts[2].y - pts[0].y, pts[1].y - pts[0].y, pts[0].y - P.y);
	/* `pts` and `P` has integer value as coordinates
	   so `abs(u[2])` < 1 means `u[2]` is 0, that means
	   triangle is degenerate, in this case return something with negative coordinates */
	if (std::abs(u.z) < 1) return Vec3f(-1, 1, 1);
	return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
}
void triangle(Vec2i *pts, TGAImage& image, TGAColor color) {
	Vec2i bboxmin(image.get_width() - 1, image.get_height() - 1);
	Vec2i bboxmax(0, 0);
	Vec2i clamp(image.get_width() - 1, image.get_height() - 1);
	for (int i = 0; i < 3; i++) {
		bboxmin.x = std::max(0, std::min(bboxmin.x, pts[i].x));
		bboxmin.y = std::max(0, std::min(bboxmin.y, pts[i].y));

		bboxmax.x = std::min(clamp.x, std::max(bboxmax.x, pts[i].x));
		bboxmax.y = std::min(clamp.y, std::max(bboxmax.y, pts[i].y));
	}
	Vec2i P;
	for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
		for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
			Vec3f bc_screen = barycentric(pts, P);
			if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0) continue;
			image.set(P.x, P.y, color);
		}
	}
}
void triangle(Vec3f* pts, float* zbuffer, TGAImage& image, std::vector<TGAColor> colors, float intensity)
{
	Vec2f bboxmin(image.get_width() - 1, image.get_height() - 1);
	Vec2f bboxmax(0, 0);
	Vec2f clamp(image.get_width() - 1, image.get_height() - 1);
	for (int i = 0; i < 3; i++) {
		bboxmin.x = std::max(0.f, std::min(bboxmin.x, pts[i].x));
		bboxmin.y = std::max(0.f, std::min(bboxmin.y, pts[i].y));

		bboxmax.x = std::min(clamp.x, std::max(bboxmax.x, pts[i].x));
		bboxmax.y = std::min(clamp.y, std::max(bboxmax.y, pts[i].y));
	}
	Vec3f P;
	for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++)
	{
		for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++)
		{
			Vec3f bc_screen = barycentric(pts, P);
			if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0) continue;
			P.z = pts[0].z * bc_screen.x + pts[1].z * bc_screen.y + pts[2].z * bc_screen.z;
			TGAColor color = colors[0] * bc_screen.x * intensity + colors[1] * bc_screen.y * intensity + colors[2] * bc_screen.z * intensity;
			int idx = P.x + P.y * width;
			if (zbuffer[idx] < P.z)
			{
				zbuffer[idx] = P.z;
				image.set(P.x, P.y, color);
			}
		}
	}			
	
}
Matrix v2m(Vec3f v)
{
	Matrix m(4, 1);
	m[0][0] = v.x;
	m[1][0] = v.y;
	m[2][0] = v.z;
	m[3][0] = 1.f;
	return m;
}
Vec3f m2v(Matrix m)
{
	return Vec3f(m[0][0] / m[3][0], m[1][0] / m[3][0], m[2][0] / m[3][0]);
}

//视口变换，将图像有铺满屏幕
Matrix viewport(int x, int y, int w, int h)
{
	Matrix m = Matrix::identity(4);
	m[0][3] = x + w / 2.f;
	m[1][3] = y + h / 2.f;
	m[2][3] = 0 + depth / 2.f;

	m[0][0] = w / 2.f;
	m[1][1] = h / 2.f;
	m[2][2] = depth / 2.f;
	return m;
}
//固定相机，改变物体位置
Matrix lookat(Vec3f eye, Vec3f center, Vec3f up)
{
	Vec3f z = (eye - center).normalize();
	Vec3f x = (up ^ z).normalize();
	Vec3f y = (z ^ x).normalize();
	Matrix res = Matrix::identity(4);
	for (int i = 0; i < 3; i++) {
		res[0][i] = x[i];
		res[1][i] = y[i];
		res[2][i] = z[i];
		res[i][3] = -center[i];
	}
	return res;
}

Vec3f world2screen(Vec3f v)
{
	return Vec3f(int((v.x + 1.) * width / 2 + .5), int((v.y + 1.) * height / 2 + .5), v.z);
}
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
	TGAImage image(width, height, TGAImage::RGB);
	textureImage.read_tga_file("obj/african_head_diffuse.tga");
	textureImage.flip_vertically();
	int textureWidth = textureImage.get_width();
	int textureHeight = textureImage.get_height();
	

	//raster  
	//for (int i = 0; i < model->nfaces(); i++)
	//{
	//	std::vector<int> face = model->face(i);
	//	Vec2i screen_coords[3];
	//	Vec3f world_coords[3];
	//	for (int j = 0; j < 3; j++)
	//	{
	//		Vec3f v = model->vert(face[j]);//每个面有三个顶点，依次取出顶点
	//		screen_coords[j] = Vec2i((v.x + 1.) * width / 2., (v.y + 1.) * height / 2.);
	//		world_coords[j] = v;
	//	}
	//	//面法线方向
	//	Vec3f n = (world_coords[2] - world_coords[0]) ^ (world_coords[1] - world_coords[0]);
	//	n.normalize();
	//	float intensity = n * light_dir;
	//	if (intensity > 0)  //back-face culling
	//	{
	//		triangle(screen_coords[0], screen_coords[1], screen_coords[2], image, TGAColor(intensity * 255, intensity * 255, intensity * 255, 255));
	//	}
	//}
	//z 方向计算buffer 再raster
	float* zbuffer = new float[width * height];

	Matrix ModelView = lookat(eye, center, Vec3f(0, 1, 0));// up direction
	Matrix Projection = Matrix::identity(4);
	Projection[3][2] = -1.f / (eye - center).norm();
	Matrix viewPort = viewport(width / 8, width / 8, width * 3 / 4, width * 3 / 4);

	for (int i = 0; i < width * height; i++)
	{
		zbuffer[i] = -std::numeric_limits<float>::max();  //越大距离相机越近，越小离得越远
	}
	for (int i = 0; i < model->nfaces(); i++)
	{
		std::vector<int> vert_indexes = model->face(i);
		std::vector<int> texture_indexes = model->face_texture(i);
		Vec3f pts[3]; //三个顶点
		Vec2f uv[3]; //三个顶点的纹理
		std::vector<TGAColor> colors;
		Vec3f world_coords[3];
		for (int j = 0; j < 3; j++)
		{
			Vec3f v = model->vert(vert_indexes[j]);
			Vec2f vt = model->texture(texture_indexes[j]);
			pts[j] = Vec3f(int((v.x + 1.) * width / 2 + .5), int((v.y + 1.) * height / 2 + .5), v.z);
			uv[j] = Vec2f(int((vt.x) * textureWidth + .5), int((vt.y) * textureHeight + .5));
			//perspective projection,multiply [1 0 0 0; 0 1 0 0; 0 0 1 0; 0, 0, -1/camera_pos_c, 1] to the matrix
			world_coords[j] = v;
			
			pts[j] = world2screen(m2v(Projection * v2m(v)));
			//或者插值出3个顶点的texture坐标，直接取一次颜色
			TGAColor color = textureImage.get(uv[j].x, uv[j].y);
			colors.push_back(color);
		}
		Vec3f n = (world_coords[2] - world_coords[0]) ^ (world_coords[1] - world_coords[0]);
		n.normalize();
		float intensity = n * light_dir;
		
		triangle(pts, zbuffer, image, colors, intensity);
	}
	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("output5.tga");
	delete model;
	return 0;
}
		