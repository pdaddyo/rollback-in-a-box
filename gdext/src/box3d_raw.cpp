#include "box3d_raw.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/variant/quaternion.hpp>
#include <godot_cpp/variant/transform3d.hpp>
#include <godot_cpp/variant/vector3.hpp>

#include "box3d/box3d.h"
#include "box3d/collision.h"

#include "rollback_shim.h"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <type_traits>

using namespace godot;

namespace {

template <typename T>
T unpack_value(const PackedByteArray &bytes) {
	ERR_FAIL_COND_V_MSG(bytes.size() != (int64_t)sizeof(T), T{}, "Box3DRaw: packed struct has the wrong byte size.");
	T value{};
	std::memcpy(&value, bytes.ptr(), sizeof(T));
	return value;
}

template <typename T>
PackedByteArray pack_value(const T &value) {
	PackedByteArray bytes;
	bytes.resize((int64_t)sizeof(T));
	std::memcpy(bytes.ptrw(), &value, sizeof(T));
	return bytes;
}

template <typename T>
struct StructCodec {
	static T decode(const Variant &value) {
		ERR_FAIL_COND_V_MSG(value.get_type() != Variant::PACKED_BYTE_ARRAY, T{},
				"Box3DRaw: this struct requires its exact PackedByteArray representation.");
		return unpack_value<T>((PackedByteArray)value);
	}

	static Variant encode(const T &value) {
		return pack_value(value);
	}
};

template <typename T>
T from_variant(const Variant &value) {
	using U = std::remove_cv_t<std::remove_reference_t<T>>;
	if constexpr (std::is_same_v<U, b3WorldId>) {
		return b3LoadWorldId((uint32_t)(int64_t)value);
	} else if constexpr (std::is_same_v<U, b3BodyId>) {
		return b3LoadBodyId((uint64_t)(int64_t)value);
	} else if constexpr (std::is_same_v<U, b3ShapeId>) {
		return b3LoadShapeId((uint64_t)(int64_t)value);
	} else if constexpr (std::is_same_v<U, b3JointId>) {
		return b3LoadJointId((uint64_t)(int64_t)value);
	} else if constexpr (std::is_same_v<U, b3Vec3>) {
		if (value.get_type() == Variant::VECTOR3) {
			const Vector3 v = value;
			return { (float)v.x, (float)v.y, (float)v.z };
		}
		return StructCodec<U>::decode(value);
	} else if constexpr (std::is_same_v<U, b3Quat>) {
		if (value.get_type() == Variant::QUATERNION) {
			const Quaternion q = value;
			return { { (float)q.x, (float)q.y, (float)q.z }, (float)q.w };
		}
		return StructCodec<U>::decode(value);
	} else if constexpr (std::is_same_v<U, b3Transform>) {
		if (value.get_type() == Variant::TRANSFORM3D) {
			const Transform3D transform = value;
			const Quaternion q = transform.basis.get_quaternion();
			return { { (float)transform.origin.x, (float)transform.origin.y, (float)transform.origin.z },
				{ { (float)q.x, (float)q.y, (float)q.z }, (float)q.w } };
		}
		return StructCodec<U>::decode(value);
	} else if constexpr (std::is_enum_v<U>) {
		return static_cast<U>((int64_t)value);
	} else if constexpr (std::is_same_v<U, bool>) {
		return (bool)value;
	} else if constexpr (std::is_integral_v<U>) {
		return static_cast<U>((int64_t)value);
	} else if constexpr (std::is_floating_point_v<U>) {
		return static_cast<U>((double)value);
	} else {
		return StructCodec<U>::decode(value);
	}
}

template <typename T>
Variant to_variant(const T &value) {
	using U = std::remove_cv_t<std::remove_reference_t<T>>;
	if constexpr (std::is_same_v<U, b3WorldId>) {
		return (int64_t)b3StoreWorldId(value);
	} else if constexpr (std::is_same_v<U, b3BodyId>) {
		return (int64_t)b3StoreBodyId(value);
	} else if constexpr (std::is_same_v<U, b3ShapeId>) {
		return (int64_t)b3StoreShapeId(value);
	} else if constexpr (std::is_same_v<U, b3JointId>) {
		return (int64_t)b3StoreJointId(value);
	} else if constexpr (std::is_same_v<U, b3Vec3>) {
		return Vector3(value.x, value.y, value.z);
	} else if constexpr (std::is_same_v<U, b3Quat>) {
		return Quaternion(value.v.x, value.v.y, value.v.z, value.s);
	} else if constexpr (std::is_same_v<U, b3Transform>) {
		return Transform3D(Quaternion(value.q.v.x, value.q.v.y, value.q.v.z, value.q.s),
				Vector3(value.p.x, value.p.y, value.p.z));
	} else if constexpr (std::is_enum_v<U>) {
		return (int64_t)value;
	} else if constexpr (std::is_same_v<U, bool>) {
		return value;
	} else if constexpr (std::is_integral_v<U>) {
		return (int64_t)value;
	} else if constexpr (std::is_floating_point_v<U>) {
		return (double)value;
	} else {
		return StructCodec<U>::encode(value);
	}
}

template <typename T>
T pointer_from_variant(const Variant &value) {
	static_assert(std::is_pointer_v<T>, "Pointer conversion requires a pointer type.");
	uintptr_t address = 0;
	if (value.get_type() == Variant::OBJECT) {
		Object *object = value;
		Box3DBuffer *buffer = Object::cast_to<Box3DBuffer>(object);
		ERR_FAIL_NULL_V_MSG(buffer, nullptr, "Box3DRaw: pointer arguments must be Box3DBuffer, an address, or null.");
		address = (uintptr_t)buffer->get_ptr();
	} else if (value.get_type() == Variant::INT) {
		address = (uintptr_t)(int64_t)value;
	} else if (value.get_type() != Variant::NIL) {
		ERR_FAIL_V_MSG(nullptr, "Box3DRaw: pointer arguments must be Box3DBuffer, an address, or null.");
	}
	return reinterpret_cast<T>(address);
}

#include "generated/box3d_structs.inc"
#include "generated/box3d_catalog.inc"

} // namespace

void Box3DBuffer::resize(int64_t size) {
	ERR_FAIL_COND(size < 0);
	const size_t words = (size_t)((size + (int64_t)sizeof(std::max_align_t) - 1) / (int64_t)sizeof(std::max_align_t));
	storage.resize(words);
	byte_size = size;
}

void Box3DBuffer::set_data(const PackedByteArray &data) {
	resize(data.size());
	if (byte_size > 0) {
		std::memcpy(storage.data(), data.ptr(), (size_t)byte_size);
	}
}

PackedByteArray Box3DBuffer::get_data() const {
	PackedByteArray data;
	data.resize(byte_size);
	if (byte_size > 0) {
		std::memcpy(data.ptrw(), storage.data(), (size_t)byte_size);
	}
	return data;
}

int64_t Box3DBuffer::get_address() const {
	return (int64_t)(uintptr_t)get_ptr();
}

void *Box3DBuffer::get_ptr() {
	return byte_size > 0 ? storage.data() : nullptr;
}

const void *Box3DBuffer::get_ptr() const {
	return byte_size > 0 ? storage.data() : nullptr;
}

void Box3DBuffer::_bind_methods() {
	ClassDB::bind_method(D_METHOD("resize", "size"), &Box3DBuffer::resize);
	ClassDB::bind_method(D_METHOD("get_size"), &Box3DBuffer::get_size);
	ClassDB::bind_method(D_METHOD("set_data", "data"), &Box3DBuffer::set_data);
	ClassDB::bind_method(D_METHOD("get_data"), &Box3DBuffer::get_data);
	ClassDB::bind_method(D_METHOD("get_address"), &Box3DBuffer::get_address);
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_BYTE_ARRAY, "data"), "set_data", "get_data");
}

Variant Box3DRaw::call_box3d(const StringName &function, const Array &arguments) {
#include "generated/box3d_functions.inc"
	ERR_FAIL_V_MSG(Variant(), "Box3DRaw: unknown Box3D function: " + String(function));
}

PackedStringArray Box3DRaw::get_functions() const {
	return generated_function_names();
}

Dictionary Box3DRaw::get_function_info(const StringName &function) const {
	return generated_function_info(function);
}

Dictionary Box3DRaw::get_constants() const {
	return generated_constants();
}

PackedStringArray Box3DRaw::get_structs() const {
	return generated_struct_names();
}

Dictionary Box3DRaw::get_struct_info(const StringName &type) const {
	return generated_struct_info(type);
}

PackedByteArray Box3DRaw::encode_struct(const StringName &type, const Variant &value) const {
	return generated_encode_struct(type, value);
}

Variant Box3DRaw::decode_struct(const StringName &type, const PackedByteArray &bytes) const {
	return generated_decode_struct(type, bytes);
}

Ref<Box3DBuffer> Box3DRaw::make_buffer(const StringName &type, const Variant &value) const {
	const PackedByteArray bytes = encode_struct(type, value);
	ERR_FAIL_COND_V(bytes.is_empty(), Ref<Box3DBuffer>());
	Ref<Box3DBuffer> buffer;
	buffer.instantiate();
	buffer->set_data(bytes);
	return buffer;
}

Ref<Box3DBuffer> Box3DRaw::make_byte_buffer(int64_t size) const {
	ERR_FAIL_COND_V(size < 0, Ref<Box3DBuffer>());
	Ref<Box3DBuffer> buffer;
	buffer.instantiate();
	buffer->resize(size);
	return buffer;
}

int Box3DRaw::get_api_function_count() const {
	return generated_function_count();
}

void Box3DRaw::install_exit_on_assert() const {
	b3r_install_exit_on_assert();
}

void Box3DRaw::_bind_methods() {
	ClassDB::bind_method(D_METHOD("call_box3d", "function", "arguments"), &Box3DRaw::call_box3d);
	ClassDB::bind_method(D_METHOD("get_functions"), &Box3DRaw::get_functions);
	ClassDB::bind_method(D_METHOD("get_function_info", "function"), &Box3DRaw::get_function_info);
	ClassDB::bind_method(D_METHOD("get_constants"), &Box3DRaw::get_constants);
	ClassDB::bind_method(D_METHOD("get_structs"), &Box3DRaw::get_structs);
	ClassDB::bind_method(D_METHOD("get_struct_info", "type"), &Box3DRaw::get_struct_info);
	ClassDB::bind_method(D_METHOD("encode_struct", "type", "value"), &Box3DRaw::encode_struct);
	ClassDB::bind_method(D_METHOD("decode_struct", "type", "bytes"), &Box3DRaw::decode_struct);
	ClassDB::bind_method(D_METHOD("make_buffer", "type", "value"), &Box3DRaw::make_buffer);
	ClassDB::bind_method(D_METHOD("make_byte_buffer", "size"), &Box3DRaw::make_byte_buffer);
	ClassDB::bind_method(D_METHOD("get_api_function_count"), &Box3DRaw::get_api_function_count);
	ClassDB::bind_method(D_METHOD("install_exit_on_assert"), &Box3DRaw::install_exit_on_assert);
}
