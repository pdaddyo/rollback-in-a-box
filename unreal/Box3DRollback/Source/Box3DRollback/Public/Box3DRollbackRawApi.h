// Copyright rollback-in-a-box. SPDX-License-Identifier: same as repository LICENSE.
//
// Engine-neutral access to the Box3D instance owned by the Unreal plugin.
//
// On Windows, raw consumers use __declspec(dllimport). Other targets do not
// have an equivalent that reliably prevents a static archive from satisfying
// a symbol locally, so consumers use this table to avoid creating a second
// copy of Box3D's process-global state.
#pragma once

#include "box3d/box3d.h"

#include <cstdint>

struct FBox3DRollbackRawApi
{
	uint32_t Version = 1;
	uint32_t Size = sizeof(FBox3DRollbackRawApi);

	decltype(&b3Atan2) Atan2 = &b3Atan2;
	decltype(&b3Body_ApplyForceToCenter) BodyApplyForceToCenter = &b3Body_ApplyForceToCenter;
	decltype(&b3Body_EnableSleep) BodyEnableSleep = &b3Body_EnableSleep;
	decltype(&b3Body_GetAngularVelocity) BodyGetAngularVelocity = &b3Body_GetAngularVelocity;
	decltype(&b3Body_GetLinearVelocity) BodyGetLinearVelocity = &b3Body_GetLinearVelocity;
	decltype(&b3Body_GetPosition) BodyGetPosition = &b3Body_GetPosition;
	decltype(&b3Body_GetRotation) BodyGetRotation = &b3Body_GetRotation;
	decltype(&b3Body_GetShapes) BodyGetShapes = &b3Body_GetShapes;
	decltype(&b3Body_GetWorldPoint) BodyGetWorldPoint = &b3Body_GetWorldPoint;
	decltype(&b3Body_SetBullet) BodySetBullet = &b3Body_SetBullet;
	decltype(&b3Body_SetGravityScale) BodySetGravityScale = &b3Body_SetGravityScale;
	decltype(&b3Body_SetLinearVelocity) BodySetLinearVelocity = &b3Body_SetLinearVelocity;
	decltype(&b3Body_SetMotionLocks) BodySetMotionLocks = &b3Body_SetMotionLocks;
	decltype(&b3Body_SetTransform) BodySetTransform = &b3Body_SetTransform;
	decltype(&b3ComputeCosSin) ComputeCosSin = &b3ComputeCosSin;
	decltype(&b3CreateDistanceJoint) CreateDistanceJoint = &b3CreateDistanceJoint;
	decltype(&b3DefaultDistanceJointDef) DefaultDistanceJointDef = &b3DefaultDistanceJointDef;
	decltype(&b3DefaultFilter) DefaultFilter = &b3DefaultFilter;
	decltype(&b3DistanceJoint_EnableLimit) DistanceJointEnableLimit = &b3DistanceJoint_EnableLimit;
	decltype(&b3DistanceJoint_EnableMotor) DistanceJointEnableMotor = &b3DistanceJoint_EnableMotor;
	decltype(&b3DistanceJoint_GetCurrentLength) DistanceJointGetCurrentLength = &b3DistanceJoint_GetCurrentLength;
	decltype(&b3DistanceJoint_GetSpringHertz) DistanceJointGetSpringHertz = &b3DistanceJoint_GetSpringHertz;
	decltype(&b3DistanceJoint_SetLength) DistanceJointSetLength = &b3DistanceJoint_SetLength;
	decltype(&b3DistanceJoint_SetLengthRange) DistanceJointSetLengthRange = &b3DistanceJoint_SetLengthRange;
	decltype(&b3DistanceJoint_SetMaxMotorForce) DistanceJointSetMaxMotorForce = &b3DistanceJoint_SetMaxMotorForce;
	decltype(&b3DistanceJoint_SetMotorSpeed) DistanceJointSetMotorSpeed = &b3DistanceJoint_SetMotorSpeed;
	decltype(&b3DistanceJoint_SetSpringForceRange) DistanceJointSetSpringForceRange = &b3DistanceJoint_SetSpringForceRange;
	decltype(&b3DistanceJoint_SetSpringHertz) DistanceJointSetSpringHertz = &b3DistanceJoint_SetSpringHertz;
	decltype(&b3Joint_GetBodyA) JointGetBodyA = &b3Joint_GetBodyA;
	decltype(&b3Joint_GetConstraintForce) JointGetConstraintForce = &b3Joint_GetConstraintForce;
	decltype(&b3Joint_GetLocalFrameA) JointGetLocalFrameA = &b3Joint_GetLocalFrameA;
	decltype(&b3Joint_GetLocalFrameB) JointGetLocalFrameB = &b3Joint_GetLocalFrameB;
	decltype(&b3SetLengthUnitsPerMeter) SetLengthUnitsPerMeter = &b3SetLengthUnitsPerMeter;
	decltype(&b3Shape_SetFilter) ShapeSetFilter = &b3Shape_SetFilter;
	decltype(&b3World_SetGravity) WorldSetGravity = &b3World_SetGravity;
	decltype(&b3World_SetMaximumLinearSpeed) WorldSetMaximumLinearSpeed = &b3World_SetMaximumLinearSpeed;
};

extern "C" BOX3DROLLBACK_API const FBox3DRollbackRawApi* Box3DRollback_GetRawApiV1();
