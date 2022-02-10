#include <davinci.h>
using namespace davinci;

#include <GLFW/glfw3.h>

#include "dvr_rasterizer.h"
#include "dvr_triangle.h"
#include "dvr_benchmark.h"

//-------------------------------------------------------------------------------------
enum MouseAction
{
	MT_NULL,
	MT_FIRST_LINE,
	MT_TRIANGLE
};

//-------------------------------------------------------------------------------------
enum ELEMENT_DEPTH
{
	ED_GRID_BORDER = 0,
	ED_TRIANGLE_PIXEL = 1,
	ED_TRIANGLE_BORDER = 2,
	ED_ACTION = 3,
	ED_CURSOR = 4
};

//-------------------------------------------------------------------------------------
const float		MIN_SCALE = 4.f;
const float		MAX_SCALE = 60.f;
const float		TRIANGLE_BORDER_WIDTH = 1.5f;
const float		ACTION_WIDTH = 1.f;
const Vector3	GRID_BORDER_COLOR = Vector3(0.f, 0.f, 0.f);
const Vector3	GRID_CROSS_COLOR = Vector3(0.8f, 0.8f, 0.8f);
const Vector3	TRIANGLE_BORDER_COLOR = Vector3(0.f, 0.f, 0.f);
const Vector3	ACTION_COLOR = Vector3(1.f, 0.f, 0.f);

//-------------------------------------------------------------------------------------
int				g_viewWidth = 1024;
int				g_viewHeight = 720;
int				g_canvasWidth;
int				g_canvasHeight;
float			g_scale = MAX_SCALE;
int				g_lbCorner_x = 20;
int				g_lbCorner_y = 20;
bool			g_bNeedRedraw = true;
MouseAction		g_currentAction = MT_NULL;
float			g_posX, g_posY;
bool			g_bSnapMode = false;
float			g_fSnapSize = 0.125f;
int				g_nDrawGridLevel = 0; //0:all 1:fine+coarse+tile  2:coarse+tile 3: tile 
Triangle		g_currentTriangle;
TriangleVector	g_allTriangles;
bool			g_bDrawExtension = false;

//-------------------------------------------------------------------------------------
static void _onFramebufferSize(GLFWwindow* window, int w, int h)
{
	g_viewWidth = w;
	g_viewHeight = h > 0 ? h : 1;

	g_canvasWidth  = ((int32_t)floorf(g_viewWidth / (g_scale * 64))+1) * 64;
	g_canvasHeight = ((int32_t)floorf(g_viewHeight / (g_scale * 64))+1) * 64;

	g_nDrawGridLevel = 0;
	if (g_scale < 10.f) g_nDrawGridLevel = 3;
	else if (g_scale < 20.f) g_nDrawGridLevel = 2;
	else if (g_scale < 50.f) g_nDrawGridLevel = 1;

	g_bNeedRedraw = true;
}

//-------------------------------------------------------------------------------------
static void _drawGrid(void)
{
	glDisable(GL_LINE_SMOOTH);
	glDisable(GL_BLEND);

	// Set grid color
	glLineWidth(1.f);

	int32_t horLength = (int32_t)floorf(g_viewWidth / g_scale);
	int32_t verLength = (int32_t)floorf(g_viewHeight / g_scale);
	float lineDepth = ED_GRID_BORDER / 10.f;

	Vector3 lineColor[] = { Vector3(0.f, 0.f, 0.f), Vector3(0.5f, 0.5f, 0.5f), Vector3(0.7f,0.7f,0.7f), Vector3(0.9f, 0.9f,0.9f) };

	glBegin(GL_LINES);
	// Horizontal lines
	for (int i = 0; i <= verLength; i++) {
		int level = 3;

		if (i % 64 == 0)level = 0;
		else if (i % 16 == 0) level = 1;
		else if (i % 4 == 0) level = 2;
		if (level > 3 - g_nDrawGridLevel) continue;

		glColor3fv(lineColor[level].ptr());

		glVertex3f(0, (float)i, lineDepth);
		glVertex3f((float)horLength, (float)i, lineDepth);
	}

	// Vertical lines
	for (int j = 0; j <= horLength; j++) {
		int level = 3;

		if (j % 64 == 0)level = 0;
		else if (j % 16 == 0) level = 1;
		else if (j % 4 == 0) level = 2;
		if (level > 3 - g_nDrawGridLevel) continue;

		glColor3fv(lineColor[level].ptr());

		glVertex3f((float)j, 0, lineDepth);
		glVertex3f((float)j, (float)verLength, lineDepth);
	}

	//cross
	if (g_nDrawGridLevel < 3) {
		glColor3f(GRID_CROSS_COLOR.x, GRID_CROSS_COLOR.y, GRID_CROSS_COLOR.z);
		float crossLength = 0.2f;
		for (int i = 0; i < verLength; i++) {
			for (int j = 0; j < horLength; j++) {
				glVertex3f(j + 0.5f - crossLength, i + 0.5f - crossLength, lineDepth);
				glVertex3f(j + 0.5f + crossLength, i + 0.5f + crossLength, lineDepth);

				glVertex3f(j + 0.5f - crossLength, i + 0.5f + crossLength, lineDepth);
				glVertex3f(j + 0.5f + crossLength, i + 0.5f - crossLength, lineDepth);
			}
		}
	}

	glEnd();
}

//-------------------------------------------------------------------------------------
static void _drawTrianglePixel(const Triangle& triangle, float depth)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(triangle.pixelColor[0], triangle.pixelColor[1], triangle.pixelColor[2], 0.5f);
	glBegin(GL_QUADS);
	for (const auto& dot : triangle.pixels) {
		float x = (float)dot.first;
		float y = (float)dot.second;

		glVertex3f(x, y, depth);
		glVertex3f(x + 1.f, y, depth);
		glVertex3f(x + 1.f, y + 1.f, depth);
		glVertex3f(x, y + 1.f, depth);
	}
	glEnd();

	glDisable(GL_LINE_SMOOTH);
	glDisable(GL_BLEND);
}

//-------------------------------------------------------------------------------------
static void _drawTriangleBorder(const Triangle& triangle, const Vector3& color, float lineWidth, float depth)
{
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH, GL_NICEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glLineWidth(lineWidth);

	glColor3f(color.x, color.y, color.z);
	glBegin(GL_LINE_LOOP);
	glVertex3f(triangle.p0[0], triangle.p0[1], depth);
	glVertex3f(triangle.p1[0], triangle.p1[1], depth);
	glVertex3f(triangle.p2[0], triangle.p2[1], depth);
	glEnd();

	glDisable(GL_LINE_SMOOTH);
	glDisable(GL_BLEND);
}

//-------------------------------------------------------------------------------------
static void _drawTriangles(void)
{
	float pixelDepth = (float)ED_TRIANGLE_PIXEL / 10.f;

	for (const auto& triangle : g_allTriangles) {
		_drawTrianglePixel(triangle, pixelDepth);
	}

	float borderDepth = (float)ED_TRIANGLE_BORDER / 10.f;
	for (const auto& triangle : g_allTriangles) {
		_drawTriangleBorder(triangle, TRIANGLE_BORDER_COLOR, TRIANGLE_BORDER_WIDTH, borderDepth);
	}
}

//-------------------------------------------------------------------------------------
static void _drawSegmentInAABB(const Vector2& p0, const Vector2& p1, float xMin, float xMax, float yMin, float yMax)
{
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH, GL_NICEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glLineWidth(ACTION_WIDTH);

	glColor3f(0.7f, 0.7f, 0.7f);

	float actionDepth = ED_ACTION / 10.f;

	float dx = p1.x - p0.x;
	float dy = p1.y - p0.y;
	if (std::abs(dx) > std::abs(dy)) {
		float _ymin = (xMin - p0.x)*dy / dx + p0.y;
		float _ymax = (xMax - p0.x)*dy / dx + p0.y;

		glBegin(GL_LINES);
		glVertex3f(xMin, _ymin, actionDepth);
		if (dx > 0) {
			glVertex3f(p0.x, p0.y, actionDepth);
			glVertex3f(p1.x, p1.y, actionDepth);
		}
		else {
			glVertex3f(p1.x, p1.y, actionDepth);
			glVertex3f(p0.x, p0.y, actionDepth);
		}
		glVertex3f(xMax, _ymax, actionDepth);
		glEnd();
	}
	else {
		float _xmin = (yMin - p0.y)*dx / dy + p0.x;
		float _xmax = (yMax - p0.y)*dx / dy + p0.x;

		glBegin(GL_LINES);
		glVertex3f(_xmin, yMin, actionDepth);
		if (dy > 0) {
			glVertex3f(p0.x, p0.y, actionDepth);
			glVertex3f(p1.x, p1.y, actionDepth);
		}
		else {
			glVertex3f(p1.x, p1.y, actionDepth);
			glVertex3f(p0.x, p0.y, actionDepth);
		}
		glVertex3f(_xmax, yMax, actionDepth);
		glEnd();
	}
}

//-------------------------------------------------------------------------------------
static void _drawAction(void)
{
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH, GL_NICEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glLineWidth(ACTION_WIDTH);

	glColor3f(ACTION_COLOR.x, ACTION_COLOR.y, ACTION_COLOR.z);

	float actionDepth = ED_ACTION / 10.f;

	if (g_bDrawExtension) {
		_drawSegmentInAABB(g_currentTriangle.p0.xy(), g_currentTriangle.p1.xy(), 0, (float)g_canvasWidth, 0, (float)g_canvasHeight);
		_drawSegmentInAABB(g_currentTriangle.p1.xy(), g_currentTriangle.p2.xy(), 0, (float)g_canvasWidth, 0, (float)g_canvasHeight);
		_drawSegmentInAABB(g_currentTriangle.p2.xy(), g_currentTriangle.p0.xy(), 0, (float)g_canvasWidth, 0, (float)g_canvasHeight);
	}

	switch (g_currentAction) {
	case MT_FIRST_LINE:
	{
		glBegin(GL_LINES);
			glVertex3f(g_currentTriangle.p0.x, g_currentTriangle.p0.y, actionDepth);
			glVertex3f(g_currentTriangle.p1.x, g_currentTriangle.p1.y, actionDepth);
		glEnd();
	}
	break;

	case MT_TRIANGLE:
	{
		g_currentTriangle.build(ACTION_COLOR, g_canvasWidth, g_canvasHeight);

		_drawTrianglePixel(g_currentTriangle, actionDepth);
		_drawTriangleBorder(g_currentTriangle, ACTION_COLOR, ACTION_WIDTH, actionDepth);
	}
	break;

	default: break;
	}

}

//-------------------------------------------------------------------------------------
static void _drawMouseCursor(void)
{
	glLineWidth(1.f);
	glColor3f(1.0f, 1.0f, 1.0f);
	int cursorLength = 10;

	glEnable(GL_COLOR_LOGIC_OP);
	glLogicOp(GL_XOR);

	float cursorDepth = ED_CURSOR / 10.f;
	glBegin(GL_LINES);
		glVertex3f(g_posX - cursorLength / g_scale, g_posY, cursorDepth);
		glVertex3f(g_posX + cursorLength / g_scale, g_posY, cursorDepth);

		glVertex3f(g_posX, g_posY - cursorLength / g_scale, cursorDepth);
		glVertex3f(g_posX, g_posY + cursorLength / g_scale, cursorDepth);
	glEnd();

	glDisable(GL_COLOR_LOGIC_OP);
}

//-------------------------------------------------------------------------------------
static void _onWindowRefresh(GLFWwindow* window)
{
	// Clear screen
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	// Setup projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-g_lbCorner_x/g_scale, (g_viewWidth- g_lbCorner_x )/g_scale, -g_lbCorner_y/g_scale, (g_viewHeight- g_lbCorner_y)/g_scale, -10, 10);

	// Setup view matrix
	glViewport(0, 0, g_viewWidth, g_viewHeight);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//draw grid
	_drawGrid();

	//draw triangles
	_drawTriangles();

	//draw action
	_drawAction();

	//draw mouse
	_drawMouseCursor();

	glfwSwapBuffers(window);
	g_bNeedRedraw = false;
}

//-------------------------------------------------------------------------------------
inline static float _snapPosition(float pos)
{
	if (g_fSnapSize < 1) {
		int invSize = (int)(1.f / g_fSnapSize);

		float x_f = floorf(((pos - floorf(pos))*invSize + 0.5f))/(float)invSize;
		return floorf(pos) + x_f;
	}
	else {
		return floorf(pos - floorf(pos / g_fSnapSize) + 0.5f) * g_fSnapSize;
	}
}

//-------------------------------------------------------------------------------------
static void _onCursorPos(GLFWwindow* window, double x, double y)
{
	int wnd_width, wnd_height, fb_width, fb_height;

	glfwGetWindowSize(window, &wnd_width, &wnd_height);
	glfwGetFramebufferSize(window, &fb_width, &fb_height);

	float scale = (float)fb_width / (float)wnd_width;
	
	g_posX = (float)(x*scale - g_lbCorner_x) / g_scale;
	g_posY = (float)(fb_height - 1 - y*scale - g_lbCorner_y)/g_scale;

	if (g_bSnapMode) {
		g_posX = _snapPosition(g_posX);
		g_posY = _snapPosition(g_posY);
	}

	switch (g_currentAction)
	{
	case MT_FIRST_LINE:
	{
		g_currentTriangle.p1 = Vector3(g_posX, g_posY, 0);
		g_bNeedRedraw = true;
	}
	break;

	case MT_TRIANGLE:
	{
		g_currentTriangle.p2 = Vector3(g_posX, g_posY, 0);
		g_bNeedRedraw = true;
	}
	break;

	default: break;
	}
	g_bNeedRedraw = true;
}

//-------------------------------------------------------------------------------------
static void _onMouseButton(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
	{
		switch (g_currentAction) {
		case MT_NULL:
		{
			g_currentTriangle.p0 = g_currentTriangle.p1 = Vector3(g_posX, g_posY, 0);

			g_currentAction = MT_FIRST_LINE;
			g_bDrawExtension = false;
		}
		break;

		case MT_FIRST_LINE:
		{
			g_currentTriangle.p1 = g_currentTriangle.p2 = Vector3(g_posX, g_posY, 0);

			g_currentAction = MT_TRIANGLE;
			g_bDrawExtension = true;
		}
		break;

		case MT_TRIANGLE:
		{
			g_currentTriangle.p2 = Vector3(g_posX, g_posY, 0);
			g_currentAction = MT_NULL;

			//build triangle
			Vector3 randColor = Vector3(rand()*0.8f / RAND_MAX, rand()*0.8f / RAND_MAX, rand()*0.8f / RAND_MAX);

			g_currentTriangle.build(randColor, g_canvasWidth, g_canvasHeight, true);
			g_allTriangles.push_back(g_currentTriangle);
			g_bDrawExtension = true;
		}
		break;

		default: break;
		}
	}
	else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
	{
		g_currentAction = MT_NULL;
		g_bDrawExtension = false;
	}

	g_bNeedRedraw = true;
}

//-------------------------------------------------------------------------------------
static void _onMouseScroll(GLFWwindow* window, double xoffset, double yoffset)
{
	g_scale += (float)yoffset*g_scale*0.05f;
	if (g_scale < MIN_SCALE) g_scale = MIN_SCALE;
	if (g_scale > MAX_SCALE) g_scale = MAX_SCALE;

	_onFramebufferSize(window, g_viewWidth, g_viewHeight);
}

//-------------------------------------------------------------------------------------
static void _onKeyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);

	if (key == GLFW_KEY_S && action == GLFW_PRESS)
		g_bSnapMode = !g_bSnapMode;

	if (key == GLFW_KEY_C && action == GLFW_PRESS) {
		g_allTriangles.clear();
		g_bNeedRedraw = true;
		g_bDrawExtension = false;
	}
}

//-------------------------------------------------------------------------------------
static void _buildDemoTriangles(void)
{
	// https://msdn.microsoft.com/en-us/library/windows/desktop/cc627092(v=vs.85).aspx

	float triangleData[] = {
		1, 2,7, 4,5, 2,
		5, 2,7, 4,8, 1,
		8, 1,7, 4,9.5f, 2.5f,
		1, 7,6, 6,2, 4,
		5.25f, 6.75f, 6.25f, 7.75,6.25f, 6.75f,
		6.5f, 5.5f,7.5f, 7.5f,7.5f, 6.5f,
		11.5f, 1.5f,11.5f, 3.5f, 12.5f, 2.5f,
		13.5f, 0.5f,13.5f, 2.5f, 15.5f, 2.5f,
		13.5f, 0.5f,15.5f, 2.5f, 15.5f, 0.5f,
		13.5f, 6.5f,14.5f, 5.5f, 14.5f, 3.5f,
		13.5f, 6.5f,15, 8, 14.5f, 5.5f,
		9.5f, 0.5f, 10.5f, 0.5f,9.5f, -1.5f,
		7.6f, 5.5f,9.75f, 7.25f, 11.6f, 5.5f,
		7.6f, 5.5f, 11.6f, 5.5f, 9.5f, 2.6f,
	};

	size_t triangleCounts = sizeof(triangleData) / (6 * sizeof(float));
	for (size_t i = 0; i < triangleCounts; i++) {

		Vector3 p0(triangleData[i * 6 + 0], triangleData[i * 6 + 1], 0.f);
		Vector3 p1(triangleData[i * 6 + 2], triangleData[i * 6 + 3], 0.f);
		Vector3 p2(triangleData[i * 6 + 4], triangleData[i * 6 + 5], 0.f);
		Vector3 randColor = Vector3(rand()*0.8f / RAND_MAX, rand()*0.8f / RAND_MAX, rand()*0.8f / RAND_MAX);

		Triangle triangle(p0, p1, p2);
		triangle.build(randColor, g_canvasWidth, g_canvasHeight, false);
		g_allTriangles.emplace_back(triangle);
	}
}
//-------------------------------------------------------------------------------------
void foo(void)
{
	Rasterizer::DebugParam debug;
	debug.debug = true;
	debug.x = 681;
	debug.y = 190;
	debug.tile_id = 42;
	debug.coarse_id = 14;
	debug.fine_id = 14;
	debug.edge_id = 0; 
	
	{
		Vector3 v0(1, 2, 0), v1(7, 4, 0), v2(5, 2, 0);
		Rasterizer::drawTriangleLarrabee(1024, 1024, v0, v1, v2, [](const std::pair<int32_t, int32_t>& pos, const Vector3&) {
			printf("%d, %d\n", pos.first, pos.second);
		}, &debug);
	}
	return;


	{
		Vector3 v0(824.796997f, 272.355164f, 0), v1(651.741699f, 173.502167f, 0), v2(787.095825f, 323.581726f, 0);
		Rasterizer::drawTriangleLarrabee(1024, 1024, v0, v1, v2, [](const std::pair<int32_t, int32_t>& pos, const Vector3&) {
			//printf("%d, %d\n", pos.first, pos.second);
		}, &debug);
	}

	{
		Vector3 v0(651.741699f, 173.502167f, 0), v1(824.796997f, 272.355164f, 0), v2(1024.f, 0.f, 0);
		Rasterizer::drawTriangleLarrabee(1024, 1024, v0, v1, v2, [](const std::pair<int32_t, int32_t>& pos, const Vector3&) {
			//printf("%d, %d\n", pos.first, pos.second);
		}, &debug);
	}
}

//-------------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
	//init glfw
	if (GLFW_TRUE != glfwInit()) {
		return -1;
	}

	// glfw window creation
	GLFWwindow* window = glfwCreateWindow(g_viewWidth, g_viewHeight, "Larabee Rasterization Test", NULL, NULL);
	if (window == NULL) {
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	glfwWindowHint(GLFW_SAMPLES, 4);

	// Set callback functions
	glfwSetFramebufferSizeCallback(window, _onFramebufferSize);
	glfwSetWindowRefreshCallback(window, _onWindowRefresh);
	glfwSetCursorPosCallback(window, _onCursorPos);
	glfwSetMouseButtonCallback(window, _onMouseButton);
	glfwSetKeyCallback(window, _onKeyboard);
	glfwSetScrollCallback(window, _onMouseScroll);

	// Enable vsync
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	// set cursor
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

	//build view matrix
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	_onFramebufferSize(window, width, height);

	//build some demo triangles
	_buildDemoTriangles();

	// render loop
	while (!glfwWindowShouldClose(window)) {
		// Only redraw if we need to
		if (g_bNeedRedraw)
			_onWindowRefresh(window);

		// Wait for new events
		glfwWaitEvents();
	}

	// Close OpenGL window and terminate GLFW
	glfwTerminate();
	return 0;
}
