#pragma once

#include "wrap/input_wrap.h"
#include "core/platform.h" // for ARRCOUNT
#include <initializer_list>

extern Input g_hwin;

// These are the abstract hardware inputs provided by the machine (not app specific aliases).
// Left/Right modifiers are or'ed into one. 
// Consider adding specific Left/Right options to poll the raw ones, though I don't know if that would be compatible.
enum class inp {
	None, // Must remain 0. Modifiers assume it to be so.
	Control,
	Shift,
	Alt,
	Escape,
	A,
	B,
	C,
	D,
	E,
	F,
	G,
	H,
	I,
	J,
	K,
	L,
	M,
	N,
	O,
	P,
	Q,
	R,
	S,
	T,
	U,
	V,
	W,
	X,
	Y,
	Z,
	Left,
	Right,
	Up,
	Down,
	LMB, // Left mouse button
	RMB, // Right mouse button
	MMB, // Middle mouse button
	INP_COUNT,
};

// Maybe use instead of initializer lists. Without a constructor, these are aggregate initialized and will put 0's in any slots without an init.
// So {inp::Alt} comes out to {inp::Alt, 0, 0} which is exactly what we want.
// I guess need to look at the disassembly, to see if it's better than initializer list.
// Mem wise, it's almost certain that init list is at least int64+int32 for ptr and count. yeah it's 2 pointers actually (start/end), so it's at least as large as 4 mods, plus any "allocs" it will do? (hopefully none)
// So the practical difference is:
// Initializer list: If there is nothing, pushes stack by 16 bytes, puts null ptrs inside the two values, calls it a day.
// Struct: If there is nothing, pushes stack by 16 bytes (for 4 mods), puts zeroes inside each one (4 times instead of 2), calls it a day.
// So it's likely the struct is slower, unless it optimizes the zero initialization to use something faster than setting 4 values to 0.
// On the other end, when iterating, iterating the list is just iterating the pointer. Iterating the struct is iterating until 4 or until a mod is 0. 
// Either way it's fairly irrelevant, since it only happens while key is pressed, down, or released. (and down is gonna be very rare, with modifiers)
struct Modifiers {
	inp mods[3];
};


struct HwInput {
	inp input;
	inp mods_on_press[3]; // up to 3 modifiers recorded on press
	bool press_claimed = false;
	bool release_claimed = false;
	const char* claimed_by = 0;
	bool pressed = false;
	bool released = false;
	bool down_prev = false;

	// This is all that is filled in by the hardware.
	bool down = false;
};

struct Inputs {
	bool isPressed(inp i, const char* id = 0) {
		return isPressed({}, i, id);
	}
	bool isPressed(std::initializer_list<inp> mods, inp i, const char* id = 0) {
		HwInput& in = inputs[(int)i];
		if (in.press_claimed) {
			// Can potentially return true here if the same id asks again, if that seems useful.
			return false;
		} else {
			if (in.pressed) {
				// Check that all requested modifiers are satisfied.
				bool mod_sat = true;
				for (inp m : mods) {
					if (isUp(m)) {
						mod_sat = false;
						break;
					}
				}
				if (mod_sat) {
					// Record the modifiers that were down during the press, to test during release.
					ASSERT(mods.size() <= ARRSIZE(HwInput::mods_on_press));
					int m_i = 0;
					for (inp m : mods) {
						if (m_i >= ARRSIZE(HwInput::mods_on_press))
							break;
						in.mods_on_press[m_i++] = m;
					}
					in.claimed_by = id;
					in.press_claimed = true;
					return true;
				} else {
					return false;
				}
			} else {
				return false;
			}
		}
	}
	bool isReleased(inp i, const char* id = 0) {
		return isReleased({}, i, id);
	}
	bool isReleased(std::initializer_list<inp> mods, inp i, const char* id = 0) {
		HwInput& in = inputs[(int)i];
		if (in.release_claimed) {
			return false;
		} else {
			if (in.released) {
				if (modsPresentOnPress(mods, i) && claimedOnPressBy(i, id)) {
					in.release_claimed = true;
					return true;
				} else {
					return false;
				}
			} else {
				return false;
			}
		}
	}
	bool isDown(inp i, const char* id = 0) {
		return isDown({}, i, id);
	}
	bool isDown(std::initializer_list<inp> mods, inp i, const char* id = 0) {
		HwInput& in = inputs[(int)i];
		if (in.down) {
			if (modsPresentOnPress(mods, i) && claimedOnPressBy(i, id)) {
				return true;
			} else {
				return false;
			}
		} else {
			return false;
		}
	}
	bool isUp(inp i) {
		HwInput& in = inputs[(int)i];
		return !in.down;
	}
	void newFrame() { 
		for (int i = 0; i < (int)inp::INP_COUNT; ++i) {
			HwInput& in = inputs[(int)i];
			in.down = false;
		}

		/* FEED THE RAW INPUTS HERE */
		// For now, just polls the global input wrapper over glfw. 
		inputs[(int)inp::Shift].down = g_hwin.key.down[KEY_LEFT_SHIFT] || g_hwin.key.down[KEY_RIGHT_SHIFT];
		inputs[(int)inp::Alt].down = g_hwin.key.down[KEY_LEFT_ALT] || g_hwin.key.down[KEY_RIGHT_ALT];
		inputs[(int)inp::Control].down = g_hwin.key.down[KEY_LEFT_CONTROL] || g_hwin.key.down[KEY_RIGHT_CONTROL];
		inputs[(int)inp::Escape].down = g_hwin.key.down[KEY_ESCAPE];
		inputs[(int)inp::A].down = g_hwin.key.down['A'];
		inputs[(int)inp::B].down = g_hwin.key.down['B'];
		inputs[(int)inp::C].down = g_hwin.key.down['C'];
		inputs[(int)inp::D].down = g_hwin.key.down['D'];
		inputs[(int)inp::E].down = g_hwin.key.down['E'];
		inputs[(int)inp::F].down = g_hwin.key.down['F'];
		inputs[(int)inp::G].down = g_hwin.key.down['G'];
		inputs[(int)inp::H].down = g_hwin.key.down['H'];
		inputs[(int)inp::I].down = g_hwin.key.down['I'];
		inputs[(int)inp::J].down = g_hwin.key.down['J'];
		inputs[(int)inp::K].down = g_hwin.key.down['K'];
		inputs[(int)inp::L].down = g_hwin.key.down['L'];
		inputs[(int)inp::M].down = g_hwin.key.down['M'];
		inputs[(int)inp::N].down = g_hwin.key.down['N'];
		inputs[(int)inp::O].down = g_hwin.key.down['O'];
		inputs[(int)inp::P].down = g_hwin.key.down['P'];
		inputs[(int)inp::Q].down = g_hwin.key.down['Q'];
		inputs[(int)inp::R].down = g_hwin.key.down['R'];
		inputs[(int)inp::S].down = g_hwin.key.down['S'];
		inputs[(int)inp::T].down = g_hwin.key.down['T'];
		inputs[(int)inp::U].down = g_hwin.key.down['U'];
		inputs[(int)inp::V].down = g_hwin.key.down['V'];
		inputs[(int)inp::W].down = g_hwin.key.down['W'];
		inputs[(int)inp::X].down = g_hwin.key.down['X'];
		inputs[(int)inp::Y].down = g_hwin.key.down['Y'];
		inputs[(int)inp::Z].down = g_hwin.key.down['Z'];
		inputs[(int)inp::Left].down = g_hwin.key.down[KEY_LEFT];
		inputs[(int)inp::Right].down = g_hwin.key.down[KEY_RIGHT];
		inputs[(int)inp::Up].down = g_hwin.key.down[KEY_UP];
		inputs[(int)inp::Down].down = g_hwin.key.down[KEY_DOWN];
		inputs[(int)inp::LMB].down = g_hwin.mouse.down[MOUSE_BUTTON_1];
		inputs[(int)inp::RMB].down = g_hwin.mouse.down[MOUSE_BUTTON_2];
		inputs[(int)inp::MMB].down = g_hwin.mouse.down[MOUSE_BUTTON_3];

		for (int i = 0; i < (int)inp::INP_COUNT; ++i) {
			HwInput& in = inputs[(int)i];
			in.press_claimed = false;
			in.release_claimed = false;
			in.pressed = in.down && !in.down_prev;
			in.released = !in.down && in.down_prev;
			if (!in.down && !in.released) {
				in.claimed_by = 0;
				memset(in.mods_on_press, 0, sizeof(in.mods_on_press));
			}
			in.down_prev = in.down;
		}
	}

private:
	HwInput inputs[(int)inp::INP_COUNT];
	bool modsPresentOnPress(std::initializer_list<inp> mods, inp i) {
		ASSERT((int)i >= 0 && (int)i < (int)inp::INP_COUNT);
		HwInput& in = inputs[(int)i];
		bool mod_sat = true;
		for (inp m : mods) {
			bool present = false;
			for (int m_i = 0; m_i < ARRSIZE(HwInput::mods_on_press); ++m_i) {
				if (in.mods_on_press[m_i] == inp::None)
					break;
				if (in.mods_on_press[m_i] == m) {
					present = true;
					break;
				}
			}
			if (!present) {
				mod_sat = false;
				break;
			}
		}
		return mod_sat;
	}
	bool claimedOnPressBy(inp i, const char* id) {
		ASSERT((int)i >= 0 && (int)i < (int)inp::INP_COUNT);
		HwInput& in = inputs[(int)i];
		// If either id or alias id is null, then there's nothing to check. Otherwise, compare the ids.
		// If you're not specifying, allow to read back whoever.
		if (!id) return true;
		// If an id is specified, by the claim was by nobody, then it's not a match.
		if (!in.claimed_by) return false;
		// Otherwise, compare the id's (they're both non-null).
		return strcmp(id, in.claimed_by) == 0;
	}
};

extern Inputs g_In;

/*
inline void testControls() {

	//if (g_In.isDown(inp::SHIFT) && g_In.isPressed(inp::K)) log("SHIFT & K pressed\n");
	if (g_In.isPressed({inp::Alt, inp::Shift}, inp::K, "me"))
		log("ALT+SHIFT+K pressed by 'me'\n");
	if (g_In.isReleased({inp::Shift, inp::Alt}, inp::K, "me")) // shift/alt were held while K was pressed by "me", then it was released.
		log("ALT+SHIFT+K released by 'me'\n");
	if (g_In.isPressed({inp::Shift}, inp::K, "me"))
		log("SHIFT+K pressed by 'me'\n");
	if (g_In.isReleased({inp::Shift}, inp::K, "me"))
		log("SHIFT+K released by 'me'\n");
	if (g_In.isPressed({inp::Shift}, inp::LMB)) log("SHIFT+LMB pressed\n");

	//if (g_In.isPressed(inp::K)) log("K pressed\n");
	if (g_In.isPressed(inp::K, "you")) log("K pressed by 'you'\n");
	if (g_In.isPressed(inp::K, "me"))
		log("K pressed by 'me'\n");
	if (g_In.isReleased(inp::K, "me")) 
		log("K released by 'me'\n");
	//if (g_In.isDown(inp::SHIFT)) log("SHIFT down\n");
	if (g_In.isDown(inp::K))
		log("K down\n");
}
*/