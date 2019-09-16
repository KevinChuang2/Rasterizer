
#include <algorithm>
float min3(float a, float b, float c)
{
	return std::min(a, std::min(b, c));
}
float max3(float a, float b, float c)
{
	return std::max(a, std::max(b, c));
}