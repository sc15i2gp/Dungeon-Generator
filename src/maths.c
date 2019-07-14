#include "maths.h"

mat4 operator*(mat4 m_0, mat4 m_1)
{
	mat4 m_2 = {};
	for(int col = 0; col < 4; col++)
	{
		for(int row = 0; row < 4; row++)
		{
			for(int i = 0; i < 4; i++)
			{
				m_2[col][row] += m_0[i][row]*m_1[col][i];
			}
		}
	}
	return m_2;
}

mat4 identity()
{
	mat4 m;
	for(int i = 0; i < 4; i++)
	{
		for(int j = 0; j < 4; j++)
		{
			if(i == j) 
			{
				m[i][j] = 1.0f;
			}
			else m[i][j] = 0.0f;
		}
	}
	return m;
}

mat4 translate(vec3d v)
{
	mat4 m = identity();
	m[3].xyz = v;
	return m;
}

mat4 rotate_about_axis(vec3d axis, float angle)
{
	mat4 rotation = identity();
	float c_angle = 1.0f - cos(angle);
	float s_angle = sin(angle);

	for(int i = 0; i < 3; i++) rotation[i][i] = cos(angle) + axis[i]*axis[i]*c_angle;

	float element_c_term = 0.0f;
	float element_s_term = 0.0f;
	
	element_c_term = axis.x*axis.y*c_angle;
	element_s_term = axis.z*s_angle;
	rotation[0][1] = element_c_term + element_s_term;
	rotation[1][0] = element_c_term - element_s_term;

	element_c_term = axis.z*axis.x*c_angle;
	element_s_term = axis.y*s_angle;
	rotation[0][2] = element_c_term - element_s_term;
	rotation[2][0] = element_c_term + element_s_term;

	element_c_term = axis.y*axis.z*c_angle;
	element_s_term = axis.x*s_angle;
	rotation[1][2] = element_c_term + element_s_term;
	rotation[2][1] = element_c_term - element_s_term;

	return rotation;
}

mat4 scale(float factor)
{
	mat4 s = identity();
	for(int i = 0; i < 3; i++) s[i][i] = factor;
	return s;
}

mat4 orthographic_projection(float left, float right, float bottom, float top, float front, float back)
{
	mat4 m = identity();
	m[0][0] = 2.0f/(right-left);
	m[1][1] = 2.0f/(top-bottom);
	m[2][2] = 2.0f/(back-front);
	
	m[3][0] = -(right+left)/(right-left);
	m[3][1] = -(top+bottom)/(top-bottom);
	m[3][2] = -(back+front)/(back-front);
	return m;
}

float dot(vec3d v_0, vec3d v_1)
{
	float d = 0.0f;
	for(int i = 0; i < 3; i++) d += v_0[i]*v_1[i];
	return d;
}

vec3d cross(vec3d v_0, vec3d v_1)
{
	vec3d c = {};
	c.x = v_0.y*v_1.z - v_0.z*v_1.y;
	c.y = v_0.z*v_1.x - v_0.x*v_1.z;
	c.z = v_0.x*v_1.y - v_0.y*v_1.x;
	return c;
}

vec2d operator-(vec2d v_0, vec2d v_1)
{
	vec2d v;
	v.x = v_0.x - v_1.x;
	v.y = v_0.y - v_1.y;
	return v;
}

vec2d operator+(vec2d v_0, vec2d v_1)
{
	vec2d v;
	v.x = v_0.x + v_1.x;
	v.y = v_0.y + v_1.y;
	return v;
}

void print_matrix(mat4 m)
{
	for(int i = 0; i < 4; i++)
	{
		printf("%.3f | %.3f | %.3f | %.3f\n", m[0][i], m[1][i], m[2][i], m[3][i]);
		if(i < 3) printf("------------------------------\n");
	}
	printf("\n");
}
