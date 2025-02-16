#include "core/platform.h" // for assert
#include <string.h> // for memcpy

// Usage:
// To define a new struct to serialize, use STRUCT(MyStruct) and END at the end.
// When you want to start serializing a member of a struct, do an ADD() with the current app version.
// When you want to stop serializing a member of a struct (such as if it was removed), change the ADD() to a REM(), keep the add version, and add the current app version as the rem version. The newer versions will skip this member in old files.
// If you want to start using the same member name again, you can do so, but you need to keep the REM around for the prior data. You can rename the member inside the REM, and use the same name for the new ADD (the names are not saved, but the notion of "added at ver X, and removed at ver Y, at this location in the file", needs to remain in the record).
// Renaming a member is fine, since the names are never stored.
// You cannot ever remove a type fully, once it's been serialized anywhere (because code needs to know it's size, which is encoded in it's collection of ADD/REM).
// However you can rename these deprecated types to whatever you want, since again the names are never saved.
// The order of members is crucial, since it will be written/read in that order, so it can't be altered. However the order does not have to match the actual struct used in the app code.

// The number of bits needed to encode this size (ie 0001 0000 -> 5).
// It's the equivalent of finding the 0-based index of the highest set bit, plus one.
inline int numBitsNeeded(size_t s) {
    int i = 0;
    for (; s; s >>= 1, i++);
    return i;
}

struct SerializeCtx {
    bool  is_reading = true;
    int   version    = 0;
    char* buf_cur    = 0;
    char* buf_end    = 0;
    unsigned char integrity = 0;

    static SerializeCtx makeWriter(char* start, char* end, int current_version) {
        SerializeCtx ctx;
        ctx.is_reading = false;
        ctx.buf_cur = start;
        ctx.buf_end = end;
        ctx.version = current_version;
        ctx.serializeBytes(&ctx.version, sizeof(int));
        return ctx;
    }

    static SerializeCtx makeReader(char* start, char* end) {
        SerializeCtx ctx;
        ctx.is_reading = true;
        ctx.buf_cur = start;
        ctx.buf_end = end;
        // Read in the version.
        ctx.serializeBytes(&ctx.version, sizeof(int));
        return ctx;
    }

    void skip(size_t bytes_to_skip) {
        ASSERT(size_t(buf_end - buf_cur) >= bytes_to_skip);
        buf_cur += bytes_to_skip;
    }

    void serializeBytes(void* data, size_t size) {
        // Make sure we either have enough space, or enough bytes to read.
        ASSERT(size_t(buf_end - buf_cur) >= size);
        if (is_reading)
            memcpy(data, buf_cur, size);
        else
            memcpy(buf_cur, data, size);
        buf_cur += size;
    }

    // Serialize size with variable length encoding, from 1 to 8 bytes.
    void serializeSize(size_t* size) {
        if (is_reading) {
            size_t data = 0;
            int chunk = 0; // The current 7-bit chunk we are writing to.
            while (true) {
                ASSERT(buf_cur != buf_end); // Expected more data based on encoding.
                char byte = *buf_cur++;
                // Place the 7 lower bits of the byte into the data.
                data |= size_t(byte & 0x7f) << (chunk * 7);
                chunk += 1;
                // If the upper bit is set, keep reading, otherwise stop.
                if (byte & 0x80) {
                    continue;
                } else {
                    break;
                }
            }
            *size = data;
        } else {
            size_t data = *size;
            int num_bits = max(1, numBitsNeeded(data));
            // 1 bit per byte is used to encode continuation, so the maximum bits that can fit into size_t is sizeof(size_t)*7.
            // I think we can fit more into the last byte, by using the continuation bit, if we assume that max bytes is sizeof(size_t)...but it doesn't seem worth the trouble.
            // This code would also work transparently with signed integers, since they effectively use the top bit, and that's another reason to keep it. Except negative numbers
            // would always trigger the largest encoding size, and also would not work for 64 bit values, so perhaps it's more complicated.
            ASSERT(num_bits <= (sizeof(size_t) * 7));

            while(num_bits > 0) {
                // Take the lowest 7-bit chunk from our data and put it into a byte.
                char byte = data & 0x7f;
                if (num_bits > 7) {
                    // If there are more than 7 bits left to write, mark top bit in this byte to indicate continuation.
                    byte |= 0x80;
                    // Look at the next 7 bits of data.
                    data = data >> 7;
                    num_bits -= 7;
                } else {
                    // Otherwise this is the last byte to write.
                    num_bits = 0;
                }
                ASSERT(buf_cur != buf_end); // Ran out of space to encode.
                *buf_cur++ = byte;
            }
        }
    }

    void serializeIntegrityCheck() {
        if (is_reading) {
            // When reading, read in the stored integrity bytes, and compare them with the expected value.
            unsigned char read_integrity = 0;
            serializeBytes(&read_integrity, sizeof(read_integrity));
            ASSERT(read_integrity == integrity);
        } else {
            // When writing, write out the current integrity bytes.
            serializeBytes(&integrity, sizeof(integrity));
        }
        integrity += 1;
    }
};

// Define a plain-old-data type which will be serialized as opaque bytes.
// These are bedrock and can never be changed based on version, so pick carefully.
// Beware of padding for simple, but non-basic types (such as vec3 etc).
#define POD(type, size)                                   \
inline void Serialize(SerializeCtx& ctx, type& data) {    \
    static_assert(sizeof(type) == size);                  \
    ctx.serializeBytes(&data, sizeof(type));              \
}

// Start a new struct.
#define STRUCT(type)                                       \
inline void Serialize(SerializeCtx& _ctx, type& _struct) { \

// End the struct.
#define END                         \
    _ctx.serializeIntegrityCheck(); \
} 

// Add a member at a given version.
#define ADD(member_type, member_name, add_version) \
if (_ctx.version >= add_version)                   \
    Serialize(_ctx, _struct.member_name);

// Remove a member past a given version.
#define REM(member_type, member_name, add_version, rem_version)  \
if (_ctx.version >= add_version && _ctx.version < rem_version) { \
    member_type dummy;                                           \
    Serialize(_ctx, dummy);                                      \
}


/* Implementor's notes:

ADD ignores the member type, so it's purely for symmetry during the description process. However, this can be a bit unsafe for templated types, because
you can have ADD(vector<float>, vars) in the descriptor, and vector<int> in the actual struct, and it will compile file (because the Serialize function is generated for vector<int>, inferred from the member type.

REM ignores the member name, since it has no place to use it anyway (the struct no longer has that member, so it wouldn't compile). So the name is only there for symmetry, in the description.


*/