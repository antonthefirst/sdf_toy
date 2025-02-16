#include "orbit_camera.h"

void OrbitCamera::update(const Input& in) {
	if (in.rotate_down) {
		azim +=  in.mouse_del.x*3.0f;
		elev += -in.mouse_del.y*3.0f;
	}
	if (in.wheel_del != 0.0f)
		dist *= exp(- in.wheel_del * 0.125f);
}
pose3 OrbitCamera::getWorldFromCamera() const {
	pose3 rot = pose3::R(quat::fromAxisAngle(vec3(1,0,0), elev) * quat::fromAxisAngle(vec3(0,1,0), azim)); 
	return ~rot * pose3::TR(vec3(0, 0, dist), quat::I());
}