#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/string_name.hpp>

#include <cstddef>
#include <vector>

namespace godot {

// Aligned, owned storage for Box3D APIs that use pointers or arrays. The raw
// bridge never borrows a PackedByteArray's potentially unaligned storage.
class Box3DBuffer : public RefCounted {
	GDCLASS(Box3DBuffer, RefCounted)

	std::vector<std::max_align_t> storage;
	int64_t byte_size = 0;

protected:
	static void _bind_methods();

public:
	void resize(int64_t size);
	int64_t get_size() const { return byte_size; }
	void set_data(const PackedByteArray &data);
	PackedByteArray get_data() const;
	int64_t get_address() const;

	void *get_ptr();
	const void *get_ptr() const;
};

// Mechanical bridge for every function declared by Box3D's public headers.
// Exact C names are retained so upstream coverage can be checked mechanically.
class Box3DRaw : public RefCounted {
	GDCLASS(Box3DRaw, RefCounted)

protected:
	static void _bind_methods();

public:
	Variant call_box3d(const StringName &function, const Array &arguments);
	PackedStringArray get_functions() const;
	Dictionary get_function_info(const StringName &function) const;
	Dictionary get_constants() const;
	PackedStringArray get_structs() const;
	Dictionary get_struct_info(const StringName &type) const;
	PackedByteArray encode_struct(const StringName &type, const Variant &value) const;
	Variant decode_struct(const StringName &type, const PackedByteArray &bytes) const;
	Ref<Box3DBuffer> make_buffer(const StringName &type, const Variant &value) const;
	Ref<Box3DBuffer> make_byte_buffer(int64_t size) const;
	int get_api_function_count() const;
};

} // namespace godot
