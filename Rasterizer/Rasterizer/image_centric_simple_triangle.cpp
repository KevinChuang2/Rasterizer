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

void OutputFrame(const std::vector<glm::vec3>& frameBuffer, const char* filename, unsigned int width, unsigned int height);
void image_centric_simple_triangle()
{
	std::vector<glm::vec3> frameBuffer(g_scWidth * g_scHeight, glm::vec3(0, 0, 0)); // clear color black = vec3(0, 0, 0)
	glm::vec3 triangleVertices[] = {
		glm::vec3(-0.5, 0.0, 1.0),
		glm::vec3(0.0, 0.5, 1.0),
		glm::vec3(0.5, 0.0, 1.0)
	};
	for (auto& it : triangleVertices)
	{
		it = TO_RASTER(it);
	}

	glm::mat3 M =
	{
		{ triangleVertices[0].x, triangleVertices[1].x, triangleVertices[2].x },
		{ triangleVertices[0].y, triangleVertices[1].y, triangleVertices[2].y },
		{ triangleVertices[0].z, triangleVertices[1].z, triangleVertices[2].z },
	};
	M = glm::inverse(M);
	glm::vec3 E0 = M * glm::vec3(1, 0, 0);
	glm::vec3 E1 = M * glm::vec3(0, 1, 0);
	glm::vec3 E2 = M * glm::vec3(0, 0, 1);

	// Start rasterizing by looping over pixels to output a per-pixel color
	for (auto y = 0; y < g_scHeight; y++)
	{
		for (auto x = 0; x < g_scWidth; x++)
		{
			// Sample location at the center of each pixel
			glm::vec3 sample = { x + 0.5f, g_scHeight - y + 0.5f, 1.0f };

			// Evaluate edge functions at every fragment
			float alpha = glm::dot(E0, sample);
			float beta = glm::dot(E1, sample);
			float gamma = glm::dot(E2, sample);

			// If sample is "inside" of all three half-spaces bounded by the three edges of our triangle, it's 'on' the triangle
			if ((alpha >= 0.0f) && (beta >= 0.0f) && (gamma >= 0.0f))
			{
				frameBuffer[x + y * g_scWidth] = glm::vec3(1, 0, 0);
			}
		}
	}

	OutputFrame(frameBuffer, "test.ppm", g_scWidth, g_scHeight);
}
