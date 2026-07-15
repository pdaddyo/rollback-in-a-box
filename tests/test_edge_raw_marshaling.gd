extends SceneTree

# Edge cases for the Box3DRaw marshaling layer: unknown functions/structs,
# wrong argument counts, wrong Variant types, wrong byte sizes, and buffer
# size limits. These calls intentionally push engine errors ("ERROR: Box3DRaw:
# ..."); the failure signal for the suite remains the literal "FAIL:" prefix.

var api := Box3D.new()
var raw: Box3DRaw
var cases := 0

func fail(msg: String) -> void:
	push_error("FAIL: " + msg)
	quit(1)

func all_zero(bytes: PackedByteArray) -> bool:
	return bytes.count(0) == bytes.size()

func check_unknown_function_name() -> void:
	cases += 1
	var result = raw.call_box3d(&"b3DoesNotExist", [])
	if result != null:
		return fail("call_box3d on unknown function should return null")
	result = raw.call_box3d(StringName(), [])
	if result != null:
		return fail("call_box3d on empty StringName should return null")

func check_wrong_argument_count() -> void:
	cases += 1
	if raw.call_box3d(&"b3World_IsValid", []) != null:
		return fail("b3World_IsValid with 0 args should return null")
	if raw.call_box3d(&"b3World_IsValid", [0, 0]) != null:
		return fail("b3World_IsValid with 2 args should return null")
	if raw.call_box3d(&"b3Length", []) != null:
		return fail("b3Length with 0 args should return null")

func check_wrong_variant_type_for_vec3() -> void:
	cases += 1
	# float where b3Vec3 expected: StructCodec falls back to a zeroed b3Vec3.
	var result = api.b3Length(1.5)
	if typeof(result) != TYPE_FLOAT:
		return fail("b3Length(1.5) should still return a float, got %s" % type_string(typeof(result)))
	if result != 0.0:
		return fail("b3Length on zero-decoded vector should be exactly 0.0, got %s" % result)

func check_wrong_byte_size_packed_struct_arg() -> void:
	cases += 1
	# 3 bytes where sizeof(b3AABB) expected: unpack_value yields a zeroed AABB.
	var result = api.b3AABB_Area(PackedByteArray([1, 2, 3]))
	if typeof(result) != TYPE_FLOAT:
		return fail("b3AABB_Area with wrong byte size should return a float")
	if result != 0.0:
		return fail("b3AABB_Area on zeroed AABB should be 0.0, got %s" % result)

func check_unknown_struct_type() -> void:
	cases += 1
	var buffer = raw.make_buffer(&"b3NotAStruct", {})
	if buffer != null:
		return fail("make_buffer with unknown struct type should return null")
	var encoded: PackedByteArray = raw.encode_struct(&"b3NotAStruct", {})
	if encoded.size() != 0:
		return fail("encode_struct with unknown struct type should be empty")
	var decoded = raw.decode_struct(&"b3NotAStruct", PackedByteArray())
	if decoded != null:
		return fail("decode_struct with unknown struct type should return null")

func check_decode_struct_wrong_byte_size() -> void:
	cases += 1
	# Wrong byte size decodes to a ZEROED struct dictionary, not null.
	for bytes in [PackedByteArray(), PackedByteArray([7])]:
		var decoded = raw.decode_struct(&"b3Sphere", bytes)
		if typeof(decoded) != TYPE_DICTIONARY:
			return fail("decode_struct(b3Sphere, %d bytes) should return a Dictionary" % bytes.size())
		if not decoded.has("center") or not decoded.has("radius"):
			return fail("zeroed b3Sphere dictionary is missing fields: %s" % decoded)
		if decoded["center"] != Vector3.ZERO:
			return fail("zeroed b3Sphere center should be Vector3.ZERO, got %s" % decoded["center"])
		if decoded["radius"] != 0.0:
			return fail("zeroed b3Sphere radius should be 0.0, got %s" % decoded["radius"])

func check_encode_struct_wrong_variant_value() -> void:
	cases += 1
	# int where Dictionary/PackedByteArray expected: codec ERR_FAILs to a zero
	# struct, so encode_struct still returns sizeof(struct) zero bytes and
	# make_buffer SUCCEEDS with zeroed content.
	var sphere_size: int = raw.get_struct_info(&"b3Sphere")["size"]
	var buffer = raw.make_buffer(&"b3Sphere", 42)
	if buffer == null:
		return fail("make_buffer(b3Sphere, 42) should succeed with zeroed content")
	if buffer.get_size() != sphere_size:
		return fail("zeroed b3Sphere buffer size %d != struct size %d" % [buffer.get_size(), sphere_size])
	if not all_zero(buffer.get_data()):
		return fail("make_buffer(b3Sphere, 42) buffer should be all zeros")

	# b3ChildShape has no dictionary codec: a Dictionary falls to the base
	# codec (PackedByteArray required) and encodes as a zero struct.
	var child_size: int = raw.get_struct_info(&"b3ChildShape")["size"]
	var encoded: PackedByteArray = raw.encode_struct(&"b3ChildShape", {})
	if encoded.size() != child_size:
		return fail("encode_struct(b3ChildShape, {}) size %d != struct size %d" % [encoded.size(), child_size])
	if not all_zero(encoded):
		return fail("encode_struct(b3ChildShape, {}) should be all zeros")

func check_buffer_sizes() -> void:
	cases += 1
	if raw.make_byte_buffer(-1) != null:
		return fail("make_byte_buffer(-1) should return null")

	var buf = raw.make_byte_buffer(0)
	if buf == null:
		return fail("make_byte_buffer(0) should return a valid buffer")
	if buf.get_size() != 0:
		return fail("zero-size buffer get_size() should be 0")
	if not buf.get_data().is_empty():
		return fail("zero-size buffer get_data() should be empty")
	if buf.get_address() != 0:
		return fail("zero-size buffer get_address() should be 0")

	var buf2 = raw.make_byte_buffer(16)
	if buf2 == null:
		return fail("make_byte_buffer(16) failed")
	buf2.resize(-4)
	if buf2.get_size() != 16:
		return fail("resize(-4) should leave size at 16, got %d" % buf2.get_size())

func check_array_field_length_mismatch() -> void:
	cases += 1
	# Fixed-size array field given 1 NONZERO element instead of 8: the length
	# check ERR_FAILs before any element is copied, so encode_struct returns a
	# full-size byte array that is still all zeros. If the vertex made it into
	# the output, the 8-element validation would be missing.
	var hull_size: int = raw.get_struct_info(&"b3BoxHull")["size"]
	var bytes: PackedByteArray = raw.encode_struct(&"b3BoxHull", {"boxVertices": [Vector3(1, 2, 3)]})
	if bytes.size() != hull_size:
		return fail("encode_struct(b3BoxHull, short array) size %d != struct size %d" % [bytes.size(), hull_size])
	if not all_zero(bytes):
		return fail("encode_struct(b3BoxHull, short array) should reject the array and stay zeroed")

func check_null_callback_world_queries() -> void:
	cases += 1
	# Null where a callback function pointer is expected: pointer_from_variant
	# maps null to nullptr, and on an empty world the tree traversal never
	# invokes it, so the queries must return tree stats without crashing.
	var world: int = raw.call_box3d(&"b3CreateWorld", [raw.make_buffer(&"b3WorldDef", api.b3DefaultWorldDef())])
	if not api.b3World_IsValid(world):
		return fail("could not create an empty world for null-callback queries")
	var filter: Dictionary = api.b3DefaultQueryFilter()
	var ray_stats = raw.call_box3d(&"b3World_CastRay", [world, Vector3(0, 10, 0), Vector3(0, -20, 0), filter, null, null])
	if typeof(ray_stats) != TYPE_DICTIONARY or not ray_stats.has("nodeVisits"):
		return fail("b3World_CastRay with null callback should return tree stats, got %s" % [ray_stats])
	var aabb := {"lowerBound": Vector3(-1, -1, -1), "upperBound": Vector3(1, 1, 1)}
	var overlap_stats = raw.call_box3d(&"b3World_OverlapAABB", [world, aabb, filter, null, null])
	if typeof(overlap_stats) != TYPE_DICTIONARY or not overlap_stats.has("nodeVisits"):
		return fail("b3World_OverlapAABB with null callback should return tree stats, got %s" % [overlap_stats])
	api.b3DestroyWorld(world)

func check_catalog_completeness() -> void:
	cases += 1
	var file := FileAccess.open("res://api/box3d-api.json", FileAccess.READ)
	if file == null:
		return fail("could not open api/box3d-api.json")
	var model = JSON.parse_string(file.get_as_text())
	if typeof(model) != TYPE_DICTIONARY:
		return fail("api/box3d-api.json did not parse to a Dictionary")

	var expected_functions := int(model["function_count"])
	if raw.get_api_function_count() != expected_functions:
		return fail("get_api_function_count()=%d != model function_count=%d; re-run tools/generate_bindings.py" % [raw.get_api_function_count(), expected_functions])
	if raw.get_functions().size() != expected_functions:
		return fail("get_functions() lists %d names, expected %d; re-run tools/generate_bindings.py" % [raw.get_functions().size(), expected_functions])

	var model_structs: Array = model["structs"]
	var expected_structs := int(model["struct_count"])
	if model_structs.size() != expected_structs:
		return fail("model struct list %d != struct_count %d" % [model_structs.size(), expected_structs])
	var struct_names := raw.get_structs()
	if struct_names.size() != expected_structs:
		return fail("get_structs() lists %d names, expected %d; re-run tools/generate_bindings.py" % [struct_names.size(), expected_structs])
	for entry in model_structs:
		var struct_name: String = entry["name"]
		if not struct_names.has(struct_name):
			return fail("struct %s missing from get_structs(); re-run tools/generate_bindings.py" % struct_name)
		var info: Dictionary = raw.get_struct_info(struct_name)
		if info.is_empty() or int(info["size"]) <= 0:
			return fail("get_struct_info(%s) missing or has size <= 0; re-run tools/generate_bindings.py" % struct_name)

	var expected_constants: int = model["constants"].size()
	if raw.get_constants().size() != expected_constants:
		return fail("get_constants() has %d entries, expected %d; re-run tools/generate_bindings.py" % [raw.get_constants().size(), expected_constants])

	if not raw.get_function_info(&"bogus").is_empty():
		return fail("get_function_info(bogus) should be an empty Dictionary")
	if not raw.get_struct_info(&"bogus").is_empty():
		return fail("get_struct_info(bogus) should be an empty Dictionary")

func _initialize() -> void:
	raw = api.raw
	check_unknown_function_name()
	check_wrong_argument_count()
	check_wrong_variant_type_for_vec3()
	check_wrong_byte_size_packed_struct_arg()
	check_unknown_struct_type()
	check_decode_struct_wrong_byte_size()
	check_encode_struct_wrong_variant_value()
	check_buffer_sizes()
	check_array_field_length_mismatch()
	check_null_callback_world_queries()
	check_catalog_completeness()
	print("edge raw marshaling: OK cases=%d" % cases)
	quit(0)
