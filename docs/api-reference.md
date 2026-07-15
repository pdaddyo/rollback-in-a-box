# Box3D API reference

This file is generated from the pinned Box3D public headers. Do not edit it by hand.

Coverage: **695 public functions** (572 exported `B3_API`, 123 public inline) and **108 concrete public structs**.

Use the generated `Box3D` class for exact C function names or call `Box3DRaw.call_box3d()` directly. IDs are Godot integers. Value structs accept Dictionaries or exact packed bytes. Pointer parameters accept `Box3DBuffer`, a native address returned by Box3D, or `null`.

## Functions

| Function | Signature | Kind |
| --- | --- | --- |
| `b3AABB_Area` | `float b3AABB_Area(b3AABB)` | inline |
| `b3AABB_Center` | `b3Vec3 b3AABB_Center(b3AABB)` | inline |
| `b3AABB_Contains` | `bool b3AABB_Contains(b3AABB, b3AABB)` | inline |
| `b3AABB_Extents` | `b3Vec3 b3AABB_Extents(b3AABB)` | inline |
| `b3AABB_Inflate` | `b3AABB b3AABB_Inflate(b3AABB, float)` | inline |
| `b3AABB_Overlaps` | `bool b3AABB_Overlaps(b3AABB, b3AABB)` | inline |
| `b3AABB_Transform` | `b3AABB b3AABB_Transform(b3Transform, b3AABB)` | inline |
| `b3AABB_Union` | `b3AABB b3AABB_Union(b3AABB, b3AABB)` | inline |
| `b3Abs` | `b3Vec3 b3Abs(b3Vec3)` | inline |
| `b3AbsFloat` | `float b3AbsFloat(float)` | inline |
| `b3AbsMatrix3` | `b3Matrix3 b3AbsMatrix3(b3Matrix3)` | inline |
| `b3Add` | `b3Vec3 b3Add(b3Vec3, b3Vec3)` | inline |
| `b3AddMM` | `b3Matrix3 b3AddMM(b3Matrix3, b3Matrix3)` | inline |
| `b3Atan2` | `float b3Atan2(float, float)` | exported |
| `b3Blend2` | `b3Vec3 b3Blend2(float, b3Vec3, float, b3Vec3)` | inline |
| `b3Body_ApplyAngularImpulse` | `void b3Body_ApplyAngularImpulse(b3BodyId, b3Vec3, bool)` | exported |
| `b3Body_ApplyForce` | `void b3Body_ApplyForce(b3BodyId, b3Vec3, b3Pos, bool)` | exported |
| `b3Body_ApplyForceToCenter` | `void b3Body_ApplyForceToCenter(b3BodyId, b3Vec3, bool)` | exported |
| `b3Body_ApplyLinearImpulse` | `void b3Body_ApplyLinearImpulse(b3BodyId, b3Vec3, b3Pos, bool)` | exported |
| `b3Body_ApplyLinearImpulseToCenter` | `void b3Body_ApplyLinearImpulseToCenter(b3BodyId, b3Vec3, bool)` | exported |
| `b3Body_ApplyMassFromShapes` | `void b3Body_ApplyMassFromShapes(b3BodyId)` | exported |
| `b3Body_ApplyTorque` | `void b3Body_ApplyTorque(b3BodyId, b3Vec3, bool)` | exported |
| `b3Body_CastRay` | `b3BodyCastResult b3Body_CastRay(b3BodyId, b3Pos, b3Vec3, b3QueryFilter, float, b3WorldTransform)` | exported |
| `b3Body_CastShape` | `b3BodyCastResult b3Body_CastShape(b3BodyId, b3Pos, const b3ShapeProxy *, b3Vec3, b3QueryFilter, float, bool, b3WorldTransform)` | exported |
| `b3Body_CollideMover` | `int b3Body_CollideMover(b3BodyId, b3BodyPlaneResult *, int, b3Pos, const b3Capsule *, b3QueryFilter, b3WorldTransform)` | exported |
| `b3Body_ComputeAABB` | `b3AABB b3Body_ComputeAABB(b3BodyId)` | exported |
| `b3Body_Disable` | `void b3Body_Disable(b3BodyId)` | exported |
| `b3Body_Enable` | `void b3Body_Enable(b3BodyId)` | exported |
| `b3Body_EnableContactRecycling` | `void b3Body_EnableContactRecycling(b3BodyId, bool)` | exported |
| `b3Body_EnableHitEvents` | `void b3Body_EnableHitEvents(b3BodyId, bool)` | exported |
| `b3Body_EnableSleep` | `void b3Body_EnableSleep(b3BodyId, bool)` | exported |
| `b3Body_GetAngularDamping` | `float b3Body_GetAngularDamping(b3BodyId)` | exported |
| `b3Body_GetAngularVelocity` | `b3Vec3 b3Body_GetAngularVelocity(b3BodyId)` | exported |
| `b3Body_GetClosestPoint` | `float b3Body_GetClosestPoint(b3BodyId, b3Vec3 *, b3Vec3)` | exported |
| `b3Body_GetContactCapacity` | `int b3Body_GetContactCapacity(b3BodyId)` | exported |
| `b3Body_GetContactData` | `int b3Body_GetContactData(b3BodyId, b3ContactData *, int)` | exported |
| `b3Body_GetGravityScale` | `float b3Body_GetGravityScale(b3BodyId)` | exported |
| `b3Body_GetInverseMass` | `float b3Body_GetInverseMass(b3BodyId)` | exported |
| `b3Body_GetJointCount` | `int b3Body_GetJointCount(b3BodyId)` | exported |
| `b3Body_GetJoints` | `int b3Body_GetJoints(b3BodyId, b3JointId *, int)` | exported |
| `b3Body_GetLinearDamping` | `float b3Body_GetLinearDamping(b3BodyId)` | exported |
| `b3Body_GetLinearVelocity` | `b3Vec3 b3Body_GetLinearVelocity(b3BodyId)` | exported |
| `b3Body_GetLocalCenter` | `b3Vec3 b3Body_GetLocalCenter(b3BodyId)` | exported |
| `b3Body_GetLocalPoint` | `b3Vec3 b3Body_GetLocalPoint(b3BodyId, b3Pos)` | exported |
| `b3Body_GetLocalPointVelocity` | `b3Vec3 b3Body_GetLocalPointVelocity(b3BodyId, b3Vec3)` | exported |
| `b3Body_GetLocalRotationalInertia` | `b3Matrix3 b3Body_GetLocalRotationalInertia(b3BodyId)` | exported |
| `b3Body_GetLocalVector` | `b3Vec3 b3Body_GetLocalVector(b3BodyId, b3Vec3)` | exported |
| `b3Body_GetMass` | `float b3Body_GetMass(b3BodyId)` | exported |
| `b3Body_GetMassData` | `b3MassData b3Body_GetMassData(b3BodyId)` | exported |
| `b3Body_GetMotionLocks` | `b3MotionLocks b3Body_GetMotionLocks(b3BodyId)` | exported |
| `b3Body_GetName` | `const char * b3Body_GetName(b3BodyId)` | exported |
| `b3Body_GetPosition` | `b3Pos b3Body_GetPosition(b3BodyId)` | exported |
| `b3Body_GetRotation` | `b3Quat b3Body_GetRotation(b3BodyId)` | exported |
| `b3Body_GetShapeCount` | `int b3Body_GetShapeCount(b3BodyId)` | exported |
| `b3Body_GetShapes` | `int b3Body_GetShapes(b3BodyId, b3ShapeId *, int)` | exported |
| `b3Body_GetSleepThreshold` | `float b3Body_GetSleepThreshold(b3BodyId)` | exported |
| `b3Body_GetTransform` | `b3WorldTransform b3Body_GetTransform(b3BodyId)` | exported |
| `b3Body_GetType` | `b3BodyType b3Body_GetType(b3BodyId)` | exported |
| `b3Body_GetUserData` | `void * b3Body_GetUserData(b3BodyId)` | exported |
| `b3Body_GetWorld` | `b3WorldId b3Body_GetWorld(b3BodyId)` | exported |
| `b3Body_GetWorldCenter` | `b3Pos b3Body_GetWorldCenter(b3BodyId)` | exported |
| `b3Body_GetWorldInverseRotationalInertia` | `b3Matrix3 b3Body_GetWorldInverseRotationalInertia(b3BodyId)` | exported |
| `b3Body_GetWorldPoint` | `b3Pos b3Body_GetWorldPoint(b3BodyId, b3Vec3)` | exported |
| `b3Body_GetWorldPointVelocity` | `b3Vec3 b3Body_GetWorldPointVelocity(b3BodyId, b3Pos)` | exported |
| `b3Body_GetWorldVector` | `b3Vec3 b3Body_GetWorldVector(b3BodyId, b3Vec3)` | exported |
| `b3Body_IsAwake` | `bool b3Body_IsAwake(b3BodyId)` | exported |
| `b3Body_IsBullet` | `bool b3Body_IsBullet(b3BodyId)` | exported |
| `b3Body_IsContactRecyclingEnabled` | `bool b3Body_IsContactRecyclingEnabled(b3BodyId)` | exported |
| `b3Body_IsEnabled` | `bool b3Body_IsEnabled(b3BodyId)` | exported |
| `b3Body_IsSleepEnabled` | `bool b3Body_IsSleepEnabled(b3BodyId)` | exported |
| `b3Body_IsValid` | `bool b3Body_IsValid(b3BodyId)` | exported |
| `b3Body_OverlapShape` | `bool b3Body_OverlapShape(b3BodyId, b3Pos, const b3ShapeProxy *, b3QueryFilter, b3WorldTransform)` | exported |
| `b3Body_SetAngularDamping` | `void b3Body_SetAngularDamping(b3BodyId, float)` | exported |
| `b3Body_SetAngularVelocity` | `void b3Body_SetAngularVelocity(b3BodyId, b3Vec3)` | exported |
| `b3Body_SetAwake` | `void b3Body_SetAwake(b3BodyId, bool)` | exported |
| `b3Body_SetBullet` | `void b3Body_SetBullet(b3BodyId, bool)` | exported |
| `b3Body_SetGravityScale` | `void b3Body_SetGravityScale(b3BodyId, float)` | exported |
| `b3Body_SetLinearDamping` | `void b3Body_SetLinearDamping(b3BodyId, float)` | exported |
| `b3Body_SetLinearVelocity` | `void b3Body_SetLinearVelocity(b3BodyId, b3Vec3)` | exported |
| `b3Body_SetMassData` | `void b3Body_SetMassData(b3BodyId, b3MassData)` | exported |
| `b3Body_SetMotionLocks` | `void b3Body_SetMotionLocks(b3BodyId, b3MotionLocks)` | exported |
| `b3Body_SetName` | `void b3Body_SetName(b3BodyId, const char *)` | exported |
| `b3Body_SetSleepThreshold` | `void b3Body_SetSleepThreshold(b3BodyId, float)` | exported |
| `b3Body_SetTargetTransform` | `void b3Body_SetTargetTransform(b3BodyId, b3WorldTransform, float, bool)` | exported |
| `b3Body_SetTransform` | `void b3Body_SetTransform(b3BodyId, b3Pos, b3Quat)` | exported |
| `b3Body_SetType` | `void b3Body_SetType(b3BodyId, b3BodyType)` | exported |
| `b3Body_SetUserData` | `void b3Body_SetUserData(b3BodyId, void *)` | exported |
| `b3Clamp` | `b3Vec3 b3Clamp(b3Vec3, b3Vec3, b3Vec3)` | inline |
| `b3ClampFloat` | `float b3ClampFloat(float, float, float)` | inline |
| `b3ClampInt` | `int b3ClampInt(int, int, int)` | inline |
| `b3ClipVector` | `b3Vec3 b3ClipVector(b3Vec3, const b3CollisionPlane *, int)` | exported |
| `b3CloneAndTransformHull` | `b3HullData * b3CloneAndTransformHull(const b3HullData *, b3Transform, b3Vec3)` | exported |
| `b3CloneHull` | `b3HullData * b3CloneHull(const b3HullData *)` | exported |
| `b3ClosestPointToAABB` | `b3Vec3 b3ClosestPointToAABB(b3Vec3, b3AABB)` | inline |
| `b3CollideCapsuleAndSphere` | `void b3CollideCapsuleAndSphere(b3LocalManifold *, int, const b3Capsule *, const b3Sphere *, b3Transform)` | exported |
| `b3CollideCapsuleAndTriangle` | `void b3CollideCapsuleAndTriangle(b3LocalManifold *, int, const b3Capsule *, const b3Vec3 *, b3SimplexCache *)` | exported |
| `b3CollideCapsules` | `void b3CollideCapsules(b3LocalManifold *, int, const b3Capsule *, const b3Capsule *, b3Transform)` | exported |
| `b3CollideHullAndCapsule` | `void b3CollideHullAndCapsule(b3LocalManifold *, int, const b3HullData *, const b3Capsule *, b3Transform, b3SimplexCache *)` | exported |
| `b3CollideHullAndSphere` | `void b3CollideHullAndSphere(b3LocalManifold *, int, const b3HullData *, const b3Sphere *, b3Transform, b3SimplexCache *)` | exported |
| `b3CollideHullAndTriangle` | `void b3CollideHullAndTriangle(b3LocalManifold *, int, const b3HullData *, b3Vec3, b3Vec3, b3Vec3, int, b3SATCache *, bool)` | exported |
| `b3CollideHulls` | `void b3CollideHulls(b3LocalManifold *, int, const b3HullData *, const b3HullData *, b3Transform, b3SATCache *)` | exported |
| `b3CollideSphereAndTriangle` | `void b3CollideSphereAndTriangle(b3LocalManifold *, int, const b3Sphere *, const b3Vec3 *)` | exported |
| `b3CollideSpheres` | `void b3CollideSpheres(b3LocalManifold *, int, const b3Sphere *, const b3Sphere *, b3Transform)` | exported |
| `b3ComputeCapsuleAABB` | `b3AABB b3ComputeCapsuleAABB(const b3Capsule *, b3Transform)` | exported |
| `b3ComputeCapsuleMass` | `b3MassData b3ComputeCapsuleMass(const b3Capsule *, float)` | exported |
| `b3ComputeCompoundAABB` | `b3AABB b3ComputeCompoundAABB(const b3CompoundData *, b3Transform)` | exported |
| `b3ComputeCosSin` | `b3CosSin b3ComputeCosSin(float)` | exported |
| `b3ComputeHeightFieldAABB` | `b3AABB b3ComputeHeightFieldAABB(const b3HeightFieldData *, b3Transform)` | exported |
| `b3ComputeHullAABB` | `b3AABB b3ComputeHullAABB(const b3HullData *, b3Transform)` | exported |
| `b3ComputeHullMass` | `b3MassData b3ComputeHullMass(const b3HullData *, float)` | exported |
| `b3ComputeMeshAABB` | `b3AABB b3ComputeMeshAABB(const b3MeshData *, b3Transform, b3Vec3)` | exported |
| `b3ComputeQuatBetweenUnitVectors` | `b3Quat b3ComputeQuatBetweenUnitVectors(b3Vec3, b3Vec3)` | exported |
| `b3ComputeSphereAABB` | `b3AABB b3ComputeSphereAABB(const b3Sphere *, b3Transform)` | exported |
| `b3ComputeSphereMass` | `b3MassData b3ComputeSphereMass(const b3Sphere *, float)` | exported |
| `b3Conjugate` | `b3Quat b3Conjugate(b3Quat)` | inline |
| `b3Contact_GetData` | `b3ContactData b3Contact_GetData(b3ContactId)` | exported |
| `b3Contact_IsValid` | `bool b3Contact_IsValid(b3ContactId)` | exported |
| `b3ConvertBytesToCompound` | `b3CompoundData * b3ConvertBytesToCompound(uint8_t *, int)` | exported |
| `b3ConvertCompoundToBytes` | `uint8_t * b3ConvertCompoundToBytes(b3CompoundData *)` | exported |
| `b3Cos` | `float b3Cos(float)` | inline |
| `b3CreateBakedCompoundShape` | `b3ShapeId b3CreateBakedCompoundShape(b3BodyId, b3ShapeDef *, const b3CompoundData *)` | exported |
| `b3CreateBody` | `b3BodyId b3CreateBody(b3WorldId, const b3BodyDef *)` | exported |
| `b3CreateBoxMesh` | `b3MeshData * b3CreateBoxMesh(b3Vec3, b3Vec3, bool)` | exported |
| `b3CreateCapsuleShape` | `b3ShapeId b3CreateCapsuleShape(b3BodyId, const b3ShapeDef *, const b3Capsule *)` | exported |
| `b3CreateCompound` | `b3CompoundData * b3CreateCompound(const b3CompoundDef *)` | exported |
| `b3CreateCone` | `b3HullData * b3CreateCone(float, float, float, int)` | exported |
| `b3CreateCylinder` | `b3HullData * b3CreateCylinder(float, float, float, int)` | exported |
| `b3CreateDistanceJoint` | `b3JointId b3CreateDistanceJoint(b3WorldId, const b3DistanceJointDef *)` | exported |
| `b3CreateFilterJoint` | `b3JointId b3CreateFilterJoint(b3WorldId, const b3FilterJointDef *)` | exported |
| `b3CreateGrid` | `b3HeightFieldData * b3CreateGrid(int, int, b3Vec3, bool)` | exported |
| `b3CreateGridMesh` | `b3MeshData * b3CreateGridMesh(int, int, float, int, bool)` | exported |
| `b3CreateHeightField` | `b3HeightFieldData * b3CreateHeightField(const b3HeightFieldDef *)` | exported |
| `b3CreateHeightFieldShape` | `b3ShapeId b3CreateHeightFieldShape(b3BodyId, const b3ShapeDef *, const b3HeightFieldData *)` | exported |
| `b3CreateHollowBoxMesh` | `b3MeshData * b3CreateHollowBoxMesh(b3Vec3, b3Vec3)` | exported |
| `b3CreateHull` | `b3HullData * b3CreateHull(const b3Vec3 *, int, int)` | exported |
| `b3CreateHullShape` | `b3ShapeId b3CreateHullShape(b3BodyId, const b3ShapeDef *, const b3HullData *)` | exported |
| `b3CreateMesh` | `b3MeshData * b3CreateMesh(const b3MeshDef *, int *, int)` | exported |
| `b3CreateMeshShape` | `b3ShapeId b3CreateMeshShape(b3BodyId, const b3ShapeDef *, const b3MeshData *, b3Vec3)` | exported |
| `b3CreateMotorJoint` | `b3JointId b3CreateMotorJoint(b3WorldId, const b3MotorJointDef *)` | exported |
| `b3CreateParallelJoint` | `b3JointId b3CreateParallelJoint(b3WorldId, const b3ParallelJointDef *)` | exported |
| `b3CreatePlatformMesh` | `b3MeshData * b3CreatePlatformMesh(b3Vec3, float, float, float)` | exported |
| `b3CreatePrismaticJoint` | `b3JointId b3CreatePrismaticJoint(b3WorldId, const b3PrismaticJointDef *)` | exported |
| `b3CreateRecording` | `b3Recording * b3CreateRecording(int)` | exported |
| `b3CreateRevoluteJoint` | `b3JointId b3CreateRevoluteJoint(b3WorldId, const b3RevoluteJointDef *)` | exported |
| `b3CreateRock` | `b3HullData * b3CreateRock(float)` | exported |
| `b3CreateSphereShape` | `b3ShapeId b3CreateSphereShape(b3BodyId, const b3ShapeDef *, const b3Sphere *)` | exported |
| `b3CreateSphericalJoint` | `b3JointId b3CreateSphericalJoint(b3WorldId, const b3SphericalJointDef *)` | exported |
| `b3CreateTorusMesh` | `b3MeshData * b3CreateTorusMesh(int, int, float, float)` | exported |
| `b3CreateTransformedHullShape` | `b3ShapeId b3CreateTransformedHullShape(b3BodyId, const b3ShapeDef *, const b3HullData *, b3Transform, b3Vec3)` | exported |
| `b3CreateWave` | `b3HeightFieldData * b3CreateWave(int, int, b3Vec3, float, float, bool)` | exported |
| `b3CreateWaveMesh` | `b3MeshData * b3CreateWaveMesh(int, int, float, float, float, float)` | exported |
| `b3CreateWeldJoint` | `b3JointId b3CreateWeldJoint(b3WorldId, const b3WeldJointDef *)` | exported |
| `b3CreateWheelJoint` | `b3JointId b3CreateWheelJoint(b3WorldId, const b3WheelJointDef *)` | exported |
| `b3CreateWorld` | `b3WorldId b3CreateWorld(const b3WorldDef *)` | exported |
| `b3Cross` | `b3Vec3 b3Cross(b3Vec3, b3Vec3)` | inline |
| `b3DefaultBodyDef` | `b3BodyDef b3DefaultBodyDef()` | exported |
| `b3DefaultDebugDraw` | `b3DebugDraw b3DefaultDebugDraw()` | exported |
| `b3DefaultDistanceJointDef` | `b3DistanceJointDef b3DefaultDistanceJointDef()` | exported |
| `b3DefaultExplosionDef` | `b3ExplosionDef b3DefaultExplosionDef()` | exported |
| `b3DefaultFilter` | `b3Filter b3DefaultFilter()` | exported |
| `b3DefaultFilterJointDef` | `b3FilterJointDef b3DefaultFilterJointDef()` | exported |
| `b3DefaultMotorJointDef` | `b3MotorJointDef b3DefaultMotorJointDef()` | exported |
| `b3DefaultParallelJointDef` | `b3ParallelJointDef b3DefaultParallelJointDef()` | exported |
| `b3DefaultPrismaticJointDef` | `b3PrismaticJointDef b3DefaultPrismaticJointDef()` | exported |
| `b3DefaultQueryFilter` | `b3QueryFilter b3DefaultQueryFilter()` | exported |
| `b3DefaultRevoluteJointDef` | `b3RevoluteJointDef b3DefaultRevoluteJointDef()` | exported |
| `b3DefaultShapeDef` | `b3ShapeDef b3DefaultShapeDef()` | exported |
| `b3DefaultSphericalJointDef` | `b3SphericalJointDef b3DefaultSphericalJointDef()` | exported |
| `b3DefaultSurfaceMaterial` | `b3SurfaceMaterial b3DefaultSurfaceMaterial()` | exported |
| `b3DefaultWeldJointDef` | `b3WeldJointDef b3DefaultWeldJointDef()` | exported |
| `b3DefaultWheelJointDef` | `b3WheelJointDef b3DefaultWheelJointDef()` | exported |
| `b3DefaultWorldDef` | `b3WorldDef b3DefaultWorldDef()` | exported |
| `b3DestroyBody` | `void b3DestroyBody(b3BodyId)` | exported |
| `b3DestroyCompound` | `void b3DestroyCompound(b3CompoundData *)` | exported |
| `b3DestroyHeightField` | `void b3DestroyHeightField(b3HeightFieldData *)` | exported |
| `b3DestroyHull` | `void b3DestroyHull(b3HullData *)` | exported |
| `b3DestroyJoint` | `void b3DestroyJoint(b3JointId, bool)` | exported |
| `b3DestroyMesh` | `void b3DestroyMesh(b3MeshData *)` | exported |
| `b3DestroyRecording` | `void b3DestroyRecording(b3Recording *)` | exported |
| `b3DestroyShape` | `void b3DestroyShape(b3ShapeId, bool)` | exported |
| `b3DestroyWorld` | `void b3DestroyWorld(b3WorldId)` | exported |
| `b3Det` | `float b3Det(b3Matrix3)` | inline |
| `b3Distance` | `float b3Distance(b3Vec3, b3Vec3)` | inline |
| `b3DistanceJoint_EnableLimit` | `void b3DistanceJoint_EnableLimit(b3JointId, bool)` | exported |
| `b3DistanceJoint_EnableMotor` | `void b3DistanceJoint_EnableMotor(b3JointId, bool)` | exported |
| `b3DistanceJoint_EnableSpring` | `void b3DistanceJoint_EnableSpring(b3JointId, bool)` | exported |
| `b3DistanceJoint_GetCurrentLength` | `float b3DistanceJoint_GetCurrentLength(b3JointId)` | exported |
| `b3DistanceJoint_GetLength` | `float b3DistanceJoint_GetLength(b3JointId)` | exported |
| `b3DistanceJoint_GetMaxLength` | `float b3DistanceJoint_GetMaxLength(b3JointId)` | exported |
| `b3DistanceJoint_GetMaxMotorForce` | `float b3DistanceJoint_GetMaxMotorForce(b3JointId)` | exported |
| `b3DistanceJoint_GetMinLength` | `float b3DistanceJoint_GetMinLength(b3JointId)` | exported |
| `b3DistanceJoint_GetMotorForce` | `float b3DistanceJoint_GetMotorForce(b3JointId)` | exported |
| `b3DistanceJoint_GetMotorSpeed` | `float b3DistanceJoint_GetMotorSpeed(b3JointId)` | exported |
| `b3DistanceJoint_GetSpringDampingRatio` | `float b3DistanceJoint_GetSpringDampingRatio(b3JointId)` | exported |
| `b3DistanceJoint_GetSpringForceRange` | `void b3DistanceJoint_GetSpringForceRange(b3JointId, float *, float *)` | exported |
| `b3DistanceJoint_GetSpringHertz` | `float b3DistanceJoint_GetSpringHertz(b3JointId)` | exported |
| `b3DistanceJoint_IsLimitEnabled` | `bool b3DistanceJoint_IsLimitEnabled(b3JointId)` | exported |
| `b3DistanceJoint_IsMotorEnabled` | `bool b3DistanceJoint_IsMotorEnabled(b3JointId)` | exported |
| `b3DistanceJoint_IsSpringEnabled` | `bool b3DistanceJoint_IsSpringEnabled(b3JointId)` | exported |
| `b3DistanceJoint_SetLength` | `void b3DistanceJoint_SetLength(b3JointId, float)` | exported |
| `b3DistanceJoint_SetLengthRange` | `void b3DistanceJoint_SetLengthRange(b3JointId, float, float)` | exported |
| `b3DistanceJoint_SetMaxMotorForce` | `void b3DistanceJoint_SetMaxMotorForce(b3JointId, float)` | exported |
| `b3DistanceJoint_SetMotorSpeed` | `void b3DistanceJoint_SetMotorSpeed(b3JointId, float)` | exported |
| `b3DistanceJoint_SetSpringDampingRatio` | `void b3DistanceJoint_SetSpringDampingRatio(b3JointId, float)` | exported |
| `b3DistanceJoint_SetSpringForceRange` | `void b3DistanceJoint_SetSpringForceRange(b3JointId, float, float)` | exported |
| `b3DistanceJoint_SetSpringHertz` | `void b3DistanceJoint_SetSpringHertz(b3JointId, float)` | exported |
| `b3DistanceSquared` | `float b3DistanceSquared(b3Vec3, b3Vec3)` | inline |
| `b3Dot` | `float b3Dot(b3Vec3, b3Vec3)` | inline |
| `b3DotQuat` | `float b3DotQuat(b3Quat, b3Quat)` | inline |
| `b3DumpHeightData` | `void b3DumpHeightData(const b3HeightFieldDef *, const char *)` | exported |
| `b3DynamicTree_BoxCast` | `b3TreeStats b3DynamicTree_BoxCast(const b3DynamicTree *, const b3BoxCastInput *, uint64_t, bool, b3TreeBoxCastCallbackFcn *, void *)` | exported |
| `b3DynamicTree_Create` | `b3DynamicTree b3DynamicTree_Create(int)` | exported |
| `b3DynamicTree_CreateProxy` | `int b3DynamicTree_CreateProxy(b3DynamicTree *, b3AABB, uint64_t, uint64_t)` | exported |
| `b3DynamicTree_Destroy` | `void b3DynamicTree_Destroy(b3DynamicTree *)` | exported |
| `b3DynamicTree_DestroyProxy` | `void b3DynamicTree_DestroyProxy(b3DynamicTree *, int)` | exported |
| `b3DynamicTree_EnlargeProxy` | `void b3DynamicTree_EnlargeProxy(b3DynamicTree *, int, b3AABB)` | exported |
| `b3DynamicTree_GetAABB` | `b3AABB b3DynamicTree_GetAABB(const b3DynamicTree *, int)` | inline |
| `b3DynamicTree_GetAreaRatio` | `float b3DynamicTree_GetAreaRatio(const b3DynamicTree *)` | exported |
| `b3DynamicTree_GetByteCount` | `int b3DynamicTree_GetByteCount(const b3DynamicTree *)` | exported |
| `b3DynamicTree_GetCategoryBits` | `uint64_t b3DynamicTree_GetCategoryBits(b3DynamicTree *, int)` | exported |
| `b3DynamicTree_GetHeight` | `int b3DynamicTree_GetHeight(const b3DynamicTree *)` | exported |
| `b3DynamicTree_GetProxyCount` | `int b3DynamicTree_GetProxyCount(const b3DynamicTree *)` | exported |
| `b3DynamicTree_GetRootBounds` | `b3AABB b3DynamicTree_GetRootBounds(const b3DynamicTree *)` | exported |
| `b3DynamicTree_GetUserData` | `uint64_t b3DynamicTree_GetUserData(const b3DynamicTree *, int)` | inline |
| `b3DynamicTree_Load` | `b3DynamicTree b3DynamicTree_Load(const char *, float)` | exported |
| `b3DynamicTree_MoveProxy` | `void b3DynamicTree_MoveProxy(b3DynamicTree *, int, b3AABB)` | exported |
| `b3DynamicTree_Query` | `b3TreeStats b3DynamicTree_Query(const b3DynamicTree *, b3AABB, uint64_t, bool, b3TreeQueryCallbackFcn *, void *)` | exported |
| `b3DynamicTree_QueryClosest` | `b3TreeStats b3DynamicTree_QueryClosest(const b3DynamicTree *, b3Vec3, uint64_t, bool, b3TreeQueryClosestCallbackFcn *, void *, float *)` | exported |
| `b3DynamicTree_RayCast` | `b3TreeStats b3DynamicTree_RayCast(const b3DynamicTree *, const b3RayCastInput *, uint64_t, bool, b3TreeRayCastCallbackFcn *, void *)` | exported |
| `b3DynamicTree_Rebuild` | `int b3DynamicTree_Rebuild(b3DynamicTree *, bool)` | exported |
| `b3DynamicTree_Save` | `void b3DynamicTree_Save(const b3DynamicTree *, const char *)` | exported |
| `b3DynamicTree_SetCategoryBits` | `void b3DynamicTree_SetCategoryBits(b3DynamicTree *, int, uint64_t)` | exported |
| `b3DynamicTree_Validate` | `void b3DynamicTree_Validate(const b3DynamicTree *)` | exported |
| `b3DynamicTree_ValidateNoEnlarged` | `void b3DynamicTree_ValidateNoEnlarged(const b3DynamicTree *)` | exported |
| `b3GetAxisAngle` | `b3Vec3 b3GetAxisAngle(float *, b3Quat)` | inline |
| `b3GetByteCount` | `int b3GetByteCount()` | exported |
| `b3GetCompoundCapsule` | `b3CompoundCapsule b3GetCompoundCapsule(const b3CompoundData *, int)` | exported |
| `b3GetCompoundChild` | `b3ChildShape b3GetCompoundChild(const b3CompoundData *, int)` | exported |
| `b3GetCompoundHull` | `b3CompoundHull b3GetCompoundHull(const b3CompoundData *, int)` | exported |
| `b3GetCompoundMaterials` | `const b3SurfaceMaterial * b3GetCompoundMaterials(const b3CompoundData *)` | exported |
| `b3GetCompoundMesh` | `b3CompoundMesh b3GetCompoundMesh(const b3CompoundData *, int)` | exported |
| `b3GetCompoundSphere` | `b3CompoundSphere b3GetCompoundSphere(const b3CompoundData *, int)` | exported |
| `b3GetGraphColor` | `b3HexColor b3GetGraphColor(int)` | exported |
| `b3GetHeight` | `int b3GetHeight(const b3MeshData *)` | exported |
| `b3GetHeightFieldCompressedHeights` | `const uint16_t * b3GetHeightFieldCompressedHeights(const b3HeightFieldData *)` | inline |
| `b3GetHeightFieldFlags` | `const uint8_t * b3GetHeightFieldFlags(const b3HeightFieldData *)` | inline |
| `b3GetHeightFieldMaterialIndices` | `const uint8_t * b3GetHeightFieldMaterialIndices(const b3HeightFieldData *)` | inline |
| `b3GetHullEdges` | `const b3HullHalfEdge * b3GetHullEdges(const b3HullData *)` | inline |
| `b3GetHullFaces` | `const b3HullFace * b3GetHullFaces(const b3HullData *)` | inline |
| `b3GetHullPlanes` | `const b3Plane * b3GetHullPlanes(const b3HullData *)` | inline |
| `b3GetHullPoints` | `const b3Vec3 * b3GetHullPoints(const b3HullData *)` | inline |
| `b3GetHullVertices` | `const b3HullVertex * b3GetHullVertices(const b3HullData *)` | inline |
| `b3GetLengthAndNormalize` | `b3Vec3 b3GetLengthAndNormalize(float *, b3Vec3)` | inline |
| `b3GetLengthUnitsPerMeter` | `float b3GetLengthUnitsPerMeter()` | inline |
| `b3GetMaxWorldCount` | `int b3GetMaxWorldCount()` | exported |
| `b3GetMeshFlags` | `const uint8_t * b3GetMeshFlags(const b3MeshData *)` | inline |
| `b3GetMeshMaterialIndices` | `const uint8_t * b3GetMeshMaterialIndices(const b3MeshData *)` | inline |
| `b3GetMeshNodes` | `const b3MeshNode * b3GetMeshNodes(const b3MeshData *)` | inline |
| `b3GetMeshTriangles` | `const b3MeshTriangle * b3GetMeshTriangles(const b3MeshData *)` | inline |
| `b3GetMeshVertices` | `const b3Vec3 * b3GetMeshVertices(const b3MeshData *)` | inline |
| `b3GetMilliseconds` | `float b3GetMilliseconds(uint64_t)` | exported |
| `b3GetMillisecondsAndReset` | `float b3GetMillisecondsAndReset(uint64_t *)` | exported |
| `b3GetQuatAngle` | `float b3GetQuatAngle(b3Quat)` | inline |
| `b3GetStallThreshold` | `float b3GetStallThreshold()` | inline |
| `b3GetSweepTransform` | `b3Transform b3GetSweepTransform(const b3Sweep *, float)` | exported |
| `b3GetSwingAngle` | `float b3GetSwingAngle(b3Quat)` | inline |
| `b3GetTicks` | `uint64_t b3GetTicks()` | exported |
| `b3GetTwistAngle` | `float b3GetTwistAngle(b3Quat)` | inline |
| `b3GetVersion` | `b3Version b3GetVersion()` | exported |
| `b3GetWorldCount` | `int b3GetWorldCount()` | exported |
| `b3Hash` | `uint32_t b3Hash(uint32_t, const uint8_t *, int)` | exported |
| `b3InvMulQuat` | `b3Quat b3InvMulQuat(b3Quat, b3Quat)` | inline |
| `b3InvMulTransforms` | `b3Transform b3InvMulTransforms(b3Transform, b3Transform)` | inline |
| `b3InvMulWorldTransforms` | `b3Transform b3InvMulWorldTransforms(b3WorldTransform, b3WorldTransform)` | inline |
| `b3InvRotateVector` | `b3Vec3 b3InvRotateVector(b3Quat, b3Vec3)` | inline |
| `b3InvTransformPoint` | `b3Vec3 b3InvTransformPoint(b3Transform, b3Vec3)` | inline |
| `b3InvTransformWorldPoint` | `b3Vec3 b3InvTransformWorldPoint(b3WorldTransform, b3Pos)` | inline |
| `b3InvertMatrix` | `b3Matrix3 b3InvertMatrix(b3Matrix3)` | inline |
| `b3InvertT` | `b3Matrix3 b3InvertT(b3Matrix3)` | inline |
| `b3InvertTransform` | `b3Transform b3InvertTransform(b3Transform)` | inline |
| `b3IsBoundedAABB` | `bool b3IsBoundedAABB(b3AABB)` | exported |
| `b3IsDoublePrecision` | `bool b3IsDoublePrecision()` | exported |
| `b3IsNormalized` | `bool b3IsNormalized(b3Vec3)` | inline |
| `b3IsNormalizedQuat` | `bool b3IsNormalizedQuat(b3Quat)` | inline |
| `b3IsSaneAABB` | `bool b3IsSaneAABB(b3AABB)` | exported |
| `b3IsValidAABB` | `bool b3IsValidAABB(b3AABB)` | exported |
| `b3IsValidFloat` | `bool b3IsValidFloat(float)` | exported |
| `b3IsValidMatrix3` | `bool b3IsValidMatrix3(b3Matrix3)` | exported |
| `b3IsValidPlane` | `bool b3IsValidPlane(b3Plane)` | exported |
| `b3IsValidPosition` | `bool b3IsValidPosition(b3Pos)` | exported |
| `b3IsValidQuat` | `bool b3IsValidQuat(b3Quat)` | exported |
| `b3IsValidRay` | `bool b3IsValidRay(const b3RayCastInput *)` | exported |
| `b3IsValidTransform` | `bool b3IsValidTransform(b3Transform)` | exported |
| `b3IsValidVec3` | `bool b3IsValidVec3(b3Vec3)` | exported |
| `b3IsValidWorldTransform` | `bool b3IsValidWorldTransform(b3WorldTransform)` | exported |
| `b3Joint_GetAngularSeparation` | `float b3Joint_GetAngularSeparation(b3JointId)` | exported |
| `b3Joint_GetBodyA` | `b3BodyId b3Joint_GetBodyA(b3JointId)` | exported |
| `b3Joint_GetBodyB` | `b3BodyId b3Joint_GetBodyB(b3JointId)` | exported |
| `b3Joint_GetCollideConnected` | `bool b3Joint_GetCollideConnected(b3JointId)` | exported |
| `b3Joint_GetConstraintForce` | `b3Vec3 b3Joint_GetConstraintForce(b3JointId)` | exported |
| `b3Joint_GetConstraintTorque` | `b3Vec3 b3Joint_GetConstraintTorque(b3JointId)` | exported |
| `b3Joint_GetConstraintTuning` | `void b3Joint_GetConstraintTuning(b3JointId, float *, float *)` | exported |
| `b3Joint_GetForceThreshold` | `float b3Joint_GetForceThreshold(b3JointId)` | exported |
| `b3Joint_GetLinearSeparation` | `float b3Joint_GetLinearSeparation(b3JointId)` | exported |
| `b3Joint_GetLocalFrameA` | `b3Transform b3Joint_GetLocalFrameA(b3JointId)` | exported |
| `b3Joint_GetLocalFrameB` | `b3Transform b3Joint_GetLocalFrameB(b3JointId)` | exported |
| `b3Joint_GetTorqueThreshold` | `float b3Joint_GetTorqueThreshold(b3JointId)` | exported |
| `b3Joint_GetType` | `b3JointType b3Joint_GetType(b3JointId)` | exported |
| `b3Joint_GetUserData` | `void * b3Joint_GetUserData(b3JointId)` | exported |
| `b3Joint_GetWorld` | `b3WorldId b3Joint_GetWorld(b3JointId)` | exported |
| `b3Joint_IsValid` | `bool b3Joint_IsValid(b3JointId)` | exported |
| `b3Joint_SetCollideConnected` | `void b3Joint_SetCollideConnected(b3JointId, bool)` | exported |
| `b3Joint_SetConstraintTuning` | `void b3Joint_SetConstraintTuning(b3JointId, float, float)` | exported |
| `b3Joint_SetForceThreshold` | `void b3Joint_SetForceThreshold(b3JointId, float)` | exported |
| `b3Joint_SetLocalFrameA` | `void b3Joint_SetLocalFrameA(b3JointId, b3Transform)` | exported |
| `b3Joint_SetLocalFrameB` | `void b3Joint_SetLocalFrameB(b3JointId, b3Transform)` | exported |
| `b3Joint_SetTorqueThreshold` | `void b3Joint_SetTorqueThreshold(b3JointId, float)` | exported |
| `b3Joint_SetUserData` | `void b3Joint_SetUserData(b3JointId, void *)` | exported |
| `b3Joint_WakeBodies` | `void b3Joint_WakeBodies(b3JointId)` | exported |
| `b3Length` | `float b3Length(b3Vec3)` | inline |
| `b3LengthSquared` | `float b3LengthSquared(b3Vec3)` | inline |
| `b3Lerp` | `b3Vec3 b3Lerp(b3Vec3, b3Vec3, float)` | inline |
| `b3LerpFloat` | `float b3LerpFloat(float, float, float)` | inline |
| `b3LerpPosition` | `b3Pos b3LerpPosition(b3Pos, b3Pos, float)` | inline |
| `b3LineDistance` | `b3SegmentDistanceResult b3LineDistance(b3Vec3, b3Vec3, b3Vec3, b3Vec3)` | exported |
| `b3LoadBodyId` | `b3BodyId b3LoadBodyId(uint64_t)` | inline |
| `b3LoadContactId` | `b3ContactId b3LoadContactId(uint32_t *)` | inline |
| `b3LoadHeightField` | `b3HeightFieldData * b3LoadHeightField(const char *)` | exported |
| `b3LoadJointId` | `b3JointId b3LoadJointId(uint64_t)` | inline |
| `b3LoadRecordingFromFile` | `b3Recording * b3LoadRecordingFromFile(const char *)` | exported |
| `b3LoadShapeId` | `b3ShapeId b3LoadShapeId(uint64_t)` | inline |
| `b3LoadWorldId` | `b3WorldId b3LoadWorldId(uint32_t)` | inline |
| `b3MakeAABB` | `b3AABB b3MakeAABB(const b3Vec3 *, int, float)` | inline |
| `b3MakeBoxHull` | `b3BoxHull b3MakeBoxHull(float, float, float)` | exported |
| `b3MakeCubeHull` | `b3BoxHull b3MakeCubeHull(float)` | exported |
| `b3MakeDebugColor` | `uint32_t b3MakeDebugColor(b3HexColor, b3DebugMaterial)` | inline |
| `b3MakeMatrixFromQuat` | `b3Matrix3 b3MakeMatrixFromQuat(b3Quat)` | inline |
| `b3MakeOffsetBoxHull` | `b3BoxHull b3MakeOffsetBoxHull(float, float, float, b3Vec3)` | exported |
| `b3MakeQuatFromAxisAngle` | `b3Quat b3MakeQuatFromAxisAngle(b3Vec3, float)` | inline |
| `b3MakeQuatFromMatrix` | `b3Quat b3MakeQuatFromMatrix(const b3Matrix3 *)` | exported |
| `b3MakeScaledBoxHull` | `b3BoxHull b3MakeScaledBoxHull(b3Vec3, b3Transform, b3Vec3)` | exported |
| `b3MakeTransformedBoxHull` | `b3BoxHull b3MakeTransformedBoxHull(float, float, float, b3Transform)` | exported |
| `b3MakeWorldTransform` | `b3WorldTransform b3MakeWorldTransform(b3Transform)` | inline |
| `b3Max` | `b3Vec3 b3Max(b3Vec3, b3Vec3)` | inline |
| `b3MaxFloat` | `float b3MaxFloat(float, float)` | inline |
| `b3MaxInt` | `int b3MaxInt(int, int)` | inline |
| `b3Min` | `b3Vec3 b3Min(b3Vec3, b3Vec3)` | inline |
| `b3MinFloat` | `float b3MinFloat(float, float)` | inline |
| `b3MinInt` | `int b3MinInt(int, int)` | inline |
| `b3MotorJoint_GetAngularDampingRatio` | `float b3MotorJoint_GetAngularDampingRatio(b3JointId)` | exported |
| `b3MotorJoint_GetAngularHertz` | `float b3MotorJoint_GetAngularHertz(b3JointId)` | exported |
| `b3MotorJoint_GetAngularVelocity` | `b3Vec3 b3MotorJoint_GetAngularVelocity(b3JointId)` | exported |
| `b3MotorJoint_GetLinearDampingRatio` | `float b3MotorJoint_GetLinearDampingRatio(b3JointId)` | exported |
| `b3MotorJoint_GetLinearHertz` | `float b3MotorJoint_GetLinearHertz(b3JointId)` | exported |
| `b3MotorJoint_GetLinearVelocity` | `b3Vec3 b3MotorJoint_GetLinearVelocity(b3JointId)` | exported |
| `b3MotorJoint_GetMaxSpringForce` | `float b3MotorJoint_GetMaxSpringForce(b3JointId)` | exported |
| `b3MotorJoint_GetMaxSpringTorque` | `float b3MotorJoint_GetMaxSpringTorque(b3JointId)` | exported |
| `b3MotorJoint_GetMaxVelocityForce` | `float b3MotorJoint_GetMaxVelocityForce(b3JointId)` | exported |
| `b3MotorJoint_GetMaxVelocityTorque` | `float b3MotorJoint_GetMaxVelocityTorque(b3JointId)` | exported |
| `b3MotorJoint_SetAngularDampingRatio` | `void b3MotorJoint_SetAngularDampingRatio(b3JointId, float)` | exported |
| `b3MotorJoint_SetAngularHertz` | `void b3MotorJoint_SetAngularHertz(b3JointId, float)` | exported |
| `b3MotorJoint_SetAngularVelocity` | `void b3MotorJoint_SetAngularVelocity(b3JointId, b3Vec3)` | exported |
| `b3MotorJoint_SetLinearDampingRatio` | `void b3MotorJoint_SetLinearDampingRatio(b3JointId, float)` | exported |
| `b3MotorJoint_SetLinearHertz` | `void b3MotorJoint_SetLinearHertz(b3JointId, float)` | exported |
| `b3MotorJoint_SetLinearVelocity` | `void b3MotorJoint_SetLinearVelocity(b3JointId, b3Vec3)` | exported |
| `b3MotorJoint_SetMaxSpringForce` | `void b3MotorJoint_SetMaxSpringForce(b3JointId, float)` | exported |
| `b3MotorJoint_SetMaxSpringTorque` | `void b3MotorJoint_SetMaxSpringTorque(b3JointId, float)` | exported |
| `b3MotorJoint_SetMaxVelocityForce` | `void b3MotorJoint_SetMaxVelocityForce(b3JointId, float)` | exported |
| `b3MotorJoint_SetMaxVelocityTorque` | `void b3MotorJoint_SetMaxVelocityTorque(b3JointId, float)` | exported |
| `b3Mul` | `b3Vec3 b3Mul(b3Vec3, b3Vec3)` | inline |
| `b3MulAdd` | `b3Vec3 b3MulAdd(b3Vec3, float, b3Vec3)` | inline |
| `b3MulMM` | `b3Matrix3 b3MulMM(b3Matrix3, b3Matrix3)` | inline |
| `b3MulMV` | `b3Vec3 b3MulMV(b3Matrix3, b3Vec3)` | inline |
| `b3MulQuat` | `b3Quat b3MulQuat(b3Quat, b3Quat)` | inline |
| `b3MulSM` | `b3Matrix3 b3MulSM(float, b3Matrix3)` | inline |
| `b3MulSV` | `b3Vec3 b3MulSV(float, b3Vec3)` | inline |
| `b3MulSub` | `b3Vec3 b3MulSub(b3Vec3, float, b3Vec3)` | inline |
| `b3MulTransforms` | `b3Transform b3MulTransforms(b3Transform, b3Transform)` | inline |
| `b3MulWorldTransforms` | `b3WorldTransform b3MulWorldTransforms(b3WorldTransform, b3Transform)` | inline |
| `b3NLerp` | `b3Quat b3NLerp(b3Quat, b3Quat, float)` | inline |
| `b3Neg` | `b3Vec3 b3Neg(b3Vec3)` | inline |
| `b3NegateMat3` | `b3Matrix3 b3NegateMat3(b3Matrix3)` | inline |
| `b3NegateQuat` | `b3Quat b3NegateQuat(b3Quat)` | inline |
| `b3Normalize` | `b3Vec3 b3Normalize(b3Vec3)` | inline |
| `b3NormalizeQuat` | `b3Quat b3NormalizeQuat(b3Quat)` | inline |
| `b3OffsetAABB` | `b3AABB b3OffsetAABB(b3AABB, b3Pos)` | inline |
| `b3OffsetPos` | `b3Pos b3OffsetPos(b3Pos, b3Vec3)` | inline |
| `b3OverlapCapsule` | `bool b3OverlapCapsule(const b3Capsule *, b3Transform, const b3ShapeProxy *)` | exported |
| `b3OverlapCompound` | `bool b3OverlapCompound(const b3CompoundData *, b3Transform, const b3ShapeProxy *)` | exported |
| `b3OverlapHeightField` | `bool b3OverlapHeightField(const b3HeightFieldData *, b3Transform, const b3ShapeProxy *)` | exported |
| `b3OverlapHull` | `bool b3OverlapHull(const b3HullData *, b3Transform, const b3ShapeProxy *)` | exported |
| `b3OverlapMesh` | `bool b3OverlapMesh(const b3Mesh *, b3Transform, const b3ShapeProxy *)` | exported |
| `b3OverlapSphere` | `bool b3OverlapSphere(const b3Sphere *, b3Transform, const b3ShapeProxy *)` | exported |
| `b3ParallelJoint_GetMaxTorque` | `float b3ParallelJoint_GetMaxTorque(b3JointId)` | exported |
| `b3ParallelJoint_GetSpringDampingRatio` | `float b3ParallelJoint_GetSpringDampingRatio(b3JointId)` | exported |
| `b3ParallelJoint_GetSpringHertz` | `float b3ParallelJoint_GetSpringHertz(b3JointId)` | exported |
| `b3ParallelJoint_SetMaxTorque` | `void b3ParallelJoint_SetMaxTorque(b3JointId, float)` | exported |
| `b3ParallelJoint_SetSpringDampingRatio` | `void b3ParallelJoint_SetSpringDampingRatio(b3JointId, float)` | exported |
| `b3ParallelJoint_SetSpringHertz` | `void b3ParallelJoint_SetSpringHertz(b3JointId, float)` | exported |
| `b3Perp` | `b3Vec3 b3Perp(b3Vec3)` | inline |
| `b3PointToSegmentDistance` | `b3Vec3 b3PointToSegmentDistance(b3Vec3, b3Vec3, b3Vec3)` | exported |
| `b3PrismaticJoint_EnableLimit` | `void b3PrismaticJoint_EnableLimit(b3JointId, bool)` | exported |
| `b3PrismaticJoint_EnableMotor` | `void b3PrismaticJoint_EnableMotor(b3JointId, bool)` | exported |
| `b3PrismaticJoint_EnableSpring` | `void b3PrismaticJoint_EnableSpring(b3JointId, bool)` | exported |
| `b3PrismaticJoint_GetLowerLimit` | `float b3PrismaticJoint_GetLowerLimit(b3JointId)` | exported |
| `b3PrismaticJoint_GetMaxMotorForce` | `float b3PrismaticJoint_GetMaxMotorForce(b3JointId)` | exported |
| `b3PrismaticJoint_GetMotorForce` | `float b3PrismaticJoint_GetMotorForce(b3JointId)` | exported |
| `b3PrismaticJoint_GetMotorSpeed` | `float b3PrismaticJoint_GetMotorSpeed(b3JointId)` | exported |
| `b3PrismaticJoint_GetSpeed` | `float b3PrismaticJoint_GetSpeed(b3JointId)` | exported |
| `b3PrismaticJoint_GetSpringDampingRatio` | `float b3PrismaticJoint_GetSpringDampingRatio(b3JointId)` | exported |
| `b3PrismaticJoint_GetSpringHertz` | `float b3PrismaticJoint_GetSpringHertz(b3JointId)` | exported |
| `b3PrismaticJoint_GetTargetTranslation` | `float b3PrismaticJoint_GetTargetTranslation(b3JointId)` | exported |
| `b3PrismaticJoint_GetTranslation` | `float b3PrismaticJoint_GetTranslation(b3JointId)` | exported |
| `b3PrismaticJoint_GetUpperLimit` | `float b3PrismaticJoint_GetUpperLimit(b3JointId)` | exported |
| `b3PrismaticJoint_IsLimitEnabled` | `bool b3PrismaticJoint_IsLimitEnabled(b3JointId)` | exported |
| `b3PrismaticJoint_IsMotorEnabled` | `bool b3PrismaticJoint_IsMotorEnabled(b3JointId)` | exported |
| `b3PrismaticJoint_IsSpringEnabled` | `bool b3PrismaticJoint_IsSpringEnabled(b3JointId)` | exported |
| `b3PrismaticJoint_SetLimits` | `void b3PrismaticJoint_SetLimits(b3JointId, float, float)` | exported |
| `b3PrismaticJoint_SetMaxMotorForce` | `void b3PrismaticJoint_SetMaxMotorForce(b3JointId, float)` | exported |
| `b3PrismaticJoint_SetMotorSpeed` | `void b3PrismaticJoint_SetMotorSpeed(b3JointId, float)` | exported |
| `b3PrismaticJoint_SetSpringDampingRatio` | `void b3PrismaticJoint_SetSpringDampingRatio(b3JointId, float)` | exported |
| `b3PrismaticJoint_SetSpringHertz` | `void b3PrismaticJoint_SetSpringHertz(b3JointId, float)` | exported |
| `b3PrismaticJoint_SetTargetTranslation` | `void b3PrismaticJoint_SetTargetTranslation(b3JointId, float)` | exported |
| `b3QueryCompound` | `void b3QueryCompound(const b3CompoundData *, b3AABB, b3CompoundQueryFcn *, void *)` | exported |
| `b3QueryHeightField` | `void b3QueryHeightField(const b3HeightFieldData *, b3AABB, b3MeshQueryFcn *, void *)` | exported |
| `b3QueryMesh` | `void b3QueryMesh(const b3Mesh *, const b3AABB, b3MeshQueryFcn *, void *)` | exported |
| `b3RayCastCapsule` | `b3CastOutput b3RayCastCapsule(const b3Capsule *, const b3RayCastInput *)` | exported |
| `b3RayCastCompound` | `b3CastOutput b3RayCastCompound(const b3CompoundData *, const b3RayCastInput *)` | exported |
| `b3RayCastHeightField` | `b3CastOutput b3RayCastHeightField(const b3HeightFieldData *, const b3RayCastInput *)` | exported |
| `b3RayCastHollowSphere` | `b3CastOutput b3RayCastHollowSphere(const b3Sphere *, const b3RayCastInput *)` | exported |
| `b3RayCastHull` | `b3CastOutput b3RayCastHull(const b3HullData *, const b3RayCastInput *)` | exported |
| `b3RayCastMesh` | `b3CastOutput b3RayCastMesh(const b3Mesh *, const b3RayCastInput *)` | exported |
| `b3RayCastSphere` | `b3CastOutput b3RayCastSphere(const b3Sphere *, const b3RayCastInput *)` | exported |
| `b3RecPlayer_Create` | `b3RecPlayer * b3RecPlayer_Create(const void *, int, int)` | exported |
| `b3RecPlayer_Destroy` | `void b3RecPlayer_Destroy(b3RecPlayer *)` | exported |
| `b3RecPlayer_DrawFrameQueries` | `void b3RecPlayer_DrawFrameQueries(b3RecPlayer *, b3DebugDraw *, int, int)` | exported |
| `b3RecPlayer_GetBodyCount` | `int b3RecPlayer_GetBodyCount(const b3RecPlayer *)` | exported |
| `b3RecPlayer_GetBodyId` | `b3BodyId b3RecPlayer_GetBodyId(const b3RecPlayer *, int)` | exported |
| `b3RecPlayer_GetDivergeFrame` | `int b3RecPlayer_GetDivergeFrame(const b3RecPlayer *)` | exported |
| `b3RecPlayer_GetFrame` | `int b3RecPlayer_GetFrame(const b3RecPlayer *)` | exported |
| `b3RecPlayer_GetFrameCount` | `int b3RecPlayer_GetFrameCount(const b3RecPlayer *)` | exported |
| `b3RecPlayer_GetFrameQuery` | `b3RecQueryInfo b3RecPlayer_GetFrameQuery(const b3RecPlayer *, int)` | exported |
| `b3RecPlayer_GetFrameQueryCount` | `int b3RecPlayer_GetFrameQueryCount(const b3RecPlayer *)` | exported |
| `b3RecPlayer_GetFrameQueryHit` | `b3RecQueryHit b3RecPlayer_GetFrameQueryHit(const b3RecPlayer *, int, int)` | exported |
| `b3RecPlayer_GetInfo` | `b3RecPlayerInfo b3RecPlayer_GetInfo(const b3RecPlayer *)` | exported |
| `b3RecPlayer_GetKeyframeBudget` | `size_t b3RecPlayer_GetKeyframeBudget(const b3RecPlayer *)` | exported |
| `b3RecPlayer_GetKeyframeBytes` | `size_t b3RecPlayer_GetKeyframeBytes(const b3RecPlayer *)` | exported |
| `b3RecPlayer_GetKeyframeInterval` | `int b3RecPlayer_GetKeyframeInterval(const b3RecPlayer *)` | exported |
| `b3RecPlayer_GetKeyframeMinInterval` | `int b3RecPlayer_GetKeyframeMinInterval(const b3RecPlayer *)` | exported |
| `b3RecPlayer_GetWorldId` | `b3WorldId b3RecPlayer_GetWorldId(const b3RecPlayer *)` | exported |
| `b3RecPlayer_HasDiverged` | `bool b3RecPlayer_HasDiverged(const b3RecPlayer *)` | exported |
| `b3RecPlayer_IsAtEnd` | `bool b3RecPlayer_IsAtEnd(const b3RecPlayer *)` | exported |
| `b3RecPlayer_IsAtPreStep` | `bool b3RecPlayer_IsAtPreStep(const b3RecPlayer *)` | exported |
| `b3RecPlayer_Restart` | `void b3RecPlayer_Restart(b3RecPlayer *)` | exported |
| `b3RecPlayer_SeekFrame` | `void b3RecPlayer_SeekFrame(b3RecPlayer *, int)` | exported |
| `b3RecPlayer_SetDebugShapeCallbacks` | `void b3RecPlayer_SetDebugShapeCallbacks(b3RecPlayer *, b3CreateDebugShapeCallback *, b3DestroyDebugShapeCallback *, void *)` | exported |
| `b3RecPlayer_SetKeyframePolicy` | `void b3RecPlayer_SetKeyframePolicy(b3RecPlayer *, size_t, int)` | exported |
| `b3RecPlayer_SetWorkerCount` | `void b3RecPlayer_SetWorkerCount(b3RecPlayer *, int)` | exported |
| `b3RecPlayer_StepFrame` | `bool b3RecPlayer_StepFrame(b3RecPlayer *)` | exported |
| `b3RecPlayer_SubStepFrame` | `void b3RecPlayer_SubStepFrame(b3RecPlayer *)` | exported |
| `b3Recording_GetData` | `const uint8_t * b3Recording_GetData(const b3Recording *)` | exported |
| `b3Recording_GetSize` | `int b3Recording_GetSize(const b3Recording *)` | exported |
| `b3RevoluteJoint_EnableLimit` | `void b3RevoluteJoint_EnableLimit(b3JointId, bool)` | exported |
| `b3RevoluteJoint_EnableMotor` | `void b3RevoluteJoint_EnableMotor(b3JointId, bool)` | exported |
| `b3RevoluteJoint_EnableSpring` | `void b3RevoluteJoint_EnableSpring(b3JointId, bool)` | exported |
| `b3RevoluteJoint_GetAngle` | `float b3RevoluteJoint_GetAngle(b3JointId)` | exported |
| `b3RevoluteJoint_GetLowerLimit` | `float b3RevoluteJoint_GetLowerLimit(b3JointId)` | exported |
| `b3RevoluteJoint_GetMaxMotorTorque` | `float b3RevoluteJoint_GetMaxMotorTorque(b3JointId)` | exported |
| `b3RevoluteJoint_GetMotorSpeed` | `float b3RevoluteJoint_GetMotorSpeed(b3JointId)` | exported |
| `b3RevoluteJoint_GetMotorTorque` | `float b3RevoluteJoint_GetMotorTorque(b3JointId)` | exported |
| `b3RevoluteJoint_GetSpringDampingRatio` | `float b3RevoluteJoint_GetSpringDampingRatio(b3JointId)` | exported |
| `b3RevoluteJoint_GetSpringHertz` | `float b3RevoluteJoint_GetSpringHertz(b3JointId)` | exported |
| `b3RevoluteJoint_GetTargetAngle` | `float b3RevoluteJoint_GetTargetAngle(b3JointId)` | exported |
| `b3RevoluteJoint_GetUpperLimit` | `float b3RevoluteJoint_GetUpperLimit(b3JointId)` | exported |
| `b3RevoluteJoint_IsLimitEnabled` | `bool b3RevoluteJoint_IsLimitEnabled(b3JointId)` | exported |
| `b3RevoluteJoint_IsMotorEnabled` | `bool b3RevoluteJoint_IsMotorEnabled(b3JointId)` | exported |
| `b3RevoluteJoint_IsSpringEnabled` | `bool b3RevoluteJoint_IsSpringEnabled(b3JointId)` | exported |
| `b3RevoluteJoint_SetLimits` | `void b3RevoluteJoint_SetLimits(b3JointId, float, float)` | exported |
| `b3RevoluteJoint_SetMaxMotorTorque` | `void b3RevoluteJoint_SetMaxMotorTorque(b3JointId, float)` | exported |
| `b3RevoluteJoint_SetMotorSpeed` | `void b3RevoluteJoint_SetMotorSpeed(b3JointId, float)` | exported |
| `b3RevoluteJoint_SetSpringDampingRatio` | `void b3RevoluteJoint_SetSpringDampingRatio(b3JointId, float)` | exported |
| `b3RevoluteJoint_SetSpringHertz` | `void b3RevoluteJoint_SetSpringHertz(b3JointId, float)` | exported |
| `b3RevoluteJoint_SetTargetAngle` | `void b3RevoluteJoint_SetTargetAngle(b3JointId, float)` | exported |
| `b3RotateVector` | `b3Vec3 b3RotateVector(b3Quat, b3Vec3)` | inline |
| `b3RoundDownFloat` | `float b3RoundDownFloat(double)` | inline |
| `b3RoundUpFloat` | `float b3RoundUpFloat(double)` | inline |
| `b3SafeScale` | `b3Vec3 b3SafeScale(b3Vec3)` | inline |
| `b3SaveRecordingToFile` | `bool b3SaveRecordingToFile(const b3Recording *, const char *)` | exported |
| `b3ScaleBox` | `void b3ScaleBox(b3Vec3 *, b3Transform *, b3Vec3, float)` | exported |
| `b3SegmentDistance` | `b3SegmentDistanceResult b3SegmentDistance(b3Vec3, b3Vec3, b3Vec3, b3Vec3)` | exported |
| `b3SetAllocator` | `void b3SetAllocator(b3AllocFcn *, b3FreeFcn *)` | exported |
| `b3SetAssertFcn` | `void b3SetAssertFcn(b3AssertFcn *)` | exported |
| `b3SetLengthUnitsPerMeter` | `void b3SetLengthUnitsPerMeter(float)` | inline |
| `b3SetLogFcn` | `void b3SetLogFcn(b3LogFcn *)` | exported |
| `b3SetStallThreshold` | `void b3SetStallThreshold(float)` | inline |
| `b3ShapeCast` | `b3CastOutput b3ShapeCast(const b3ShapeCastPairInput *)` | exported |
| `b3ShapeCastCapsule` | `b3CastOutput b3ShapeCastCapsule(const b3Capsule *, const b3ShapeCastInput *)` | exported |
| `b3ShapeCastCompound` | `b3CastOutput b3ShapeCastCompound(const b3CompoundData *, const b3ShapeCastInput *)` | exported |
| `b3ShapeCastHeightField` | `b3CastOutput b3ShapeCastHeightField(const b3HeightFieldData *, const b3ShapeCastInput *)` | exported |
| `b3ShapeCastHull` | `b3CastOutput b3ShapeCastHull(const b3HullData *, const b3ShapeCastInput *)` | exported |
| `b3ShapeCastMesh` | `b3CastOutput b3ShapeCastMesh(const b3Mesh *, const b3ShapeCastInput *)` | exported |
| `b3ShapeCastSphere` | `b3CastOutput b3ShapeCastSphere(const b3Sphere *, const b3ShapeCastInput *)` | exported |
| `b3ShapeDistance` | `b3DistanceOutput b3ShapeDistance(const b3DistanceInput *, b3SimplexCache *, b3Simplex *, int)` | exported |
| `b3Shape_ApplyWind` | `void b3Shape_ApplyWind(b3ShapeId, b3Vec3, float, float, float, bool)` | exported |
| `b3Shape_AreContactEventsEnabled` | `bool b3Shape_AreContactEventsEnabled(b3ShapeId)` | exported |
| `b3Shape_AreHitEventsEnabled` | `bool b3Shape_AreHitEventsEnabled(b3ShapeId)` | exported |
| `b3Shape_ArePreSolveEventsEnabled` | `bool b3Shape_ArePreSolveEventsEnabled(b3ShapeId)` | exported |
| `b3Shape_AreSensorEventsEnabled` | `bool b3Shape_AreSensorEventsEnabled(b3ShapeId)` | exported |
| `b3Shape_ComputeMassData` | `b3MassData b3Shape_ComputeMassData(b3ShapeId)` | exported |
| `b3Shape_EnableContactEvents` | `void b3Shape_EnableContactEvents(b3ShapeId, bool)` | exported |
| `b3Shape_EnableHitEvents` | `void b3Shape_EnableHitEvents(b3ShapeId, bool)` | exported |
| `b3Shape_EnablePreSolveEvents` | `void b3Shape_EnablePreSolveEvents(b3ShapeId, bool)` | exported |
| `b3Shape_EnableSensorEvents` | `void b3Shape_EnableSensorEvents(b3ShapeId, bool)` | exported |
| `b3Shape_GetAABB` | `b3AABB b3Shape_GetAABB(b3ShapeId)` | exported |
| `b3Shape_GetBody` | `b3BodyId b3Shape_GetBody(b3ShapeId)` | exported |
| `b3Shape_GetCapsule` | `b3Capsule b3Shape_GetCapsule(b3ShapeId)` | exported |
| `b3Shape_GetClosestPoint` | `b3Vec3 b3Shape_GetClosestPoint(b3ShapeId, b3Vec3)` | exported |
| `b3Shape_GetContactCapacity` | `int b3Shape_GetContactCapacity(b3ShapeId)` | exported |
| `b3Shape_GetContactData` | `int b3Shape_GetContactData(b3ShapeId, b3ContactData *, int)` | exported |
| `b3Shape_GetDensity` | `float b3Shape_GetDensity(b3ShapeId)` | exported |
| `b3Shape_GetFilter` | `b3Filter b3Shape_GetFilter(b3ShapeId)` | exported |
| `b3Shape_GetFriction` | `float b3Shape_GetFriction(b3ShapeId)` | exported |
| `b3Shape_GetHeightField` | `const b3HeightFieldData * b3Shape_GetHeightField(b3ShapeId)` | exported |
| `b3Shape_GetHull` | `const b3HullData * b3Shape_GetHull(b3ShapeId)` | exported |
| `b3Shape_GetMesh` | `b3Mesh b3Shape_GetMesh(b3ShapeId)` | exported |
| `b3Shape_GetMeshMaterialCount` | `int b3Shape_GetMeshMaterialCount(b3ShapeId)` | exported |
| `b3Shape_GetMeshSurfaceMaterial` | `b3SurfaceMaterial b3Shape_GetMeshSurfaceMaterial(b3ShapeId, int)` | exported |
| `b3Shape_GetName` | `const char * b3Shape_GetName(b3ShapeId)` | exported |
| `b3Shape_GetRestitution` | `float b3Shape_GetRestitution(b3ShapeId)` | exported |
| `b3Shape_GetSensorCapacity` | `int b3Shape_GetSensorCapacity(b3ShapeId)` | exported |
| `b3Shape_GetSensorData` | `int b3Shape_GetSensorData(b3ShapeId, b3ShapeId *, int)` | exported |
| `b3Shape_GetSphere` | `b3Sphere b3Shape_GetSphere(b3ShapeId)` | exported |
| `b3Shape_GetSurfaceMaterial` | `b3SurfaceMaterial b3Shape_GetSurfaceMaterial(b3ShapeId)` | exported |
| `b3Shape_GetType` | `b3ShapeType b3Shape_GetType(b3ShapeId)` | exported |
| `b3Shape_GetUserData` | `void * b3Shape_GetUserData(b3ShapeId)` | exported |
| `b3Shape_GetWorld` | `b3WorldId b3Shape_GetWorld(b3ShapeId)` | exported |
| `b3Shape_IsSensor` | `bool b3Shape_IsSensor(b3ShapeId)` | exported |
| `b3Shape_IsValid` | `bool b3Shape_IsValid(b3ShapeId)` | exported |
| `b3Shape_RayCast` | `b3WorldCastOutput b3Shape_RayCast(b3ShapeId, b3Pos, b3Vec3)` | exported |
| `b3Shape_SetCapsule` | `void b3Shape_SetCapsule(b3ShapeId, const b3Capsule *)` | exported |
| `b3Shape_SetDensity` | `void b3Shape_SetDensity(b3ShapeId, float, bool)` | exported |
| `b3Shape_SetFilter` | `void b3Shape_SetFilter(b3ShapeId, b3Filter, bool)` | exported |
| `b3Shape_SetFriction` | `void b3Shape_SetFriction(b3ShapeId, float)` | exported |
| `b3Shape_SetHull` | `void b3Shape_SetHull(b3ShapeId, const b3HullData *)` | exported |
| `b3Shape_SetMesh` | `void b3Shape_SetMesh(b3ShapeId, const b3MeshData *, b3Vec3)` | exported |
| `b3Shape_SetMeshMaterial` | `void b3Shape_SetMeshMaterial(b3ShapeId, b3SurfaceMaterial, int)` | exported |
| `b3Shape_SetName` | `void b3Shape_SetName(b3ShapeId, const char *)` | exported |
| `b3Shape_SetRestitution` | `void b3Shape_SetRestitution(b3ShapeId, float)` | exported |
| `b3Shape_SetSphere` | `void b3Shape_SetSphere(b3ShapeId, const b3Sphere *)` | exported |
| `b3Shape_SetSurfaceMaterial` | `void b3Shape_SetSurfaceMaterial(b3ShapeId, b3SurfaceMaterial)` | exported |
| `b3Shape_SetUserData` | `void b3Shape_SetUserData(b3ShapeId, void *)` | exported |
| `b3Sign` | `b3Vec3 b3Sign(b3Vec3)` | inline |
| `b3Sin` | `float b3Sin(float)` | inline |
| `b3Sleep` | `void b3Sleep(int)` | exported |
| `b3Solve3` | `b3Vec3 b3Solve3(b3Matrix3, b3Vec3)` | inline |
| `b3SolvePlanes` | `b3PlaneSolverResult b3SolvePlanes(b3Vec3, b3CollisionPlane *, int)` | exported |
| `b3SphericalJoint_EnableConeLimit` | `void b3SphericalJoint_EnableConeLimit(b3JointId, bool)` | exported |
| `b3SphericalJoint_EnableMotor` | `void b3SphericalJoint_EnableMotor(b3JointId, bool)` | exported |
| `b3SphericalJoint_EnableSpring` | `void b3SphericalJoint_EnableSpring(b3JointId, bool)` | exported |
| `b3SphericalJoint_EnableTwistLimit` | `void b3SphericalJoint_EnableTwistLimit(b3JointId, bool)` | exported |
| `b3SphericalJoint_GetConeAngle` | `float b3SphericalJoint_GetConeAngle(b3JointId)` | exported |
| `b3SphericalJoint_GetConeLimit` | `float b3SphericalJoint_GetConeLimit(b3JointId)` | exported |
| `b3SphericalJoint_GetLowerTwistLimit` | `float b3SphericalJoint_GetLowerTwistLimit(b3JointId)` | exported |
| `b3SphericalJoint_GetMaxMotorTorque` | `float b3SphericalJoint_GetMaxMotorTorque(b3JointId)` | exported |
| `b3SphericalJoint_GetMotorTorque` | `b3Vec3 b3SphericalJoint_GetMotorTorque(b3JointId)` | exported |
| `b3SphericalJoint_GetMotorVelocity` | `b3Vec3 b3SphericalJoint_GetMotorVelocity(b3JointId)` | exported |
| `b3SphericalJoint_GetSpringDampingRatio` | `float b3SphericalJoint_GetSpringDampingRatio(b3JointId)` | exported |
| `b3SphericalJoint_GetSpringHertz` | `float b3SphericalJoint_GetSpringHertz(b3JointId)` | exported |
| `b3SphericalJoint_GetTargetRotation` | `b3Quat b3SphericalJoint_GetTargetRotation(b3JointId)` | exported |
| `b3SphericalJoint_GetTwistAngle` | `float b3SphericalJoint_GetTwistAngle(b3JointId)` | exported |
| `b3SphericalJoint_GetUpperTwistLimit` | `float b3SphericalJoint_GetUpperTwistLimit(b3JointId)` | exported |
| `b3SphericalJoint_IsConeLimitEnabled` | `bool b3SphericalJoint_IsConeLimitEnabled(b3JointId)` | exported |
| `b3SphericalJoint_IsMotorEnabled` | `bool b3SphericalJoint_IsMotorEnabled(b3JointId)` | exported |
| `b3SphericalJoint_IsSpringEnabled` | `bool b3SphericalJoint_IsSpringEnabled(b3JointId)` | exported |
| `b3SphericalJoint_IsTwistLimitEnabled` | `bool b3SphericalJoint_IsTwistLimitEnabled(b3JointId)` | exported |
| `b3SphericalJoint_SetConeLimit` | `void b3SphericalJoint_SetConeLimit(b3JointId, float)` | exported |
| `b3SphericalJoint_SetMaxMotorTorque` | `void b3SphericalJoint_SetMaxMotorTorque(b3JointId, float)` | exported |
| `b3SphericalJoint_SetMotorVelocity` | `void b3SphericalJoint_SetMotorVelocity(b3JointId, b3Vec3)` | exported |
| `b3SphericalJoint_SetSpringDampingRatio` | `void b3SphericalJoint_SetSpringDampingRatio(b3JointId, float)` | exported |
| `b3SphericalJoint_SetSpringHertz` | `void b3SphericalJoint_SetSpringHertz(b3JointId, float)` | exported |
| `b3SphericalJoint_SetTargetRotation` | `void b3SphericalJoint_SetTargetRotation(b3JointId, b3Quat)` | exported |
| `b3SphericalJoint_SetTwistLimits` | `void b3SphericalJoint_SetTwistLimits(b3JointId, float, float)` | exported |
| `b3Steiner` | `b3Matrix3 b3Steiner(float, b3Vec3)` | exported |
| `b3StoreBodyId` | `uint64_t b3StoreBodyId(b3BodyId)` | inline |
| `b3StoreContactId` | `void b3StoreContactId(b3ContactId, uint32_t *)` | inline |
| `b3StoreJointId` | `uint64_t b3StoreJointId(b3JointId)` | inline |
| `b3StoreShapeId` | `uint64_t b3StoreShapeId(b3ShapeId)` | inline |
| `b3StoreWorldId` | `uint32_t b3StoreWorldId(b3WorldId)` | inline |
| `b3Sub` | `b3Vec3 b3Sub(b3Vec3, b3Vec3)` | inline |
| `b3SubMM` | `b3Matrix3 b3SubMM(b3Matrix3, b3Matrix3)` | inline |
| `b3SubPos` | `b3Vec3 b3SubPos(b3Pos, b3Pos)` | inline |
| `b3TimeOfImpact` | `b3TOIOutput b3TimeOfImpact(const b3TOIInput *)` | exported |
| `b3ToPos` | `b3Pos b3ToPos(b3Vec3)` | inline |
| `b3ToRelativeTransform` | `b3Transform b3ToRelativeTransform(b3WorldTransform, b3Pos)` | inline |
| `b3ToVec3` | `b3Vec3 b3ToVec3(b3Pos)` | inline |
| `b3TransformPoint` | `b3Vec3 b3TransformPoint(b3Transform, b3Vec3)` | inline |
| `b3TransformWorldPoint` | `b3Pos b3TransformWorldPoint(b3WorldTransform, b3Vec3)` | inline |
| `b3Transpose` | `b3Matrix3 b3Transpose(b3Matrix3)` | inline |
| `b3UnwindAngle` | `float b3UnwindAngle(float)` | inline |
| `b3ValidateReplay` | `bool b3ValidateReplay(const void *, int, int)` | exported |
| `b3WeldJoint_GetAngularDampingRatio` | `float b3WeldJoint_GetAngularDampingRatio(b3JointId)` | exported |
| `b3WeldJoint_GetAngularHertz` | `float b3WeldJoint_GetAngularHertz(b3JointId)` | exported |
| `b3WeldJoint_GetLinearDampingRatio` | `float b3WeldJoint_GetLinearDampingRatio(b3JointId)` | exported |
| `b3WeldJoint_GetLinearHertz` | `float b3WeldJoint_GetLinearHertz(b3JointId)` | exported |
| `b3WeldJoint_SetAngularDampingRatio` | `void b3WeldJoint_SetAngularDampingRatio(b3JointId, float)` | exported |
| `b3WeldJoint_SetAngularHertz` | `void b3WeldJoint_SetAngularHertz(b3JointId, float)` | exported |
| `b3WeldJoint_SetLinearDampingRatio` | `void b3WeldJoint_SetLinearDampingRatio(b3JointId, float)` | exported |
| `b3WeldJoint_SetLinearHertz` | `void b3WeldJoint_SetLinearHertz(b3JointId, float)` | exported |
| `b3WheelJoint_EnableSpinMotor` | `void b3WheelJoint_EnableSpinMotor(b3JointId, bool)` | exported |
| `b3WheelJoint_EnableSteering` | `void b3WheelJoint_EnableSteering(b3JointId, bool)` | exported |
| `b3WheelJoint_EnableSteeringLimit` | `void b3WheelJoint_EnableSteeringLimit(b3JointId, bool)` | exported |
| `b3WheelJoint_EnableSuspension` | `void b3WheelJoint_EnableSuspension(b3JointId, bool)` | exported |
| `b3WheelJoint_EnableSuspensionLimit` | `void b3WheelJoint_EnableSuspensionLimit(b3JointId, bool)` | exported |
| `b3WheelJoint_GetLowerSteeringLimit` | `float b3WheelJoint_GetLowerSteeringLimit(b3JointId)` | exported |
| `b3WheelJoint_GetLowerSuspensionLimit` | `float b3WheelJoint_GetLowerSuspensionLimit(b3JointId)` | exported |
| `b3WheelJoint_GetMaxSpinTorque` | `float b3WheelJoint_GetMaxSpinTorque(b3JointId)` | exported |
| `b3WheelJoint_GetMaxSteeringTorque` | `float b3WheelJoint_GetMaxSteeringTorque(b3JointId)` | exported |
| `b3WheelJoint_GetSpinMotorSpeed` | `float b3WheelJoint_GetSpinMotorSpeed(b3JointId)` | exported |
| `b3WheelJoint_GetSpinSpeed` | `float b3WheelJoint_GetSpinSpeed(b3JointId)` | exported |
| `b3WheelJoint_GetSpinTorque` | `float b3WheelJoint_GetSpinTorque(b3JointId)` | exported |
| `b3WheelJoint_GetSteeringAngle` | `float b3WheelJoint_GetSteeringAngle(b3JointId)` | exported |
| `b3WheelJoint_GetSteeringDampingRatio` | `float b3WheelJoint_GetSteeringDampingRatio(b3JointId)` | exported |
| `b3WheelJoint_GetSteeringHertz` | `float b3WheelJoint_GetSteeringHertz(b3JointId)` | exported |
| `b3WheelJoint_GetSteeringTorque` | `float b3WheelJoint_GetSteeringTorque(b3JointId)` | exported |
| `b3WheelJoint_GetSuspensionDampingRatio` | `float b3WheelJoint_GetSuspensionDampingRatio(b3JointId)` | exported |
| `b3WheelJoint_GetSuspensionHertz` | `float b3WheelJoint_GetSuspensionHertz(b3JointId)` | exported |
| `b3WheelJoint_GetTargetSteeringAngle` | `float b3WheelJoint_GetTargetSteeringAngle(b3JointId)` | exported |
| `b3WheelJoint_GetUpperSteeringLimit` | `float b3WheelJoint_GetUpperSteeringLimit(b3JointId)` | exported |
| `b3WheelJoint_GetUpperSuspensionLimit` | `float b3WheelJoint_GetUpperSuspensionLimit(b3JointId)` | exported |
| `b3WheelJoint_IsSpinMotorEnabled` | `bool b3WheelJoint_IsSpinMotorEnabled(b3JointId)` | exported |
| `b3WheelJoint_IsSteeringEnabled` | `bool b3WheelJoint_IsSteeringEnabled(b3JointId)` | exported |
| `b3WheelJoint_IsSteeringLimitEnabled` | `bool b3WheelJoint_IsSteeringLimitEnabled(b3JointId)` | exported |
| `b3WheelJoint_IsSuspensionEnabled` | `bool b3WheelJoint_IsSuspensionEnabled(b3JointId)` | exported |
| `b3WheelJoint_IsSuspensionLimitEnabled` | `bool b3WheelJoint_IsSuspensionLimitEnabled(b3JointId)` | exported |
| `b3WheelJoint_SetMaxSpinTorque` | `void b3WheelJoint_SetMaxSpinTorque(b3JointId, float)` | exported |
| `b3WheelJoint_SetMaxSteeringTorque` | `void b3WheelJoint_SetMaxSteeringTorque(b3JointId, float)` | exported |
| `b3WheelJoint_SetSpinMotorSpeed` | `void b3WheelJoint_SetSpinMotorSpeed(b3JointId, float)` | exported |
| `b3WheelJoint_SetSteeringDampingRatio` | `void b3WheelJoint_SetSteeringDampingRatio(b3JointId, float)` | exported |
| `b3WheelJoint_SetSteeringHertz` | `void b3WheelJoint_SetSteeringHertz(b3JointId, float)` | exported |
| `b3WheelJoint_SetSteeringLimits` | `void b3WheelJoint_SetSteeringLimits(b3JointId, float, float)` | exported |
| `b3WheelJoint_SetSuspensionDampingRatio` | `void b3WheelJoint_SetSuspensionDampingRatio(b3JointId, float)` | exported |
| `b3WheelJoint_SetSuspensionHertz` | `void b3WheelJoint_SetSuspensionHertz(b3JointId, float)` | exported |
| `b3WheelJoint_SetSuspensionLimits` | `void b3WheelJoint_SetSuspensionLimits(b3JointId, float, float)` | exported |
| `b3WheelJoint_SetTargetSteeringAngle` | `void b3WheelJoint_SetTargetSteeringAngle(b3JointId, float)` | exported |
| `b3World_CastMover` | `float b3World_CastMover(b3WorldId, b3Pos, const b3Capsule *, b3Vec3, b3QueryFilter, b3MoverFilterFcn *, void *)` | exported |
| `b3World_CastRay` | `b3TreeStats b3World_CastRay(b3WorldId, b3Pos, b3Vec3, b3QueryFilter, b3CastResultFcn *, void *)` | exported |
| `b3World_CastRayClosest` | `b3RayResult b3World_CastRayClosest(b3WorldId, b3Pos, b3Vec3, b3QueryFilter)` | exported |
| `b3World_CastShape` | `b3TreeStats b3World_CastShape(b3WorldId, b3Pos, const b3ShapeProxy *, b3Vec3, b3QueryFilter, b3CastResultFcn *, void *)` | exported |
| `b3World_CollideMover` | `void b3World_CollideMover(b3WorldId, b3Pos, const b3Capsule *, b3QueryFilter, b3PlaneResultFcn *, void *)` | exported |
| `b3World_Draw` | `void b3World_Draw(b3WorldId, b3DebugDraw *, uint64_t)` | exported |
| `b3World_DumpMemoryStats` | `void b3World_DumpMemoryStats(b3WorldId)` | exported |
| `b3World_EnableContinuous` | `void b3World_EnableContinuous(b3WorldId, bool)` | exported |
| `b3World_EnableSleeping` | `void b3World_EnableSleeping(b3WorldId, bool)` | exported |
| `b3World_EnableSpeculative` | `void b3World_EnableSpeculative(b3WorldId, bool)` | exported |
| `b3World_EnableWarmStarting` | `void b3World_EnableWarmStarting(b3WorldId, bool)` | exported |
| `b3World_Explode` | `void b3World_Explode(b3WorldId, const b3ExplosionDef *)` | exported |
| `b3World_GetAwakeBodyCount` | `int b3World_GetAwakeBodyCount(b3WorldId)` | exported |
| `b3World_GetBodyEvents` | `b3BodyEvents b3World_GetBodyEvents(b3WorldId)` | exported |
| `b3World_GetBounds` | `b3AABB b3World_GetBounds(b3WorldId)` | exported |
| `b3World_GetContactEvents` | `b3ContactEvents b3World_GetContactEvents(b3WorldId)` | exported |
| `b3World_GetContactRecycleDistance` | `float b3World_GetContactRecycleDistance(b3WorldId)` | exported |
| `b3World_GetCounters` | `b3Counters b3World_GetCounters(b3WorldId)` | exported |
| `b3World_GetGravity` | `b3Vec3 b3World_GetGravity(b3WorldId)` | exported |
| `b3World_GetHitEventThreshold` | `float b3World_GetHitEventThreshold(b3WorldId)` | exported |
| `b3World_GetJointEvents` | `b3JointEvents b3World_GetJointEvents(b3WorldId)` | exported |
| `b3World_GetMaxCapacity` | `b3Capacity b3World_GetMaxCapacity(b3WorldId)` | exported |
| `b3World_GetMaximumLinearSpeed` | `float b3World_GetMaximumLinearSpeed(b3WorldId)` | exported |
| `b3World_GetProfile` | `b3Profile b3World_GetProfile(b3WorldId)` | exported |
| `b3World_GetRestitutionThreshold` | `float b3World_GetRestitutionThreshold(b3WorldId)` | exported |
| `b3World_GetSensorEvents` | `b3SensorEvents b3World_GetSensorEvents(b3WorldId)` | exported |
| `b3World_GetUserData` | `void * b3World_GetUserData(b3WorldId)` | exported |
| `b3World_GetWorkerCount` | `int b3World_GetWorkerCount(b3WorldId)` | exported |
| `b3World_IsContinuousEnabled` | `bool b3World_IsContinuousEnabled(b3WorldId)` | exported |
| `b3World_IsSleepingEnabled` | `bool b3World_IsSleepingEnabled(b3WorldId)` | exported |
| `b3World_IsValid` | `bool b3World_IsValid(b3WorldId)` | exported |
| `b3World_IsWarmStartingEnabled` | `bool b3World_IsWarmStartingEnabled(b3WorldId)` | exported |
| `b3World_OverlapAABB` | `b3TreeStats b3World_OverlapAABB(b3WorldId, b3AABB, b3QueryFilter, b3OverlapResultFcn *, void *)` | exported |
| `b3World_OverlapShape` | `b3TreeStats b3World_OverlapShape(b3WorldId, b3Pos, const b3ShapeProxy *, b3QueryFilter, b3OverlapResultFcn *, void *)` | exported |
| `b3World_RebuildStaticTree` | `void b3World_RebuildStaticTree(b3WorldId)` | exported |
| `b3World_SetContactRecycleDistance` | `void b3World_SetContactRecycleDistance(b3WorldId, float)` | exported |
| `b3World_SetContactTuning` | `void b3World_SetContactTuning(b3WorldId, float, float, float)` | exported |
| `b3World_SetCustomFilterCallback` | `void b3World_SetCustomFilterCallback(b3WorldId, b3CustomFilterFcn *, void *)` | exported |
| `b3World_SetFrictionCallback` | `void b3World_SetFrictionCallback(b3WorldId, b3FrictionCallback *)` | exported |
| `b3World_SetGravity` | `void b3World_SetGravity(b3WorldId, b3Vec3)` | exported |
| `b3World_SetHitEventThreshold` | `void b3World_SetHitEventThreshold(b3WorldId, float)` | exported |
| `b3World_SetMaximumLinearSpeed` | `void b3World_SetMaximumLinearSpeed(b3WorldId, float)` | exported |
| `b3World_SetPreSolveCallback` | `void b3World_SetPreSolveCallback(b3WorldId, b3PreSolveFcn *, void *)` | exported |
| `b3World_SetRestitutionCallback` | `void b3World_SetRestitutionCallback(b3WorldId, b3RestitutionCallback *)` | exported |
| `b3World_SetRestitutionThreshold` | `void b3World_SetRestitutionThreshold(b3WorldId, float)` | exported |
| `b3World_SetUserData` | `void b3World_SetUserData(b3WorldId, void *)` | exported |
| `b3World_SetWorkerCount` | `void b3World_SetWorkerCount(b3WorldId, int)` | exported |
| `b3World_StartRecording` | `void b3World_StartRecording(b3WorldId, b3Recording *)` | exported |
| `b3World_Step` | `void b3World_Step(b3WorldId, float, int)` | exported |
| `b3World_StopRecording` | `void b3World_StopRecording(b3WorldId)` | exported |
| `b3Yield` | `void b3Yield()` | exported |

## Structs

| Struct | Dictionary fields |
| --- | --- |
| `b3AABB` | `lowerBound: b3Vec3`, `upperBound: b3Vec3` |
| `b3BodyCastResult` | `shapeId: b3ShapeId`, `point: b3Pos`, `normal: b3Vec3`, `fraction: float`, `triangleIndex: int`, `userMaterialId: uint64_t`, `iterations: int`, `hit: bool` |
| `b3BodyDef` | `type: b3BodyType`, `position: b3Pos`, `rotation: b3Quat`, `linearVelocity: b3Vec3`, `angularVelocity: b3Vec3`, `linearDamping: float`, `angularDamping: float`, `gravityScale: float`, `sleepThreshold: float`, `name: const char *`, `userData: void *`, `motionLocks: b3MotionLocks`, `enableSleep: bool`, `isAwake: bool`, `isBullet: bool`, `isEnabled: bool`, `allowFastRotation: bool`, `enableContactRecycling: bool`, `internalValue: int` |
| `b3BodyEvents` | `moveEvents: b3BodyMoveEvent *`, `moveCount: int` |
| `b3BodyId` | `index1: int32_t`, `world0: uint16_t`, `generation: uint16_t` |
| `b3BodyMoveEvent` | `userData: void *`, `transform: b3WorldTransform`, `bodyId: b3BodyId`, `fellAsleep: bool` |
| `b3BodyPlaneResult` | `shapeId: b3ShapeId`, `result: b3PlaneResult` |
| `b3BoxCastInput` | `box: b3AABB`, `translation: b3Vec3`, `maxFraction: float` |
| `b3BoxHull` | `base: b3HullData`, `boxVertices: b3HullVertex[8]`, `boxPoints: b3Vec3[8]`, `boxEdges: b3HullHalfEdge[24]`, `boxFaces: b3HullFace[6]`, `padding: uint8_t[2]`, `boxPlanes: b3Plane[6]` |
| `b3Capacity` | `staticShapeCount: int`, `dynamicShapeCount: int`, `staticBodyCount: int`, `dynamicBodyCount: int`, `contactCount: int` |
| `b3Capsule` | `center1: b3Vec3`, `center2: b3Vec3`, `radius: float` |
| `b3CastOutput` | `normal: b3Vec3`, `point: b3Vec3`, `fraction: float`, `iterations: int`, `triangleIndex: int`, `childIndex: int`, `materialIndex: int`, `hit: bool` |
| `b3ChildShape` | Exact packed bytes only (contains anonymous union fields). |
| `b3CollisionPlane` | `plane: b3Plane`, `pushLimit: float`, `push: float`, `clipVelocity: bool` |
| `b3CompoundCapsule` | `capsule: b3Capsule`, `materialIndex: int` |
| `b3CompoundCapsuleDef` | `capsule: b3Capsule`, `material: b3SurfaceMaterial` |
| `b3CompoundData` | `version: uint64_t`, `byteCount: int`, `nodeOffset: int`, `tree: b3DynamicTree`, `materialOffset: int`, `materialCount: int`, `capsuleOffset: int`, `capsuleCount: int`, `hullOffset: int`, `hullCount: int`, `sharedHullCount: int`, `meshOffset: int`, `meshCount: int`, `sharedMeshCount: int`, `sphereOffset: int`, `sphereCount: int` |
| `b3CompoundDef` | `capsules: b3CompoundCapsuleDef *`, `capsuleCount: int`, `hulls: b3CompoundHullDef *`, `hullCount: int`, `meshes: b3CompoundMeshDef *`, `meshCount: int`, `spheres: b3CompoundSphereDef *`, `sphereCount: int` |
| `b3CompoundHull` | `hull: const b3HullData *`, `transform: b3Transform`, `materialIndex: int` |
| `b3CompoundHullDef` | `hull: const b3HullData *`, `transform: b3Transform`, `material: b3SurfaceMaterial` |
| `b3CompoundMesh` | `meshData: const b3MeshData *`, `transform: b3Transform`, `scale: b3Vec3`, `materialIndices: int[4]` |
| `b3CompoundMeshDef` | `meshData: const b3MeshData *`, `transform: b3Transform`, `scale: b3Vec3`, `materials: const b3SurfaceMaterial *`, `materialCount: int` |
| `b3CompoundSphere` | `sphere: b3Sphere`, `materialIndex: int` |
| `b3CompoundSphereDef` | `sphere: b3Sphere`, `material: b3SurfaceMaterial` |
| `b3ContactBeginTouchEvent` | `shapeIdA: b3ShapeId`, `shapeIdB: b3ShapeId`, `contactId: b3ContactId` |
| `b3ContactData` | `contactId: b3ContactId`, `shapeIdA: b3ShapeId`, `shapeIdB: b3ShapeId`, `manifolds: const struct b3Manifold *`, `manifoldCount: int` |
| `b3ContactEndTouchEvent` | `shapeIdA: b3ShapeId`, `shapeIdB: b3ShapeId`, `contactId: b3ContactId` |
| `b3ContactEvents` | `beginEvents: b3ContactBeginTouchEvent *`, `endEvents: b3ContactEndTouchEvent *`, `hitEvents: b3ContactHitEvent *`, `beginCount: int`, `endCount: int`, `hitCount: int` |
| `b3ContactHitEvent` | `shapeIdA: b3ShapeId`, `shapeIdB: b3ShapeId`, `contactId: b3ContactId`, `point: b3Pos`, `normal: b3Vec3`, `approachSpeed: float`, `userMaterialIdA: uint64_t`, `userMaterialIdB: uint64_t` |
| `b3ContactId` | `index1: int32_t`, `world0: uint16_t`, `padding: int16_t`, `generation: uint32_t` |
| `b3CosSin` | `cosine: float`, `sine: float` |
| `b3Counters` | `bodyCount: int`, `shapeCount: int`, `contactCount: int`, `jointCount: int`, `islandCount: int`, `stackUsed: int`, `arenaCapacity: int`, `staticTreeHeight: int`, `treeHeight: int`, `satCallCount: int`, `satCacheHitCount: int`, `byteCount: int`, `taskCount: int`, `colorCounts: int[24]`, `manifoldCounts: int[8]`, `awakeContactCount: int`, `recycledContactCount: int`, `distanceIterations: int`, `pushBackIterations: int`, `rootIterations: int` |
| `b3DebugDraw` | `DrawShapeFcn: bool (*)(void *, b3WorldTransform, b3HexColor, void *)`, `DrawSegmentFcn: void (*)(b3Pos, b3Pos, b3HexColor, void *)`, `DrawTransformFcn: void (*)(b3WorldTransform, void *)`, `DrawPointFcn: void (*)(b3Pos, float, b3HexColor, void *)`, `DrawSphereFcn: void (*)(b3Pos, float, b3HexColor, float, void *)`, `DrawCapsuleFcn: void (*)(b3Pos, b3Pos, float, b3HexColor, float, void *)`, `DrawBoundsFcn: void (*)(b3AABB, b3HexColor, void *)`, `DrawBoxFcn: void (*)(b3Vec3, b3WorldTransform, b3HexColor, void *)`, `DrawStringFcn: void (*)(b3Pos, const char *, b3HexColor, void *)`, `drawingBounds: b3AABB`, `forceScale: float`, `jointScale: float`, `drawShapes: bool`, `drawJoints: bool`, `drawJointExtras: bool`, `drawBounds: bool`, `drawMass: bool`, `drawSleep: bool`, `drawBodyNames: bool`, `drawContacts: bool`, `drawAnchorA: int`, `drawGraphColors: bool`, `drawContactFeatures: bool`, `drawContactNormals: bool`, `drawContactForces: bool`, `drawIslands: bool`, `context: void *` |
| `b3DebugShape` | Exact packed bytes only (contains anonymous union fields). |
| `b3DistanceInput` | `proxyA: b3ShapeProxy`, `proxyB: b3ShapeProxy`, `transform: b3Transform`, `useRadii: bool` |
| `b3DistanceJointDef` | `base: b3JointDef`, `length: float`, `enableSpring: bool`, `lowerSpringForce: float`, `upperSpringForce: float`, `hertz: float`, `dampingRatio: float`, `enableLimit: bool`, `minLength: float`, `maxLength: float`, `enableMotor: bool`, `maxMotorForce: float`, `motorSpeed: float` |
| `b3DistanceOutput` | `pointA: b3Vec3`, `pointB: b3Vec3`, `normal: b3Vec3`, `distance: float`, `iterations: int`, `simplexCount: int` |
| `b3DynamicTree` | `version: uint64_t`, `nodes: b3TreeNode *`, `root: int`, `nodeCount: int`, `nodeCapacity: int`, `proxyCount: int`, `freeList: int`, `leafIndices: int *`, `leafBoxes: b3AABB *`, `leafCenters: b3Vec3 *`, `binIndices: int *`, `rebuildCapacity: int` |
| `b3ExplosionDef` | `maskBits: uint64_t`, `position: b3Pos`, `radius: float`, `falloff: float`, `impulsePerArea: float` |
| `b3FeaturePair` | `owner1: uint8_t`, `index1: uint8_t`, `owner2: uint8_t`, `index2: uint8_t` |
| `b3Filter` | `categoryBits: uint64_t`, `maskBits: uint64_t`, `groupIndex: int` |
| `b3FilterJointDef` | `base: b3JointDef` |
| `b3HeightFieldData` | `version: uint64_t`, `byteCount: int`, `hash: uint32_t`, `aabb: b3AABB`, `minHeight: float`, `maxHeight: float`, `heightScale: float`, `scale: b3Vec3`, `columnCount: int`, `rowCount: int`, `heightsOffset: int`, `materialOffset: int`, `flagsOffset: int`, `clockwise: bool`, `padding: uint8_t[3]` |
| `b3HeightFieldDef` | `heights: float *`, `materialIndices: uint8_t *`, `scale: b3Vec3`, `countX: int`, `countZ: int`, `globalMinimumHeight: float`, `globalMaximumHeight: float`, `clockwiseWinding: bool` |
| `b3HullData` | `version: uint64_t`, `byteCount: int`, `hash: uint32_t`, `aabb: b3AABB`, `surfaceArea: float`, `volume: float`, `innerRadius: float`, `center: b3Vec3`, `centralInertia: b3Matrix3`, `vertexCount: int`, `vertexOffset: int`, `pointOffset: int`, `edgeCount: int`, `edgeOffset: int`, `faceCount: int`, `faceOffset: int`, `planeOffset: int`, `padding: int` |
| `b3HullFace` | `edge: uint8_t` |
| `b3HullHalfEdge` | `next: uint8_t`, `twin: uint8_t`, `origin: uint8_t`, `face: uint8_t` |
| `b3HullVertex` | `edge: uint8_t` |
| `b3JointDef` | `userData: void *`, `bodyIdA: b3BodyId`, `bodyIdB: b3BodyId`, `localFrameA: b3Transform`, `localFrameB: b3Transform`, `forceThreshold: float`, `torqueThreshold: float`, `constraintHertz: float`, `constraintDampingRatio: float`, `drawScale: float`, `collideConnected: bool`, `internalValue: int` |
| `b3JointEvent` | `jointId: b3JointId`, `userData: void *` |
| `b3JointEvents` | `jointEvents: b3JointEvent *`, `count: int` |
| `b3JointId` | `index1: int32_t`, `world0: uint16_t`, `generation: uint16_t` |
| `b3LocalManifold` | `normal: b3Vec3`, `triangleNormal: b3Vec3`, `points: b3LocalManifoldPoint *`, `pointCount: int`, `triangleIndex: int`, `i1: int`, `i2: int`, `i3: int`, `squaredDistance: float`, `feature: b3TriangleFeature`, `triangleFlags: int` |
| `b3LocalManifoldPoint` | `point: b3Vec3`, `separation: float`, `pair: b3FeaturePair`, `triangleIndex: int` |
| `b3Manifold` | `points: b3ManifoldPoint[4]`, `normal: b3Vec3`, `twistImpulse: float`, `frictionImpulse: b3Vec3`, `rollingImpulse: b3Vec3`, `pointCount: int` |
| `b3ManifoldPoint` | `anchorA: b3Vec3`, `anchorB: b3Vec3`, `separation: float`, `baseSeparation: float`, `normalImpulse: float`, `totalNormalImpulse: float`, `normalVelocity: float`, `featureId: uint32_t`, `triangleIndex: int`, `persisted: bool` |
| `b3MassData` | `mass: float`, `center: b3Vec3`, `inertia: b3Matrix3` |
| `b3Matrix3` | `cx: b3Vec3`, `cy: b3Vec3`, `cz: b3Vec3` |
| `b3Mesh` | `data: const b3MeshData *`, `scale: b3Vec3` |
| `b3MeshData` | `version: uint64_t`, `byteCount: int`, `hash: uint32_t`, `bounds: b3AABB`, `surfaceArea: float`, `treeHeight: int`, `degenerateCount: int`, `nodeOffset: int`, `nodeCount: int`, `vertexOffset: int`, `vertexCount: int`, `triangleOffset: int`, `triangleCount: int`, `materialOffset: int`, `materialCount: int`, `flagsOffset: int` |
| `b3MeshDef` | `vertices: b3Vec3 *`, `indices: int32_t *`, `materialIndices: uint8_t *`, `weldTolerance: float`, `vertexCount: int`, `triangleCount: int`, `weldVertices: bool`, `useMedianSplit: bool`, `identifyEdges: bool` |
| `b3MeshNode` | Exact packed bytes only (contains anonymous union fields). |
| `b3MeshTriangle` | `index1: int32_t`, `index2: int32_t`, `index3: int32_t` |
| `b3MotionLocks` | `linearX: bool`, `linearY: bool`, `linearZ: bool`, `angularX: bool`, `angularY: bool`, `angularZ: bool` |
| `b3MotorJointDef` | `base: b3JointDef`, `linearVelocity: b3Vec3`, `maxVelocityForce: float`, `angularVelocity: b3Vec3`, `maxVelocityTorque: float`, `linearHertz: float`, `linearDampingRatio: float`, `maxSpringForce: float`, `angularHertz: float`, `angularDampingRatio: float`, `maxSpringTorque: float` |
| `b3ParallelJointDef` | `base: b3JointDef`, `hertz: float`, `dampingRatio: float`, `maxTorque: float` |
| `b3Plane` | `normal: b3Vec3`, `offset: float` |
| `b3PlaneResult` | `plane: b3Plane`, `point: b3Vec3` |
| `b3PlaneSolverResult` | `delta: b3Vec3`, `iterationCount: int` |
| `b3PrismaticJointDef` | `base: b3JointDef`, `enableSpring: bool`, `hertz: float`, `dampingRatio: float`, `targetTranslation: float`, `enableLimit: bool`, `lowerTranslation: float`, `upperTranslation: float`, `enableMotor: bool`, `maxMotorForce: float`, `motorSpeed: float` |
| `b3Profile` | `step: float`, `pairs: float`, `collide: float`, `solve: float`, `solverSetup: float`, `constraints: float`, `prepareConstraints: float`, `integrateVelocities: float`, `warmStart: float`, `solveImpulses: float`, `integratePositions: float`, `relaxImpulses: float`, `applyRestitution: float`, `storeImpulses: float`, `splitIslands: float`, `transforms: float`, `sensorHits: float`, `jointEvents: float`, `hitEvents: float`, `refit: float`, `bullets: float`, `sleepIslands: float`, `sensors: float` |
| `b3Quat` | Exact packed bytes only (contains anonymous union fields). |
| `b3QueryFilter` | `categoryBits: uint64_t`, `maskBits: uint64_t`, `id: uint64_t`, `name: const char *` |
| `b3RayCastInput` | `origin: b3Vec3`, `translation: b3Vec3`, `maxFraction: float` |
| `b3RayResult` | `shapeId: b3ShapeId`, `point: b3Pos`, `normal: b3Vec3`, `userMaterialId: uint64_t`, `fraction: float`, `triangleIndex: int`, `childIndex: int`, `nodeVisits: int`, `leafVisits: int`, `hit: bool` |
| `b3RecPlayerInfo` | `frameCount: int`, `workerCount: int`, `timeStep: float`, `subStepCount: int`, `lengthScale: float`, `bounds: b3AABB` |
| `b3RecQueryHit` | `shape: b3ShapeId`, `point: b3Pos`, `normal: b3Vec3`, `fraction: float` |
| `b3RecQueryInfo` | `type: b3RecQueryType`, `filter: b3QueryFilter`, `aabb: b3AABB`, `origin: b3Pos`, `translation: b3Vec3`, `hitCount: int`, `key: uint64_t`, `id: uint64_t`, `name: const char *` |
| `b3RevoluteJointDef` | `base: b3JointDef`, `targetAngle: float`, `enableSpring: bool`, `hertz: float`, `dampingRatio: float`, `enableLimit: bool`, `lowerAngle: float`, `upperAngle: float`, `enableMotor: bool`, `maxMotorTorque: float`, `motorSpeed: float` |
| `b3SegmentDistanceResult` | `point1: b3Vec3`, `fraction1: float`, `point2: b3Vec3`, `fraction2: float` |
| `b3SensorBeginTouchEvent` | `sensorShapeId: b3ShapeId`, `visitorShapeId: b3ShapeId` |
| `b3SensorEndTouchEvent` | `sensorShapeId: b3ShapeId`, `visitorShapeId: b3ShapeId` |
| `b3SensorEvents` | `beginEvents: b3SensorBeginTouchEvent *`, `endEvents: b3SensorEndTouchEvent *`, `beginCount: int`, `endCount: int` |
| `b3ShapeCastInput` | `proxy: b3ShapeProxy`, `translation: b3Vec3`, `maxFraction: float`, `canEncroach: bool` |
| `b3ShapeCastPairInput` | `proxyA: b3ShapeProxy`, `proxyB: b3ShapeProxy`, `transform: b3Transform`, `translationB: b3Vec3`, `maxFraction: float`, `canEncroach: bool` |
| `b3ShapeDef` | `name: const char *`, `userData: void *`, `materials: b3SurfaceMaterial *`, `materialCount: int`, `baseMaterial: b3SurfaceMaterial`, `density: float`, `explosionScale: float`, `filter: b3Filter`, `enableCustomFiltering: bool`, `isSensor: bool`, `enableSensorEvents: bool`, `enableContactEvents: bool`, `enableHitEvents: bool`, `enablePreSolveEvents: bool`, `invokeContactCreation: bool`, `updateBodyMass: bool`, `enableSpeculativeContact: bool`, `internalValue: int` |
| `b3ShapeId` | `index1: int32_t`, `world0: uint16_t`, `generation: uint16_t` |
| `b3ShapeProxy` | `points: const b3Vec3 *`, `count: int`, `radius: float` |
| `b3Simplex` | `vertices: b3SimplexVertex[4]`, `count: int` |
| `b3SimplexCache` | `metric: float`, `count: uint16_t`, `indexA: uint8_t[4]`, `indexB: uint8_t[4]` |
| `b3SimplexVertex` | `wA: b3Vec3`, `wB: b3Vec3`, `w: b3Vec3`, `a: float`, `indexA: int`, `indexB: int` |
| `b3Sphere` | `center: b3Vec3`, `radius: float` |
| `b3SphericalJointDef` | `base: b3JointDef`, `enableSpring: bool`, `hertz: float`, `dampingRatio: float`, `targetRotation: b3Quat`, `enableConeLimit: bool`, `coneAngle: float`, `enableTwistLimit: bool`, `lowerTwistAngle: float`, `upperTwistAngle: float`, `enableMotor: bool`, `maxMotorTorque: float`, `motorVelocity: b3Vec3` |
| `b3SurfaceMaterial` | `friction: float`, `restitution: float`, `rollingResistance: float`, `tangentVelocity: b3Vec3`, `userMaterialId: uint64_t`, `customColor: uint32_t` |
| `b3Sweep` | `localCenter: b3Vec3`, `c1: b3Vec3`, `c2: b3Vec3`, `q1: b3Quat`, `q2: b3Quat` |
| `b3TOIInput` | `proxyA: b3ShapeProxy`, `proxyB: b3ShapeProxy`, `sweepA: b3Sweep`, `sweepB: b3Sweep`, `maxFraction: float` |
| `b3TOIOutput` | `state: b3TOIState`, `point: b3Vec3`, `normal: b3Vec3`, `fraction: float`, `distance: float`, `distanceIterations: int`, `pushBackIterations: int`, `rootIterations: int`, `usedFallback: bool` |
| `b3Transform` | Exact packed bytes only (contains anonymous union fields). |
| `b3TreeNode` | Exact packed bytes only (contains anonymous union fields). |
| `b3TreeNodeChildren` | `child1: int`, `child2: int` |
| `b3TreeStats` | `nodeVisits: int`, `leafVisits: int` |
| `b3Vec2` | `x: float`, `y: float` |
| `b3Vec3` | Exact packed bytes only (contains anonymous union fields). |
| `b3Version` | `major: int`, `minor: int`, `revision: int` |
| `b3WeldJointDef` | `base: b3JointDef`, `linearHertz: float`, `angularHertz: float`, `linearDampingRatio: float`, `angularDampingRatio: float` |
| `b3WheelJointDef` | `base: b3JointDef`, `enableSuspensionSpring: bool`, `suspensionHertz: float`, `suspensionDampingRatio: float`, `enableSuspensionLimit: bool`, `lowerSuspensionLimit: float`, `upperSuspensionLimit: float`, `enableSpinMotor: bool`, `maxSpinTorque: float`, `spinSpeed: float`, `enableSteering: bool`, `steeringHertz: float`, `steeringDampingRatio: float`, `targetSteeringAngle: float`, `maxSteeringTorque: float`, `enableSteeringLimit: bool`, `lowerSteeringLimit: float`, `upperSteeringLimit: float` |
| `b3WorldDef` | `gravity: b3Vec3`, `restitutionThreshold: float`, `hitEventThreshold: float`, `contactHertz: float`, `contactDampingRatio: float`, `contactSpeed: float`, `maximumLinearSpeed: float`, `frictionCallback: b3FrictionCallback *`, `restitutionCallback: b3RestitutionCallback *`, `enableSleep: bool`, `enableContinuous: bool`, `workerCount: uint32_t`, `enqueueTask: b3EnqueueTaskCallback *`, `finishTask: b3FinishTaskCallback *`, `userTaskContext: void *`, `userData: void *`, `createDebugShape: b3CreateDebugShapeCallback *`, `destroyDebugShape: b3DestroyDebugShapeCallback *`, `userDebugShapeContext: void *`, `capacity: b3Capacity`, `internalValue: int` |
| `b3WorldId` | `index1: uint16_t`, `generation: uint16_t` |
