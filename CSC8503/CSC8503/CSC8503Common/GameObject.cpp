#include "GameObject.h"
#include "CollisionDetection.h"
//#include "../GameTech/TutorialGame.h"

using namespace NCL::CSC8503;


GameObject::GameObject(string objectName) {
	name = objectName;
	worldID = -1;
	cRest = 1;
	isActive = true;
	boundingVolume = nullptr;
	physicsObject = nullptr;
	renderObject = nullptr;
	//game = g;
	//game = nullptr;
}

/*
GameObject::GameObject(TutorialGame* g, string objectName)	{
	name			= objectName;
	worldID			= -1;
	cRest			= 1;
	isActive		= true;
	boundingVolume	= nullptr;
	physicsObject	= nullptr;
	renderObject	= nullptr;
	game = g;
}
*/
GameObject::~GameObject()	{
	delete boundingVolume;
	delete physicsObject;
	delete renderObject;
}

bool GameObject::GetBroadphaseAABB(Vector3&outSize) const {
	if (!boundingVolume) {
		return false;
	}
	outSize = broadphaseAABB;
	return true;
}

void GameObject::UpdateBroadphaseAABB() {
	if (!boundingVolume) {
		return;
	}
	if (boundingVolume->type == VolumeType::AABB) {
		broadphaseAABB = ((AABBVolume&)*boundingVolume).GetHalfDimensions();
	}
	else if (boundingVolume->type == VolumeType::Sphere) {
		float r = ((SphereVolume&)*boundingVolume).GetRadius();
		broadphaseAABB = Vector3(r, r, r);
	}
	else if (boundingVolume->type == VolumeType::OBB) {
		Matrix3 mat = Matrix3(transform.GetOrientation());
		mat = mat.Absolute();
		Vector3 halfSizes = ((OBBVolume&)*boundingVolume).GetHalfDimensions();
		broadphaseAABB = mat * halfSizes;
	}
	else if (boundingVolume->type == VolumeType::Capsule) {
		Matrix3 mat = Matrix3(transform.GetOrientation());
		mat = mat.Absolute();
		Vector3 halfSizes = Vector3( ((CapsuleVolume&)*boundingVolume).GetRadius(), ((CapsuleVolume&)*boundingVolume).GetHalfHeight(), ((CapsuleVolume&)*boundingVolume).GetRadius());
		broadphaseAABB = mat * halfSizes;
		
	}
}