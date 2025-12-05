#ifndef RECTANGLE_HPP
#define RECTANGLE_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Rectangle {
public:
	
	Rectangle(float topL, float topR, float bottomL, float bottomR) 
		: topL(topL), topR(topR), bottomL(bottomL), bottomR(bottomR) {}
	float topL, topR, bottomL, bottomR;

private:
};

#endif // !WORLD_MAP_H