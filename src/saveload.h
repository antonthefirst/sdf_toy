
#include "serialize.h"
#include "core/basic_types.h"
#include "core/container.h"
#include "bounded_string.h"


//#include "version.h"
enum {
	VER_0,
	VER_1,
	VER_MIRRORED,
	VERSION_COUNT,
	VERSION_CURRENT = VERSION_COUNT - 1,
};

// Define the POD types
POD(u8,     1)
POD(s8,     1)
POD(u16,    2)
POD(s16,    2)
POD(u32,    4)
POD(s32,    4)
POD(float,  4)
POD(bool,   1)
POD(char,   1)
POD(vec2,   8)
POD(vec3,  12)
POD(vec4,  16)
POD(ivec2,  8)
POD(ivec3, 12)
POD(quat,  16)
POD(pose3, 32)

// Bunch container adapter.
template<typename T>
void Serialize(SerializeCtx& ctx, Bunch<T>& data) {
	// If we're reading, this will be zero (not that we care), otherwise, it will be the count of what we're writing.
	size_t count = (BunchCount)data.count;
	// Read into, or write out the count.
	ctx.serializeSize(&count);

	for (BunchCount i = 0; i < (BunchCount)count; ++i) {
		if (ctx.is_reading) {
			// Construct a new element, and then read into it.
			// We construct so that:
			// - non-serialized members are initialized properly
			// - members that the file might not have data for (older file) are default initialized
			T& elem = data.push();
			Serialize(ctx, elem);
		} else {
			Serialize(ctx, data[i]);
		}
	}
}

// Bounded string adapter.
// Does not detect if the string has changed max capacity between versions, so the safest thing to do is to only enlarge it.
// If the string in the file is greater than the capacity
template<int N>
void Serialize(SerializeCtx& ctx, BString<N>& data) {
	// If we're reading, this is ignored.
	size_t size = size_t(data.stringByteSize());
	ctx.serializeSize(&size);
	if (ctx.is_reading && size > (BString<N>::buffer_capacity-1)) {
		// If we are reading, and don't have enough space in the new buffer for a null-terminated string, we have to still "eat" what's on file.
		// Don't attempt to recover the string, it's better to be blank than invalid (it may straddle a utf8 codepoint).
		ctx.skip(size);
	} else {
		// If writing, or capacity is acceptable.
		ctx.serializeBytes(data.buffer, min(size, BString<N>::buffer_capacity));
	}
}

#if 0
STRUCT(Rule::StateTest)
ADD(ivec2, pos, VER_0)
ADD(int, test, VER_0)
END

STRUCT(Rule::StateWrite)
ADD(ivec2, pos, VER_0)
ADD(int, write, VER_0)
END

STRUCT(Rule)
ADD(Bunch<StateTest>, tests, VER_0)
ADD(Bunch<StateWrite>, writes, VER_0)
ADD(ivec2, weaver_move, VER_0)
ADD(int, weaver_rot, VER_0)
END

STRUCT(WeaverRules)
ADD(Bunch<Rule>, rules, VER_0)
END

STRUCT(WeaverInstance)
ADD(ivec2, pos, VER_0)
ADD(int, rot, VER_0)
ADD(int, rules, VER_0)
ADD(bool, mirrored, VER_MIRRORED)
END

// Define all other aggregate types
STRUCT(Cell)
ADD(int, state, VER_0)
ADD(int, rot, VER_0)
END

// Weaver state inside board is unsaved
STRUCT(Board)
ADD(Bunch<Cell>, cells, VER_0)
ADD(ivec2, dims, VER_0)
END

STRUCT(Puzzle)
ADD(Board, initial, VER_0)
ADD(Board, target, VER_0)
ADD(Bunch<WeaverRules>, solution_rules, VER_0)
ADD(Bunch<WeaverInstance>, solution_placements, VER_0)
ADD(int, solution_steps, VER_0)
ADD(BString<64>, name, VER_1)
ADD(BString<256>, flavor, VER_1)
END

#include <stdio.h>
inline void writeToFile(const char* filename, void* data, size_t data_bytes) {
	FILE* f = NULL;
	fopen_s(&f, filename, "wb");
	if (!f)
	{
		log("[Serialize] unable to open file '%s'\n", filename);
		return;
	}
	fwrite(data, data_bytes, 1, f);
	fflush(f);
	fclose(f);
}
inline char* readFromFile(const char* filename, size_t* data_bytes) {
	FILE* f = NULL;
	fopen_s(&f, filename, "rb");
	if (!f)
	{
		log("[Serialize] unable to open file '%s'\n", filename);
		return 0;
	}

	fseek(f, 0, SEEK_END);
	*data_bytes = ftell(f);
	fseek(f, 0, SEEK_SET);
	void* data = malloc(*data_bytes);
	if (data) {
		fread(data, *data_bytes, 1, f);
	}
	fclose(f);
	return (char*)data;
}

void save(Puzzle& puz, const char* name) {
	char filename[256];
	sprintf_s(filename, "puzzles/%s.puz", name);
	static char data_buff[1024*1024];
	SerializeCtx ctx = SerializeCtx::makeWriter(data_buff, data_buff + sizeof(data_buff), VERSION_CURRENT);
	Serialize(ctx, puz);
	writeToFile(filename, data_buff, size_t(ctx.buf_cur - data_buff));
}
void load(Puzzle& puz, const char* name) {
	char filename[256];
	sprintf_s(filename, "puzzles/%s.puz", name);
	size_t data_bytes = 0;
	char* data = readFromFile(filename, &data_bytes);
	SerializeCtx ctx = SerializeCtx::makeReader(data, data + data_bytes);
	puz = Puzzle();
	Serialize(ctx, puz);
	free(data);
}
#endif