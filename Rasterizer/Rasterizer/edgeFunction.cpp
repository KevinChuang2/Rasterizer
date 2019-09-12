#include <glm/glm.hpp>
bool edgeFunction(const glm::vec2 &a, const glm::vec2 &b, const glm::vec2 &c)
{
	return ((c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x) >= 0);
}