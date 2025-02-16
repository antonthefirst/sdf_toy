#pragma once

inline void _writeThenReadSize(size_t size, int byte_count) {
	char bytes[32] = { 0 };
	SerializeCtx w = SerializeCtx::makeWriter(bytes, bytes + sizeof(bytes), 0);
	char* w_start = w.buf_cur;
	w.serializeSize(&size);
	int w_byte_count = int(w.buf_cur - w_start);
	ASSERT(w_byte_count == byte_count);
	SerializeCtx r = SerializeCtx::makeReader(bytes, bytes + sizeof(bytes));
	size_t read_size = 0;
	r.serializeSize(&read_size);
	ASSERT(read_size == size);
}
inline void _testSerializeSize() {
	// Sanity checks
	_writeThenReadSize(0, 1);
	_writeThenReadSize(1, 1);
	// 0 - 7 bits -> 1 byte
	_writeThenReadSize(128 - 1, 1); 
	// 8 - 14 bits -> 2 bytes
	_writeThenReadSize(128, 2); 
	_writeThenReadSize(16384 - 1, 2);
	// 15 - 21 bits -> 3 bytes
	_writeThenReadSize(16384, 3);
	_writeThenReadSize(2097152 - 1, 3);
	// 22 - 28 bits -> 4 bytes
	_writeThenReadSize(2097152, 4);
	_writeThenReadSize(268435456 - 1, 4);
	// 29 - 35 bits -> 5 bytes
	_writeThenReadSize(268435456, 5);
	_writeThenReadSize(34359738368 - 1, 5);
	// ... and so on
	// Test the largest number that can be written into size_t.
	// 1 bit per byte is spent on continuation, so the largest number of bits is sizeof(size_t)*7, and then subtract one to keep the top bit empty.
	size_t largest_number = (size_t(1) << (sizeof(size_t) * 7)) - 1;
	_writeThenReadSize(largest_number, sizeof(size_t));
}

struct _BasicTestStructV0 {
	float  p = 3.15f;
	int    i = -1;
	size_t b = 32000;
	bool operator==(const _BasicTestStructV0&) const = default;
};

STRUCT(_BasicTestStructV0)
ADD(float, p, 0)
ADD(int, i, 0)
ADD(size_t, b, 0)
END

struct _BasicTestStructV1 {
	float  k = -8.0f; // Added in V2
	float  p = 3.15f;
	// int    i = -1; // Removed in V2
	size_t b = 32000;
	bool operator==(const _BasicTestStructV1&) const = default;
};

STRUCT(_BasicTestStructV1)
ADD(float, k, 1)
ADD(float, p, 0)
REM(int, i, 0, 1)
ADD(size_t, b, 0)
END

template<typename T>
inline void _writeThenReadBunch(Bunch<T> bunch) {
	char bytes[1024] = { 0 };
	SerializeCtx w = SerializeCtx::makeWriter(bytes, bytes + sizeof(bytes), 0);
	Serialize(w, bunch);
	SerializeCtx r = SerializeCtx::makeReader(bytes, bytes + sizeof(bytes));
	Bunch<T> read_bunch;
	Serialize(r, read_bunch);
	ASSERT(read_bunch == bunch);
}
inline void _testSerializeBunch() {
	Bunch<_BasicTestStructV0> bunch;
	bunch.push(_BasicTestStructV0());
	bunch.push({6.125f, 32, 1000000});
	bunch.push({-4.125f, -999, 500000});
	_writeThenReadBunch(bunch);
}
inline void _testSerializeBasicVersion() {
	char bytes[1024] = { 0 };
	// Write a V0 struct, then read back into a V1 struct.
	SerializeCtx w = SerializeCtx::makeWriter(bytes, bytes + sizeof(bytes), 0);
	_BasicTestStructV0 v0 = _BasicTestStructV0{6.125f, 32, 1000000};
	Serialize(w, v0);
	SerializeCtx r = SerializeCtx::makeReader(bytes, bytes + sizeof(bytes));
	_BasicTestStructV1 v1;
	Serialize(r, v1);
	_BasicTestStructV1 v1_def;

	ASSERT(v1.k == v1_def.k); // 'k' is added in v1, so should be default
	ASSERT(v1.p == v0.p);     // 'p' is present in v0, so it should have been read
	// 'i' is removed in v1, so we shouldn't have crashed
	ASSERT(v1.b == v0.b);     // 'b' is present in v0, so it should have been read (and not be wrong cause 'i' wasn't skipped)
}