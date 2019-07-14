#pragma once
#include <stdio.h>
#include <math.h>

#define PI 3.141592653

struct vec2d
{
	union
	{
		struct
		{
			float x;
			float y;
		};
		struct
		{
			float xy[2];
		};
	};
	
	float& operator[](int i)
	{
		return this->xy[i];
	}
};

struct vec3d
{
	union
	{
		struct
		{
			float x;
			float y;
			float z;
		};
		struct
		{
			float xyz[3];
		};
		struct
		{
			float r;
			float g;
			float b;
		};
	};
	float& operator[](int i)
	{
		return this->xyz[i];
	}
};

struct vec4d
{
	union
	{
		struct
		{
			float x;
			float y;
			float z;
			float w;
		};
		struct
		{
			vec3d xyz;
		};
		struct
		{
			float xyzw[4];
		};
		struct
		{
			float r;
			float g;
			float b;
			float a;
		};
	};
	float& operator[](int i)
	{
		return this->xyzw[i];
	}
};

struct mat4
{
	vec4d _m[4];

	mat4& operator=(mat4 m)
	{
		float* p_this = (float*)this;
		float* p_m = (float*)&m;
		for(int i = 0; i < 16; i++) p_this[i] = p_m[i];
		return *this;
	}

	vec4d& operator[](int column)
	{
		return _m[column];
	}
};

struct mat3
{
	vec3d _m[3];

	mat3& operator=(mat3 m)
	{
		float* p_this = (float*)this;
		float* p_m = (float*)&m;
		for(int i = 0; i < 3; i++) p_this[i] = p_m[i];
		return *this;
	}

	vec3d& operator[](int column)
	{
		return _m[column];
	}
};

mat4 operator*(mat4,mat4);

mat4 identity();
mat4 translate(vec3d);
mat4 rotate_about_axis(vec3d, float);
mat4 scale(float);

mat4 orthographic_projection(float, float, float, float, float, float);
mat4 look_at(vec3d position, vec3d target, vec3d up);

float dot(vec3d, vec3d);
vec3d cross(vec3d, vec3d);

vec2d operator-(vec2d,vec2d);
vec2d operator+(vec2d,vec2d);

void print_matrix(mat4);
