#ifndef CAR_STATE_H
#define CAR_STATE_H

#include <glm/glm.hpp>

namespace NetWork {
	struct CarState {
		glm::vec3 pos;
		float rot;
		float currentAngle;
		float velocity;
		float forwardRot;
	};
}
#endif // !CAR_STATE_H