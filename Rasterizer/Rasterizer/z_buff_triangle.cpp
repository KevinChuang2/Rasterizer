#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <iostream>
#include <algorithm>
#include <vector>
#define TO_RASTER(v) glm::vec4((g_scWidth * (v.x + 1.0f) / 2), (g_scHeight * (v.y + 1.f) / 2), v.z, v.w)
// Frame buffer dimensions
static const auto g_scWidth = 1280u;
static const auto g_scHeight = 720u;
struct Triangle {
	glm::vec3 v0;
	glm::vec3 v1;
	glm::vec3 v2;
	glm::vec3 c0;
	glm::vec3 c1;
	glm::vec3 c2;
};
void OutputFrame(const std::vector<glm::vec3>& frameBuffer, const char* filename, unsigned int width, unsigned int height);
float edgeFunction(const glm::vec2 &a, const glm::vec2 &b, const glm::vec2 &c);
float edgeFunction(const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &c);
float edgeFunction(const glm::vec4 &a, const glm::vec4 &b, const glm::vec4 &c);
glm::vec4 to_clip(glm::vec3 vertex, glm::mat4 view, glm::mat4 proj)
{
	return proj * view * glm::vec4(vertex, 1.0);
}
// Build view & projection matrices (right-handed sysem)
float nearPlane = 0.1f;
float farPlane = 100.f;
glm::vec3 eye(0, 0, 5);
glm::vec3 lookat(0, 0, 0);
glm::vec3 up(0, 1, 0);

glm::mat4 view = glm::lookAt(eye, lookat, up);
glm::mat4 proj = glm::perspective(glm::radians(60.f), (float)g_scWidth / (float)g_scHeight, nearPlane, farPlane);

float min3(float a, float b, float c);
float max3(float a, float b, float c);

void z_buff_triangle()
{
	std::vector<float> depthBuffer(g_scWidth * g_scHeight, INFINITY); 
	std::vector<glm::vec3> frameBuffer(g_scWidth * g_scHeight, glm::vec3(0, 0, 0)); // clear color black = vec3(0, 0, 0)

	Triangle t0;
	Triangle t1;
	t0.v0 = glm::vec3(0.5, 0.0, 0.0);
	t0.v1 = glm::vec3(-0.5, 0.5, 0.0);
	t0.v2 = glm::vec3(-0.5, -0.5, 0.0);
	t0.c0 = glm::vec3(1.0, 1.0, 1.0);
	t0.c1 = glm::vec3(0.0, 1.0, 1.0);
	t0.c2 = glm::vec3(0.0, 1.0, 1.0);

	t1.v0 = glm::vec3(-0.25, 0.25, 1.0);
	t1.v1 = glm::vec3(-0.25, -0.25, 1.0);
	t1.v2 = glm::vec3(0.25, -0.25, 1.0);
	t1.c0 = glm::vec3(1.0, 0.0, 0.0);
	t1.c1 = glm::vec3(0.0, 1.0, 0.0);
	t1.c2 = glm::vec3(0.0, 0.0, 1.0);

	std::vector<Triangle> triangles;
	triangles.push_back(t0);
	triangles.push_back(t1);

	for (auto& triangle : triangles)
	{
		//convert to clip space
		glm::vec4 v0clip = to_clip(triangle.v0, view, proj);
		glm::vec4 v1clip = to_clip(triangle.v1, view, proj);
		glm::vec4 v2clip = to_clip(triangle.v2, view, proj);
		//perform perspective divide
		v0clip = v0clip / v0clip.w;
		v1clip = v1clip / v1clip.w;
		v2clip = v2clip / v2clip.w;
		glm::vec4 v0 = TO_RASTER(v0clip);
		glm::vec4 v1 = TO_RASTER(v1clip);
		glm::vec4 v2 = TO_RASTER(v2clip);

		int minX = min3(v0.x, v1.x, v2.x);
		int minY = min3(v0.y, v1.y, v2.y);
		int maxX = max3(v0.x, v1.x, v2.x);
		int maxY = max3(v0.y, v1.y, v2.y);
		minX = std::max(minX, 0);
		minY = std::max(minY, 0);
		maxX = std::min(maxX, (int)g_scWidth - 1);
		maxY = std::min(maxY, (int) g_scHeight - 1);

		// Start rasterizing by looping over pixels to output a per-pixel color
		for (auto y = minY; y < maxY; y++)
		{
			for (auto x = minX; x < maxX; x++)
			{

				// Sample location at	 the center of each pixel
				glm::vec4 sample = { x + 0.5f, g_scHeight - y + 0.5f, 1.0f, 1.0f };

				
				float area = edgeFunction(v2, v1, v0); // area of the multiplied by 2 
				float w0 = edgeFunction(v2, v1, sample); // signed area of the v1v2p multiplied by 2 
				float w1 = edgeFunction(v0, v2, sample); // signed area of the v2v0p multiplied by 2 
				float w2 = edgeFunction(v1, v0, sample); // signed area of the triangle v0v1p multiplied by 2 

																		   // if point p is inside triangles defined by vertices v0, v1, v2
				if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
					// barycentric coordinates are the areas of the sub-triangles divided by the area of the main triangle
					w0 /= area;
					w1 /= area;
					w2 /= area;
					float r = w0 * triangle.c0[0] + w1 * triangle.c1[0] + w2*triangle.c2[0];
					float g = w0 * triangle.c0[1] + w1 * triangle.c1[1] + w2*triangle.c2[1];
					float b = w0 * triangle.c0[2] + w1 * triangle.c1[2] + w2*triangle.c2[2];
					float z = 1 / (w0 * 1/v0[2] + w1 * 1/v1[2] + w2 * 1/v2[2]);
					//std::cout << z << std::endl;
					if (z <= 1.0 && depthBuffer[x + y*g_scWidth] > z)
					{
						frameBuffer[x + y * g_scWidth] = glm::vec3(r, g, b);
						depthBuffer[x + y * g_scWidth] = z;
					}
				}

			}
		}
	}




	OutputFrame(frameBuffer, "test.ppm", g_scWidth, g_scHeight);
}