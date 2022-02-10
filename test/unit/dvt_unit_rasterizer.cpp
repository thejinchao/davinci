#include <davinci.h>
#include <gtest/gtest.h>

#include <atomic>

using namespace davinci;

class Edge
{
public:

	Edge(const Vector2 &_p1, const Vector2 &_p2) : p1(_p1), p2(_p2) {};
	Edge(const Edge &e) : p1(e.p1), p2(e.p2) {};

	Vector2 p1;
	Vector2 p2;
};

//-------------------------------------------------------------------------------------
inline bool operator == (const Edge & e1, const Edge & e2)
{
	return 	(e1.p1 == e2.p1 && e1.p2 == e2.p2) ||
		(e1.p1 == e2.p2 && e1.p2 == e2.p1);
}

//-------------------------------------------------------------------------------------
class Triangle
{
public:

	Triangle(const Vector2 &_p1, const Vector2 &_p2, const Vector2 &_p3)
		: p1(_p1), p2(_p2), p3(_p3)
	{}

	bool containsVertex(const Vector2 &v)
	{
		return p1 == v || p2 == v || p3 == v;
	}

	bool circumCircleContains(const Vector2 &v)
	{
		float ab = (p1.x * p1.x) + (p1.y * p1.y);
		float cd = (p2.x * p2.x) + (p2.y * p2.y);
		float ef = (p3.x * p3.x) + (p3.y * p3.y);

		float circum_x = (ab * (p3.y - p2.y) + cd * (p1.y - p3.y) + ef * (p2.y - p1.y)) / (p1.x * (p3.y - p2.y) + p2.x * (p1.y - p3.y) + p3.x * (p2.y - p1.y)) / 2.f;
		float circum_y = (ab * (p3.x - p2.x) + cd * (p1.x - p3.x) + ef * (p2.x - p1.x)) / (p1.y * (p3.x - p2.x) + p2.y * (p1.x - p3.x) + p3.y * (p2.x - p1.x)) / 2.f;
		float circum_radius = sqrtf(((p1.x - circum_x) * (p1.x - circum_x)) + ((p1.y - circum_y) * (p1.y - circum_y)));

		float dist = sqrtf(((v.x - circum_x) * (v.x - circum_x)) + ((v.y - circum_y) * (v.y - circum_y)));
		return dist <= circum_radius;
	}

	Vector2 p1;
	Vector2 p2;
	Vector2 p3;
};

//-------------------------------------------------------------------------------------
inline bool operator == (const Triangle &t1, const Triangle &t2)
{
	return	(t1.p1 == t2.p1 || t1.p1 == t2.p2 || t1.p1 == t2.p3) &&
		(t1.p2 == t2.p1 || t1.p2 == t2.p2 || t1.p2 == t2.p3) &&
		(t1.p3 == t2.p1 || t1.p3 == t2.p2 || t1.p3 == t2.p3);
}

//-------------------------------------------------------------------------------------
class Delaunay
{
	/*
	https://github.com/Bl4ckb0ne/delaunay-triangulation
	*/
public:
	static void triangulate(const std::vector<Vector2> & vertices, std::vector<Triangle>& _triangles)
	{
		// Determinate the super triangle
		float minX = vertices[0].x;
		float minY = vertices[0].y;
		float maxX = minX;
		float maxY = minY;

		for (std::size_t i = 0; i < vertices.size(); ++i)
		{
			if (vertices[i].x < minX) minX = vertices[i].x;
			if (vertices[i].y < minY) minY = vertices[i].y;
			if (vertices[i].x > maxX) maxX = vertices[i].x;
			if (vertices[i].y > maxY) maxY = vertices[i].y;
		}

		float dx = maxX - minX;
		float dy = maxY - minY;
		float deltaMax = std::max(dx, dy);
		float midx = (minX + maxX) / 2.f;
		float midy = (minY + maxY) / 2.f;

		Vector2 p1(midx - 20 * deltaMax, midy - deltaMax);
		Vector2 p2(midx, midy + 20 * deltaMax);
		Vector2 p3(midx + 20 * deltaMax, midy - deltaMax);

		// Create a list of triangles, and add the supertriangle in it
		_triangles.push_back(Triangle(p1, p2, p3));

		for (auto p = begin(vertices); p != end(vertices); p++) {
			std::vector<Triangle> badTriangles;
			std::vector<Edge> polygon;

			for (auto t = begin(_triangles); t != end(_triangles); t++) {
				if (t->circumCircleContains(*p)) {
					badTriangles.push_back(*t);

					polygon.push_back(Edge(t->p1, t->p2));
					polygon.push_back(Edge(t->p2, t->p3));
					polygon.push_back(Edge(t->p3, t->p1));
				}
			}

			_triangles.erase(std::remove_if(begin(_triangles), end(_triangles), [badTriangles](Triangle &t) {
				for (auto bt = begin(badTriangles); bt != end(badTriangles); bt++) {
					if (*bt == t) {
						return true;
					}
				}
				return false;
			}), end(_triangles));

			std::vector<Edge> badEdges;
			for (auto e1 = begin(polygon); e1 != end(polygon); e1++) {
				for (auto e2 = begin(polygon); e2 != end(polygon); e2++) {
					if (e1 == e2) continue;

					if (*e1 == *e2) {
						badEdges.push_back(*e1);
						badEdges.push_back(*e2);
					}
				}
			}

			polygon.erase(std::remove_if(begin(polygon), end(polygon), [badEdges](Edge &e) {
				for (auto it = begin(badEdges); it != end(badEdges); it++) {
					if (*it == e) return true;
				}
				return false;
			}), end(polygon));

			for (auto e = begin(polygon); e != end(polygon); e++)
				_triangles.push_back(Triangle(e->p1, e->p2, *p));
		}

		_triangles.erase(std::remove_if(begin(_triangles), end(_triangles), [p1, p2, p3](Triangle &t) {
			return t.containsVertex(p1) || t.containsVertex(p2) || t.containsVertex(p3);
		}), end(_triangles));
	}
};

//-------------------------------------------------------------------------------------
struct Pixel
{
	int32_t x, y;
};
typedef std::vector<Pixel> PixelBuf;

//-------------------------------------------------------------------------------------
class Canvas : noncopyable
{
public:
	Canvas() : m_width(0), m_height(0), m_pixelBuf(nullptr), m_errorFlag(false) { }
	~Canvas() {
		delete[] m_pixelBuf;
		m_pixelBuf = nullptr;
	}

	int32_t getWidth(void) const { return m_width; }
	int32_t getHeight(void) const { return m_height; }

	void init(int32_t width, int32_t height) {
		if (m_width != width || m_height != height) {
			if (m_pixelBuf) delete[] m_pixelBuf;

			m_width = width;
			m_height = height;
			//alloc pixels
			m_pixelBuf = new std::atomic<uint32_t>[m_width*m_height];
		}

		for (size_t i = 0; i < (size_t)(m_width*m_height); i++) {
			m_pixelBuf[i] = 0;
		}
		m_errorFlag = false;
	}

	void clear(void) {
		for (size_t i = 0; i < (size_t)(m_width*m_height); i++) {
			m_pixelBuf[i] = 0;
		}
		m_errorFlag = false;
	}

	void razFunction_fetchadd(const std::pair<int32_t, int32_t>& pixel, const Vector3& b) {
		if (m_errorFlag.load()) return;

		if (pixel.first < 0 || pixel.first >= m_width || pixel.second < 0 || pixel.second >= m_height) {
			bool _false = false;
			m_errorFlag.compare_exchange_strong(_false, true);
			return;
		}
		m_pixelBuf[pixel.second*m_width + pixel.first].fetch_add(1);
	}

	void razFunction(const std::pair<int32_t, int32_t>& pixel, const Vector3& b, uint32_t randCol) {
		if (m_errorFlag.load()) return;

		if (pixel.first < 0 || pixel.first >= m_width || pixel.second < 0 || pixel.second >= m_height) {
			bool _false = false;
			m_errorFlag.compare_exchange_strong(_false, true);
			return;
		}
		m_pixelBuf[pixel.second*m_width + pixel.first] = randCol;
	}

	bool checkResult(bool debug=false) {
		if (m_errorFlag) return false;

		if (debug) {
			bool succ = true;
			uint32_t* colBuf = new uint32_t[m_width*m_height];

			for (int32_t i = 0; i < m_width*m_height; i++) {
				colBuf[i] = m_pixelBuf[i].load();
				if (m_pixelBuf[i] == 0) {
					if (debug) printf("error: %d,%d=%d\n", i%m_width, i/ m_width, m_pixelBuf[(size_t)i].load());
					succ = false;
					//return false;
				}
			}

			AssetUtility::savePixelBufferToPNG("debug.png", m_width, m_height, colBuf, PF_UINT8_RGBA);
			delete[] colBuf;
			return succ;
		}
		else {
			for (size_t i = 0; i < (size_t)(m_width*m_height); i++) {
				if (m_pixelBuf[i] == 0) return false;
			}
		}
		return true;
	}

	bool checkResult(const PixelBuf& pixelExpected) {
		if (m_errorFlag) return false;

		for (const auto& pixel : pixelExpected) {
			assert(pixel.x >= 0 && pixel.x < m_width && pixel.y >= 0 && pixel.y < m_height);
			auto& p = m_pixelBuf[pixel.y*m_width + pixel.x];
			if (p != 1) return false;
			p = 0;
		}
		for (size_t i = 0; i < (size_t)(m_width*m_height); i++) {
			if (m_pixelBuf[i] != 0) return false;
		}
		return true;
	}

private:
	int32_t m_width, m_height;
	std::atomic<uint32_t>* m_pixelBuf;

	std::atomic<bool> m_errorFlag;
	std::string m_strError;
};

//-------------------------------------------------------------------------------------
static bool _triangleRasterizationRules(Canvas& canvas, const Triangle& triangle, const PixelBuf& pixelExpect, bool debug=false)
{
	auto razFunc = std::bind(&Canvas::razFunction_fetchadd, &canvas, std::placeholders::_1, std::placeholders::_2);
	
	canvas.clear();

	Rasterizer::drawTriangleLarrabee(canvas.getWidth(), canvas.getHeight(),
		Vector3(triangle.p1, 0), Vector3(triangle.p2, 0), Vector3(triangle.p3, 0),
		razFunc);

	return debug ? true : canvas.checkResult(pixelExpect);
}

//-------------------------------------------------------------------------------------
TEST(Rasterizer, Basic)
{
	return;
	const int32_t canvasWidth = 64, canvasHeight = 64;

	Canvas canvas;
	canvas.init(canvasWidth, canvasHeight);

	/*
		Triangle Rasterization rules test(without Without Multisampling)
		https://msdn.microsoft.com/en-us/library/windows/desktop/cc627092(v=vs.85).aspx
	*/
	{
		Triangle triangle(Vector2(4.5f, 7.5f), Vector2(4.5f, 7.5f), Vector2(4.5f, 7.5f));
		PixelBuf pixels = {};
		EXPECT_TRUE(_triangleRasterizationRules(canvas, triangle, pixels));
	}
	{
		Triangle triangle(Vector2(1, 2), Vector2(7, 4), Vector2(5, 2));
		PixelBuf pixels = { {2, 2}, {3, 2}, {4, 2}, {5, 3} };
		EXPECT_TRUE(_triangleRasterizationRules(canvas, triangle, pixels));
	}
	{
		Triangle triangle(Vector2(5, 2), Vector2(7, 4), Vector2(8, 1));
		PixelBuf pixels = { { 5, 2 },{ 6, 1 },{ 6, 2 },{ 6, 3 }, {7, 1} };
		EXPECT_TRUE(_triangleRasterizationRules(canvas, triangle, pixels));
	}
	{
		Triangle triangle(Vector2(8, 1), Vector2(7, 4), Vector2(9.5f, 2.5f));
		PixelBuf pixels = { { 7, 2 },{ 7, 3 }, { 8, 2 } };
		EXPECT_TRUE(_triangleRasterizationRules(canvas, triangle, pixels));
	}
	{
		Triangle triangle(Vector2(1, 7), Vector2(6, 6), Vector2(2, 4));
		PixelBuf pixels = { { 2, 4 }, {1, 5 }, { 2, 5 }, { 3, 5 }, { 4, 5 }, { 1, 6 }, { 2, 6 }, };
		EXPECT_TRUE(_triangleRasterizationRules(canvas, triangle, pixels));
	}
	{
		Triangle triangle(Vector2(5.25f, 6.75f), Vector2(6.25f, 7.75f), Vector2(6.25f, 6.75f));
		PixelBuf pixels = {  };
		EXPECT_TRUE(_triangleRasterizationRules(canvas, triangle, pixels));
	}
	{
		Triangle triangle(Vector2(6.5f, 5.5f), Vector2(7.5f, 7.5f), Vector2(7.5f, 6.5f));
		PixelBuf pixels = {};
		EXPECT_TRUE(_triangleRasterizationRules(canvas, triangle, pixels));
	}
	{
		Triangle triangle(Vector2(7.6f, 5.5f), Vector2(9.75f, 7.25f), Vector2(11.6f, 5.5f));
		PixelBuf pixels = { { 9,6 },{ 10, 6 } };
		EXPECT_TRUE(_triangleRasterizationRules(canvas, triangle, pixels));
	}
	{
		Triangle triangle(Vector2(7.6f, 5.5f), Vector2(11.6f, 5.5f), Vector2(9.5f, 2.6f));
		PixelBuf pixels = { {8, 5}, { 9, 5 }, { 10, 5 }, { 11, 5 }, { 8, 4 }, { 9, 4 }, { 10, 4 }, {9, 3} };
		EXPECT_TRUE(_triangleRasterizationRules(canvas, triangle, pixels));
	}
	{
		Triangle triangle(Vector2(11.5f, 1.5f), Vector2(11.5f, 3.5f), Vector2(12.5f, 2.5f));
		PixelBuf pixels = { {11,2} };
		EXPECT_TRUE(_triangleRasterizationRules(canvas, triangle, pixels));
	}
	{
		Triangle triangle(Vector2(9.5f, 0.5f), Vector2(10.5f, 0.5f), Vector2(9.5f, -1.5f));
		PixelBuf pixels = { { 9,0  } };
		EXPECT_TRUE(_triangleRasterizationRules(canvas, triangle, pixels));
	}
	{
		Triangle triangle(Vector2(13.5f, 0.5f), Vector2(13.5f, 2.5f), Vector2(15.5f, 2.5f));
		PixelBuf pixels = { { 13,1 }, {13, 2}, {14, 2} };
		EXPECT_TRUE(_triangleRasterizationRules(canvas, triangle, pixels));
	}
	{
		Triangle triangle(Vector2(13.5f, 0.5f), Vector2(15.5f, 2.5f), Vector2(15.5f, 0.5f));
		PixelBuf pixels = { { 14, 1 } };
		EXPECT_TRUE(_triangleRasterizationRules(canvas, triangle, pixels));
	}
	{
		Triangle triangle(Vector2(13.5f, 6.5f), Vector2(14.5f, 5.5f), Vector2(14.5f, 3.5f));
		PixelBuf pixels = {  };
		EXPECT_TRUE(_triangleRasterizationRules(canvas, triangle, pixels));
	}
	{
		Triangle triangle(Vector2(13.5f, 6.5f), Vector2(15, 8), Vector2(14.5f, 5.5f) );
		PixelBuf pixels = { {13, 6}, {14, 6}, {14, 7} };
		EXPECT_TRUE(_triangleRasterizationRules(canvas, triangle, pixels));
	}
}

//-------------------------------------------------------------------------------------
TEST(Rasterizer, Delaunay)
{
	//seed=1499743811, counts=5, range=0.8f;
	//seed=1499670829, counts=5, range=1.f;

	const int32_t width = 1024, height = 1024;
	const int32_t verticesCounts = MathUtil::rangeRandom(5, 30);

	float range = 0.8f;
	std::vector<Vector2> vertices;
	for (int32_t i = 0; i < verticesCounts; i++) {
		vertices.push_back(Vector2(MathUtil::rangeRandom(width*(1-range)/2.f, width*(1+range)/2.f), MathUtil::rangeRandom(height*(1 - range) / 2.f, height*(1 + range) / 2.f)));
	}
	vertices.push_back(Vector2(0, 0));
	vertices.push_back(Vector2((float)width, 0));
	vertices.push_back(Vector2((float)width, (float)height));
	vertices.push_back(Vector2(0, (float)height));

	std::vector<Triangle> triangles;
	Delaunay::triangulate(vertices, triangles);	

	Canvas canvas;
	canvas.init(width, height);
	canvas.clear();

	for (size_t i = 0; i < triangles.size(); i++) {
		const Triangle& triangle = triangles[i];

		uint32_t randCol = 0xFF000000 + ((uint32_t)(rand() / 240) << 16) + ((uint32_t)(rand() / 240) << 8) + ((uint32_t)(rand() / 240));

		auto razFunc = std::bind(&Canvas::razFunction, &canvas, std::placeholders::_1, std::placeholders::_2, randCol);
		Rasterizer::drawTriangleLarrabee(canvas.getWidth(), canvas.getHeight(),
			Vector3(triangle.p1, 0), Vector3(triangle.p2, 0), Vector3(triangle.p3, 0),
			razFunc);
	}
	EXPECT_TRUE(canvas.checkResult(true));
}

