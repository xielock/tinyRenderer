#include<cmath>
#include<limits>
#include<cstdlib>
#include"our_gl.h"

Matrix modelView;
Matrix viewPort;
Matrix projection;

IShader::~IShader() {}

//视口
void ViewPort(int x, int y, int w, int h)
{
	viewPort = Matrix::identity();
	viewPort[0][3] = x + w / 2.f;
	viewPort[1][3] = y + h / 2.f;
	viewPort[2][3] = 255.0 / 2.f;
	viewPort[0][0] = w / 2.f;
	viewPort[1][1] = h / 2.f;
	viewPort[2][2] = 255.f / 2.f;
}
//透视投影
void Projection(float coeff)
{
	projection = Matrix::identity();
	projection[3][2] = coeff;
}
//Model View变换
void LookAt(Vec3f eye, Vec3f center, Vec3f up)
{
	Vec3f z = (eye - center).normalize();
	Vec3f x = cross(z, up).normalize();
	Vec3f y = cross(z, x).normalize();
	modelView = Matrix::identity();
	for (int i = 0; i < 3; i++)
	{
		modelView[0][i] = x[i];
		modelView[1][i] = y[i];
		modelView[2][i] = z[i];
		modelView[i][3] = -center[i];
	}
}
//重心坐标 barycentric
Vec3f barycentric(Vec2f A, Vec2f B, Vec2f C, Vec2f P)
{
	Vec3f s[2];
	for (int i = 2; i--;)
	{
		s[i][0] = C[i] - A[i];
		s[i][1] = B[i] - A[i];
		s[i][2] = A[i] - P[i];
	}
	//Vec3f u = cross(Vec3f(C.x - A.x, B.x - A.x, A.x - P.x),Vec3f(C.y - A.y, B.y - A.y, A.y - P.y));
	Vec3f u = cross(s[0], s[1]);
	if (std::abs(u[2]) > 1e-2) // dont forget that u[2] is integer. If it is zero then triangle ABC is degenerate
		return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
	return Vec3f(-1, 1, 1); // in this case generate negative coordinates, it will be thrown away by the rasterizator
}

//绘制三角形
void triangle(Vec4f* pts, IShader& shader, TGAImage& image, TGAImage& zbuffer)
{
	Vec2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
	for(int i = 0; i< 3; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			bboxmin[j] = std::min(bboxmin[j], pts[i][j] / pts[i][3]);
			bboxmax[j] = std::max(bboxmax[j], pts[i][j] / pts[i][3]);
		}
	}
	Vec2i P;
	TGAColor color; //遍历包围盒
	for(P.x = bboxmin.x; P.x <= bboxmax.x; P.x ++)
		for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++)
		{
			//proj<2> is used for 2D projection
			Vec3f c = barycentric(proj<2>(pts[0] / pts[0][3]), proj<2>(pts[1] / pts[1][3]), proj<2>(pts[2] / pts[2][3]), proj<2>(P));
			float z = pts[0][2] * c.x + pts[1][2] * c.y + pts[2][2] * c.z;
			// the funciton of w is to make the z value of the point in the triangle more accurate
			//pts[i][3] means the w value of the point, w is the distance from the camera to the point
			float w = pts[0][3] * c.x + pts[1][3] * c.y + pts[2][3] * c.z;
			int frag_depth = std::max(0, std::min(255, int(z / w + 0.5)));
			if (c.x < 0 || c.y < 0 || c.z < 0 || zbuffer.get(P.x, P.y)[0] > frag_depth) continue;
			bool discard = shader.fragment(c, color);
			if (!discard)
			{
				zbuffer.set(P.x, P.y, frag_depth);
				image.set(P.x, P.y, color);
			}
		}
}