#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <iostream>
#include <algorithm>
#include <vector>
#define TO_RASTER(v) glm::vec4((g_scWidth * (v.x + v.w) / 2), (g_scHeight * (v.w+v.y) / 2), v.z, v.w)
//#define TO_RASTER(v) glm::vec4((g_scWidth * (v.x + v.w) / 2), (g_scHeight * (v.w - v.y) / 2), v.z, v.w)
#define ARR_SIZE(x) (sizeof(x) / sizeof(x[0]))
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
float min3(float a, float b, float c);
float max3(float a, float b, float c);
void flipImage(std::vector<glm::vec3>& frameBuffer, int g_scWidth, int g_scHeight);
bool EvaluateEdgeFunction(const glm::vec3& E, const glm::vec3& sample)
{
	// Interpolate edge function at given sample
	float result = (E.x * sample.x) + (E.y * sample.y) + E.z;

	// Apply tie-breaking rules on shared vertices in order to avoid double-shading fragments
	if (result > 0.0f) return true;
	else if (result < 0.0f) return false;

	if (E.x > 0.f) return true;
	else if (E.x < 0.0f) return false;

	if ((E.x == 0.0f) && (E.y < 0.0f)) return false;
	else return true;
}
glm::vec4 VS(glm::vec3 vertex, glm::mat4 model, glm::mat4 view, glm::mat4 proj)
{
	return proj * view * model* glm::vec4(vertex, 1.0);
}

void InitializeSceneObjects(std::vector<glm::mat4>& objects)
{
	// Construct a scene of few cubes randomly positioned

	const glm::mat4 identity(1.f);

	glm::mat4 M0 = glm::translate(identity, glm::vec3(0, 0, 2.f));
	M0 = glm::rotate(M0, glm::radians(45.f), glm::vec3(0, 1, 0));
	objects.push_back(M0);

	glm::mat4 M1 = glm::translate(identity, glm::vec3(-3.75f, 0, 0));
	M1 = glm::rotate(M1, glm::radians(30.f), glm::vec3(1, 0, 0));
	objects.push_back(M1);

	glm::mat4 M2 = glm::translate(identity, glm::vec3(3.75f, 0, 0));
	M2 = glm::rotate(M2, glm::radians(60.f), glm::vec3(0, 1, 0));
	objects.push_back(M2);

	glm::mat4 M3 = glm::translate(identity, glm::vec3(0, 0, -2.f));
	M3 = glm::rotate(M3, glm::radians(90.f), glm::vec3(0, 0, 1));
	objects.push_back(M3);
}

void cubes()
{
	// Setup vertices & indices to draw an indexed cube
	glm::vec3 vertices[] =
	{
		{ 1.0f, -1.0f, -1.0f },
		{ 1.0f, -1.0f, 1.0f },
		{ -1.0f, -1.0f, 1.0f },
		{ -1.0f, -1.0f, -1.0f },
		{ 1.0f, 1.0f, -1.0f },
		{ 1.0f, 1.0f, 1.0f },
		{ -1.0f, 1.0f, 1.0f },
		{ -1.0f, 1.0f, -1.0f },
	};

	uint32_t indices[] =
	{
		// 6 faces of cube * 2 triangles per-face * 3 vertices per-triangle = 36 indices
		1,3,0, 7,5,4, 4,1,0, 5,2,1, 2,7,3, 0,7,4, 1,2,3, 7,6,5, 4,5,1, 5,6,2, 2,6,7, 0,3,7
	};

	// Use per-face colors
	glm::vec3 colors[] =
	{
		glm::vec3(0, 0, 1),
		glm::vec3(0, 1, 0),
		glm::vec3(0, 1, 1),
		glm::vec3(1, 1, 1),
		glm::vec3(1, 0, 1),
		glm::vec3(1, 1, 0)
	};

	// Allocate and clear the frame buffer before starting to render to it
	std::vector<glm::vec3> frameBuffer(g_scWidth * g_scHeight, glm::vec3(0, 0, 0)); // clear color black = vec3(0, 0, 0)

																					// Allocate and clear the depth buffer (not z-buffer but 1/w-buffer in this part!) to 0.0
	std::vector<float> depthBuffer(g_scWidth * g_scHeight, -INFINITY);

	// Let's draw multiple objects
	std::vector<glm::mat4> objects;
	InitializeSceneObjects(objects);

	// Build view & projection matrices (right-handed sysem)
	float nearPlane = 0.1f;
	float farPlane = 100.f;
	glm::vec3 eye(0, 2,5);
	glm::vec3 lookat(0, 0, 0);
	glm::vec3 up(0, 1, 0);

	glm::mat4 view = glm::lookAt(eye, lookat, up);
	glm::mat4 proj = glm::perspective(glm::radians(60.f), static_cast<float>(g_scWidth) / static_cast<float>(g_scHeight), nearPlane, farPlane);

	// Loop over objects in the scene
	for (size_t n = 0; n < objects.size(); n++)
	{
		// Loop over triangles in a given object and rasterize them one by one
		for (uint32_t idx = 0; idx < ARR_SIZE(indices) / 3; idx++)
		{
			// We're gonna fetch object-space vertices from the vertex buffer indexed by the values in index buffer
			// and pass them directly to each VS invocation
			const glm::vec3& v0i = vertices[indices[idx * 3]];
			const glm::vec3& v1i = vertices[indices[idx * 3 + 1]];
			const glm::vec3& v2i = vertices[indices[idx * 3 + 2]];

			// Invoke function for each vertex of the triangle to transform them from object-space to clip-space (-w, w)
			glm::vec4 v0Clip = VS(v0i, objects[n], view, proj);
			glm::vec4 v1Clip = VS(v1i, objects[n], view, proj);
			glm::vec4 v2Clip = VS(v2i, objects[n], view, proj);

			// Apply viewport transformation
			// Notice that we haven't applied homogeneous division and are still utilizing homogeneous coordinates
			glm::vec4 v0Homogen = TO_RASTER(v0Clip);
			glm::vec4 v1Homogen = TO_RASTER(v1Clip);
			glm::vec4 v2Homogen = TO_RASTER(v2Clip);

			v0Clip = v0Clip / v0Clip.w;
			v1Clip = v1Clip / v1Clip.w;
			v2Clip = v2Clip / v2Clip.w;
			// Apply viewport transformation
			// Notice that we haven't applied homogeneous division and are still utilizing homogeneous coordinates
			glm::vec4 v0 = TO_RASTER(v0Clip);
			glm::vec4 v1 = TO_RASTER(v1Clip);
			glm::vec4 v2 = TO_RASTER(v2Clip);

			int minX = min3(v0.x, v1.x, v2.x);
			int minY = min3(v0.y, v1.y, v2.y);

			int maxX = max3(v0.x, v1.x, v2.x);
			int maxY = max3(v0.y, v1.y, v2.y);
			minX = std::max(minX, 0);
			minY = std::max(minY, 0);	
			maxX = std::min(maxX, (int)g_scWidth - 1);
			maxY = std::min(maxY, (int)g_scHeight - 1);


			

			// Base vertex matrix
			glm::mat3 M =
			{
				// Notice that glm is itself column-major)
				{ v0Homogen.x, v1Homogen.x, v2Homogen.x },
				{ v0Homogen.y, v1Homogen.y, v2Homogen.y },
				{ v0Homogen.w, v1Homogen.w, v2Homogen.w },
			};

			// Singular vertex matrix (det(M) == 0.0) means that the triangle has zero area,
			// which in turn means that it's a degenerate triangle which should not be rendered anyways,
			// whereas (det(M) > 0) implies a back-facing triangle so we're going to skip such primitives
			float det = glm::determinant(M);
			if (det < 0.0f)
				continue;

			// Compute the inverse of vertex matrix to use it for setting up edge & constant functions
			M = inverse(M);

			// Set up edge functions based on the vertex matrix
			glm::vec3 E0 = M[0];
			glm::vec3 E1 = M[1];
			glm::vec3 E2 = M[2];

			// Calculate constant function to interpolate 1/w
			glm::vec3 C = M * glm::vec3(1, 1, 1);
			// Start rasterizing by looping over pixels to output a per-pixel color
			for (auto y = minY; y < maxY; y++)
			{
				for (auto x = minX; x < maxX; x++)
				{
					// Sample location at the center of each pixel
					glm::vec3 sample = { x + 0.5f, y + 0.5f, 1.0f };

					// Evaluate edge functions at current fragment
					float area = edgeFunction(v2, v1, v0); // area of the multiplied by 2 
					float w0 = edgeFunction(v2, v1, sample); // signed area of the v1v2p multiplied by 2 
					float w1 = edgeFunction(v0, v2, sample); // signed area of the v2v0p multiplied by 2 
					float w2 = edgeFunction(v1, v0, sample); // signed area of the triangle v0v1p multiplied by 2 
					float z = 1 / (w0 * 1 / v0[2] + w1 * 1 / v1[2] + w2 * 1 / v2[2]);
					float oneOverW = (C.x * sample.x) + (C.y * sample.y) + C.z;
					// If sample is "inside" of all three half-spaces bounded by the three edges of the triangle, it's 'on' the triangle
					if (w0>0 && w1>0 && w2>0)
					{
						//std::cout << z << std::endl;
						//std::cout << colors[indices[3 * idx] % 6].x << std::endl;
						if (depthBuffer[x + y*g_scWidth]  < oneOverW)
						{

							depthBuffer[x + y*g_scWidth] = oneOverW;;
							frameBuffer[x + y * g_scWidth] = colors[indices[3 * idx] % 6];
						}
						
					}
				}
			}
		}
	}
	flipImage(frameBuffer, g_scWidth, g_scHeight);
	// Rendering of one frame is finished, output a .PPM file of the contents of our frame buffer to see what we actually just rendered
	OutputFrame(frameBuffer, "test.ppm", g_scWidth, g_scHeight);
}