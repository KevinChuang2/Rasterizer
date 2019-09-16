#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <iostream>
#include <vector>
#define TO_RASTER(v) glm::vec3((g_scWidth * (v.x + 1.0f) / 2), (g_scHeight * (v.y + 1.f) / 2), 1.0f)
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
void object_centric_simple_triangle()
{
	std::vector<glm::vec3> frameBuffer(g_scWidth * g_scHeight, glm::vec3(0, 0, 0)); // clear color black = vec3(0, 0, 0)
	
	Triangle t0;
	Triangle t1;
	t0.v0 = TO_RASTER(glm::vec3(0.0, 0.5, 1.0));
	t0.v1 = TO_RASTER(glm::vec3(-0.5, 0.5, 1.0));
	t0.v2 = TO_RASTER(glm::vec3(-0.5, -0.5, 1.0));
	t0.c0 = glm::vec3(1.0, 0.0, 0.0);
	t0.c1 = glm::vec3(0.0, 1.0, 0.0);
	t0.c2 = glm::vec3(0.0, 0.0, 1.0);
	t1.v0 = TO_RASTER(glm::vec3(0.0, 0.5, 1.0));
	t1.v1 = TO_RASTER(glm::vec3(0.0, -0.5, 1.0));
	t1.v2 = TO_RASTER(glm::vec3(0.5, -0.5, 1.0));
	t1.c0 = glm::vec3(1.0, 0.0, 0.0);
	t1.c1 = glm::vec3(0.0, 1.0, 0.0);
	t1.c2 = glm::vec3(0.0, 0.0, 1.0);

	std::vector<Triangle> triangles;
	triangles.push_back(t0);
	triangles.push_back(t1);

	for (auto& triangle : triangles)
	{
		// Start rasterizing by looping over pixels to output a per-pixel color
		for (auto y = 0; y < g_scHeight; y++)
		{
			for (auto x = 0; x < g_scWidth; x++)
			{

				// Sample location at	 the center of each pixel
				glm::vec3 sample = { x + 0.5f, g_scHeight - y + 0.5f, 1.0f };

				float area = edgeFunction(triangle.v2, triangle.v1, triangle.v0); // area of the triangle multiplied by 2 
				float w0 = edgeFunction(triangle.v2, triangle.v1, sample); // signed area of the triangle v1v2p multiplied by 2 
				float w1 = edgeFunction(triangle.v0, triangle.v2, sample); // signed area of the triangle v2v0p multiplied by 2 
				float w2 = edgeFunction(triangle.v1, triangle.v0, sample); // signed area of the triangle v0v1p multiplied by 2 

													// if point p is inside triangles defined by vertices v0, v1, v2
				if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
					// barycentric coordinates are the areas of the sub-triangles divided by the area of the main triangle
					w0 /= area;
					w1 /= area;
					w2 /= area;
					float r = w0 * triangle.c0[0] + w1 * triangle.c1[0] + w2*triangle.c2[0];
					float g = w0 * triangle.c0[1] + w1 * triangle.c1[1] + w2*triangle.c2[1];
					float b = w0 * triangle.c0[2] + w1 * triangle.c1[2] + w2*triangle.c2[2];
					//std::cout << area << std::endl;
					frameBuffer[x + y * g_scWidth] = glm::vec3(r,g,b);
				}

			}
		}
	}
	

	

	OutputFrame(frameBuffer, "test.ppm", g_scWidth, g_scHeight);
}