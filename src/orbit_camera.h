#pragma once

#include "math/maths.h"
#include "math/pose3.h"

struct OrbitCamera {
	struct Input {
		// Mouse movement [-1,+1] on both axes, square aspect
		vec2 mouse_del = vec2(0.0f);

		// Sensitivity
		vec2 rotation_speed = vec2(1.0f);

		// Wheel movement [-1,+1] 
		float wheel_del = 0.0f;

		// Where the mouse is hitting the scene, if any.
		bool world_hit_point_valid = false;
		vec3 world_hit_point = vec3(0.0f);

		// State of buttons for rotation, panning.
		bool rotate_down = false;
		bool pan_down = false;
	};

	void update(const Input& in);

	pose3 getWorldFromCamera() const;

private:
	pose3 world_from_camera = pose3::I();

	float azim = 0.0f;
	float elev = 0.0f;
	float dist = 4.0f;
};