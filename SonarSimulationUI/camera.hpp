#ifndef CAMERA_H  
#define CAMERA_H  

#include <math.h>  

class Point3
{
public:
	float x, y, z;
	void set(float dx, float dy, float dz)
	{
		x = dx; y = dy; z = dz;
	}
	void set(Point3& p)
	{
		x = p.x; y = p.y; z = p.z;
	}
	Point3(float xx, float yy, float zz)
	{
		x = xx; y = yy; z = zz;
	}
	Point3()
	{
		x = y = z = 0;
	}
	void build4tuple(float v[])
	{
		v[0] = x; v[1] = y; v[2] = z; v[3] = 1.0f;
	}
};

class Vector3
{
public:
	float x, y, z;
	void set(float dx, float dy, float dz)
	{
		x = dx; y = dy; z = dz;
	}
	void set(Vector3& v)
	{
		x = v.x; y = v.y; z = v.z;
	}
	void flip()
	{
		x = -x; y = -y; z = -z;
	}
	void setDiff(Point3& a, Point3& b)
	{
		x = a.x - b.x; y = a.y - b.y; z = a.z - b.z;
	}
	void normalize()
	{
		float base = pow(x, 2) + pow(y, 2) + pow(z, 2);
		x = x / pow(base, 0.5f);
		y = y / pow(base, 0.5f);
		z = z / pow(base, 0.5f);
	}
	Vector3(float xx, float yy, float zz)
	{
		x = xx; y = yy; z = zz;
	}
	Vector3(Vector3 &v)
	{
		x = v.x; y = v.y; z = v.z;
	}
	Vector3()
	{
		x = 0; y = 0; z = 0;
	}

	Vector3 cross(Vector3 b)
	{
		float x1, y1, z1;
		x1 = y*b.z - z*b.y;
		y1 = z*b.x - x*b.z;
		z1 = x*b.y - y*b.x;
		Vector3 c(x1, y1, z1);
		return c;
	}

	float dot(Vector3 b)
	{
		float d = x*b.x + y*b.y + z*b.z;
		return d;
	}
};

class Camera
{
public:
	/* 构造函数和析构函数 */
	Camera(){};
	~Camera(){};

	/* 设置摄像机的位置, 观察点和向上向量 */
	void setCamera(float eyeX, float eyeY, float eyeZ,
		float lookX, float lookY, float lookZ,
		float upX, float upY, float upZ)
	{
		/* 构造向量 */
		eye.set(eyeX, eyeY, eyeZ);
		look.set(lookX, lookY, lookZ);
		up.set(upX, upY, upZ);
		Vector3 upvec(up.x - eye.x, up.y - eye.y, up.z - eye.z);

		/* 计算n、u、v并归一化*/
		n.set(eye.x - look.x, eye.y - look.y, eye.z - look.z);
		u.set(upvec.cross(n).x, upvec.cross(n).y, upvec.cross(n).z);
		v.set(n.cross(u).x, n.cross(u).y, n.cross(u).z);

		u.normalize();
		v.normalize();
		n.normalize();
		setModelViewMatrix();
	};
	void roll(float angle)
	{
		float cs = cos(angle*3.14159265 / 180);
		float sn = sin(angle*3.14159265 / 180);
		Vector3 t(u);
		Vector3 s(v);
		u.set(cs*t.x - sn*s.x, cs*t.y - sn*s.y, cs*t.z - sn*s.z);
		v.set(sn*t.x + cs*s.x, sn*t.y + cs*s.y, sn*t.z + cs*s.z);
		setModelViewMatrix();          //每次计算完坐标轴变化后调用此函数更新视点矩阵  
	};
	void pitch(float angle)
	{
		float cs = cos(angle*3.14159265 / 180);
		float sn = sin(angle*3.14159265 / 180);
		Vector3 t(v);
		Vector3 s(n);
		v.set(cs*t.x - sn*s.x, cs*t.y - sn*s.y, cs*t.z - sn*s.z);
		n.set(sn*t.x + cs*s.x, sn*t.y + cs*s.y, sn*t.z + cs*s.z);
		setModelViewMatrix();
	};
	void yaw(float angle)
	{
		float cs = cos(angle*3.14159265 / 180);
		float sn = sin(angle*3.14159265 / 180);
		Vector3 t(n);
		Vector3 s(u);
		n.set(cs*t.x - sn*s.x, cs*t.y - sn*s.y, cs*t.z - sn*s.z);
		u.set(sn*t.x + cs*s.x, sn*t.y + cs*s.y, sn*t.z + cs*s.z);
		setModelViewMatrix();
	};
	void slide(float du, float dv, float dn)
	{
		eye.x += du*u.x + dv*v.x + dn*n.x;
		eye.y += du*u.y + dv*v.y + dn*n.y;
		eye.z += du*u.z + dv*v.z + dn*n.z;
		look.x += du*u.x + dv*v.x + dn*n.x;
		look.y += du*u.y + dv*v.y + dn*n.y;
		look.z += du*u.z + dv*v.z + dn*n.z;
		setModelViewMatrix();
	};
	float getDist()
	{
		float dist = pow(eye.x, 2) + pow(eye.y, 2) + pow(eye.z, 2);
		return pow(dist, 0.5f);
	};
	void setShape(float viewAngle, float aspect, float Near, float Far)
	{};

	float m[16];

private:
	/* 摄像机属性 */
	Point3	eye, look, up;
	Vector3	u, v, n;
	float   viewAngle, aspect, nearDist, farDist;
	void	setModelViewMatrix()
	{

		Vector3 eVec(eye.x, eye.y, eye.z);
		m[0] = u.x; m[1] = u.y; m[2] = u.z; m[3] = -eVec.dot(u);
		m[4] = v.x; m[5] = v.y; m[6] = v.z; m[7] = -eVec.dot(v);
		m[8] = n.x; m[9] = n.y; m[10] = n.z; m[11] = -eVec.dot(n);
		m[12] = 0;  m[13] = 0;  m[14] = 0;  m[15] = 1.0;
	};

};

#endif //__CAMERA_H__  