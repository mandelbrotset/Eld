#pragma once
#include <algorithm>

#include <OBJModel.h>
#include <glutil.h>
#include <float4x4.h>
#include <float3x3.h>

using namespace chag;
using namespace std;
class Node
{
public:
	
	Node(bool staticNode, float mass, float springConstantShrink, float springConstantShear, float springConstantBend, float3 position, float equilibriumDistance, float equilibriumDiagonalDistance); // vi börjar i equilibrium
	~Node(void);

	void updateInternalForces();
	void addAdjacensentNode(Node* n);
	void addSecondaryAdjacensentNode(Node* n);
	void addDiagonalNode(Node* n);
	void applyExternalForce(float3 externalForce);
	void updatePhysics(float time);

	float3 getForce();
	float getMass();
	float3 getVelocity();
	float3 getPosition();
	void setPosition(float3 position);
	float3 getAcceleration();

private:
	void molecularDynamicsMethod(float time);
	void euklidesMethod(float time);
	float3 force;
	float3 externalForce;
	float3 velocity;
	float3 lastPosition;
	float3 position;
	float3 acceleration;
	float equilibriumDistance;
	float equilibriumDiagonalDistance;
	float mass;
	vector<Node*> adjacesencenzyList;
	vector<Node*> secondaryAdjacesencenzyList;
	vector<Node*> diagonalList;
	bool staticNode;
	float springConstantShrink, springConstantShear, springConstantBend;
};

