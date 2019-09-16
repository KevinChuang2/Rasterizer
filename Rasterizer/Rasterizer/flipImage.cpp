#include <glm/glm.hpp>
#include <vector>
#include <iostream>
void flipImage(std::vector<glm::vec3>& frameBuffer, int g_scWidth, int g_scHeight)
{
	for (auto y = 0; y < g_scHeight/2; y++)
	{
		for (auto x = 0; x < g_scWidth; x++)
		{
			glm::vec3 temp = frameBuffer[x + y * g_scWidth];
			frameBuffer[x + y * g_scWidth] = frameBuffer[x + (g_scHeight - y-1)*g_scWidth];
			frameBuffer[x + (g_scHeight - y-1)*g_scWidth] = temp;
		}
	}
}