#pragma once

// A (by convention) null terminated, statically sized string buffer.
template<size_t N>
struct BString {
	static const size_t buffer_capacity = N;
	char buffer[N] = { 0 };
	// Removing this for now. 
	// When editing these strings, you typically do them from outside, and bookkeeping the size is not guaranteed (such as by imgui).
	// So it's a bit dangerous, and I'd rather just dynamically compute it via strlen for now.
	// int  string_byte_size = 0; // Number of bytes in this string, NOT including the null terminator.
	size_t stringByteSize() const {
		for (size_t i = 0; i < buffer_capacity; ++i) {
			if (buffer[i] == 0)
				return i;
		}
		// We should be null terminated at least at the end, but if we got here we clearly aren't.
		// Pretend like we are though, just in case (rather than returning the last byte).
		ASSERT(buffer[buffer_capacity-1] == 0); 
		return buffer_capacity-1;
	}
	operator char*() {
		return buffer;
	}
};