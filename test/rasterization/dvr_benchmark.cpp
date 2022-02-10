#include <atomic>

#include <davinci.h>
using namespace davinci;

#include "dvr_benchmark.h"

struct Pixel
{
	std::atomic_flag flag;

	//Pixel() : flag(nullptr) {  }
	//~Pixel() { if(flag) delete flag; }
};

//-------------------------------------------------------------------------------------
class Edge
{
public:
	Edge(const Vector3 &_p1, const Vector3 &_p2) : p1(_p1), p2(_p2) {};
	Edge(const Edge &e) : p1(e.p1), p2(e.p2) {};

	Vector3 p1;
	Vector3 p2;

	inline bool operator == (const Edge& rkVector) const {
		return ( p1 == rkVector.p1 && p2 == rkVector.p2);
	}

	inline bool operator != (const Edge& rkVector) const {
		return ( p1 != rkVector.p1 || p2 != rkVector.p2);
	}
};

//-------------------------------------------------------------------------------------
inline static float _snapPosition(float pos)
{
	float x_8_f = (pos - floorf(pos))*8.f;
	float x_8_i = floorf(x_8_f);
	if ((x_8_f - x_8_i) > 0.5f) x_8_f = x_8_i + 1.f;
	else x_8_f = x_8_i;
	return floorf(pos) + x_8_f / 8.f;
}

//-------------------------------------------------------------------------------------
static float _generateFloat(float _min, float _max, bool snap)
{
	float f = (rand() / (float)RAND_MAX)*(_max - _min) + _min;
	if (snap) f = _snapPosition(f);

	return f;
}

//-------------------------------------------------------------------------------------
static bool _circumCircleContains(const Triangle& triangle, const Vector2 &v)
{
	float ab = triangle.p0.dotProduct(triangle.p0);
	float cd = triangle.p1.dotProduct(triangle.p1);
	float ef = triangle.p2.dotProduct(triangle.p2);

	float circum_x = (ab * (triangle.p2.y - triangle.p1.y) + cd * (triangle.p0.y - triangle.p2.y) + ef * (triangle.p1.y - triangle.p0.y)) / (triangle.p0.x * (triangle.p2.y - triangle.p1.y) + triangle.p1.x * (triangle.p0.y - triangle.p2.y) + triangle.p2.x * (triangle.p1.y - triangle.p0.y)) / 2.f;
	float circum_y = (ab * (triangle.p2.x - triangle.p1.x) + cd * (triangle.p0.x - triangle.p2.x) + ef * (triangle.p1.x - triangle.p0.x)) / (triangle.p0.y * (triangle.p2.x - triangle.p1.x) + triangle.p1.y * (triangle.p0.x - triangle.p2.x) + triangle.p2.y * (triangle.p1.x - triangle.p0.x)) / 2.f;
	float circum_radius = sqrtf(((triangle.p0.x - circum_x) * (triangle.p0.x - circum_x)) + ((triangle.p0.y - circum_y) * (triangle.p0.y - circum_y)));

	float dist = sqrtf(((v.x - circum_x) * (v.x - circum_x)) + ((v.y - circum_y) * (v.y - circum_y)));
	return dist <= circum_radius;
}

//-------------------------------------------------------------------------------------
void buildDelaunayTriangles(int32_t canvasWidth, int32_t canvasHeight, size_t pointCounts, TriangleVector& triangles)
{
	std::vector< Vector2 > vertices(pointCounts);

	//random points
	for (size_t i = 0; i < pointCounts; i++) {
		vertices[i].x = _generateFloat(0, (float)canvasWidth, false);
		vertices[i].y = _generateFloat(0, (float)canvasHeight, false);
	}
	//corner
	vertices.push_back(Vector2(0, 0));
	vertices.push_back(Vector2((float)canvasWidth, 0));
	vertices.push_back(Vector2((float)canvasWidth, (float)canvasHeight));
	vertices.push_back(Vector2(0, (float)canvasHeight));

	// Determinate the super triangle
	float minX = 0;
	float minY = 0;
	float maxX = (float)canvasWidth;
	float maxY = (float)canvasHeight;

	float dx = maxX - minX;
	float dy = maxY - minY;
	float deltaMax = std::max(dx, dy);
	float midx = (minX + maxX) / 2.f;
	float midy = (minY + maxY) / 2.f;

	Vector2 p1(midx - 20 * deltaMax, midy - deltaMax);
	Vector2 p2(midx, midy + 20 * deltaMax);
	Vector2 p3(midx + 20 * deltaMax, midy - deltaMax);

	printf("p1=<%f,%f>, p2=<%f,%f>, p3=<%f,%f>\n", p1.x, p1.y, p2.x, p2.y, p3.x, p3.y);

	// Create a list of triangles, and add the supertriangle in it
	triangles.push_back(Triangle(Vector3(p1, 0), Vector3(p2, 0), Vector3(p3, 0)));

	for (auto p = begin(vertices); p != end(vertices); p++)
	{
		printf("Traitement du point: <%f,%f>\n", p->x, p->y);
		printf("_triangles contains: %zd\n", triangles.size());

		//std::cout << "Traitement du point " << *p << std::endl;
		//std::cout << "_triangles contains " << _triangles.size() << " elements" << std::endl;	

		std::vector<Triangle> badTriangles;
		std::vector<Edge> polygon;

		for (auto t = begin(triangles); t != end(triangles); t++)
		{
			//std::cout << "Processing " << std::endl << *t << std::endl;

			if (_circumCircleContains(*t, *p))
			{
				//std::cout << "Pushing bad triangle " << *t << std::endl;
				badTriangles.push_back(*t);
				polygon.push_back(Edge(t->p0, t->p1));
				polygon.push_back(Edge(t->p1, t->p2));
				polygon.push_back(Edge(t->p2, t->p0));
			}
			else
			{
				//std::cout << " does not contains " << *p << " in his circum center" << std::endl;
			}
		}

		triangles.erase(std::remove_if(begin(triangles), end(triangles), [badTriangles](Triangle &t) {
			for (auto bt = begin(badTriangles); bt != end(badTriangles); bt++)
			{
				if (*bt == t)
				{
					//std::cout << "Removing bad triangle " << std::endl << *bt << " from _triangles" << std::endl;
					return true;
				}
			}
			return false;
		}), end(triangles));

		std::vector<Edge> badEdges;
		for (auto e1 = begin(polygon); e1 != end(polygon); e1++)
		{
			for (auto e2 = begin(polygon); e2 != end(polygon); e2++)
			{
				if (e1 == e2)
					continue;

				if (*e1 == *e2)
				{
					badEdges.push_back(*e1);
					badEdges.push_back(*e2);
				}
			}
		}

		polygon.erase(std::remove_if(begin(polygon), end(polygon), [badEdges](Edge &e) {
			for (auto it = begin(badEdges); it != end(badEdges); it++)
			{
				if (*it == e)
					return true;
			}
			return false;
		}), end(polygon));

		for (auto e = begin(polygon); e != end(polygon); e++)
			triangles.push_back(Triangle(e->p1, e->p2, Vector3(*p, 0)));

	}

	triangles.erase(std::remove_if(begin(triangles), end(triangles), [p1, p2, p3](Triangle &t) {
		return t.containsVertex(Vector3(p1, 0)) || t.containsVertex(Vector3(p2, 0)) || t.containsVertex(Vector3(p3, 0));
	}), end(triangles));

	//for (auto t = begin(triangles); t != end(triangles); t++)
	//{
	//	_edges.push_back(t->e1);
	//	_edges.push_back(t->e2);
	//	_edges.push_back(t->e3);
	//}
}
