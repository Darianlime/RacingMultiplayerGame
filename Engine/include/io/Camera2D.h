#ifndef CAMERA2D_H
#define CAMERA2D_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Engine {
	enum class Camera2dDirection {
		NONE = 0,
		LEFT,
		RIGHT,
		UP,
		DOWN
	};

	class Camera2D {
	private:

	public:
		glm::vec3 cameraPos;

		glm::vec3 cameraFront;
		glm::vec3 cameraUp;
		glm::vec3 cameraRight;

		glm::vec3 worldUp;

		float roll;
		float speed;
		float sensitivity;
		float zoom;

		Camera2D(glm::vec3 postion);

		void followTarget(glm::vec3 targetPos);
		void updateCameraVectors();
		void updateCameraDirection(float dz);
		void updateCameraPos(Camera2dDirection dir, double dt);
		void updateCameraZoom(double dy);

		float getZoom();
		glm::vec3 getTargetPos();

		glm::mat4 getViewMatrix();

	};
}
#endif // !CAMERA2D_H