#include <iostream>
#include <vector>
#include <glm/glm.hpp>
void OutputFrame(const std::vector<glm::vec3>& frameBuffer, const char* filename, unsigned int width, unsigned int height)
{
	assert(frameBuffer.size() >= (width*height));

	FILE* pFile = nullptr;
	fopen_s(&pFile, filename, "w");
	fprintf(pFile, "P3\n%d %d\n%d\n ", width, height, 255);
	for (auto i = 0; i < width * height; ++i)
	{
		// Write out color values clamped to [0, 255] 
		uint32_t r = static_cast<uint32_t>(255 * glm::clamp(frameBuffer[i].r, 0.0f, 1.0f));
		uint32_t g = static_cast<uint32_t>(255 * glm::clamp(frameBuffer[i].g, 0.0f, 1.0f));
		uint32_t b = static_cast<uint32_t>(255 * glm::clamp(frameBuffer[i].b, 0.0f, 1.0f));
		fprintf(pFile, "%d %d %d ", r, g, b);
	}
	fclose(pFile);
}