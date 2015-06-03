#include "Node.h"

Node::Node(bool staticNode, float mass, float springConstantShrink, float springConstantShear, float springConstantBend, float3 position, float equilibriumDistance, float equilibriumDiagonalDistance)
{
	this->staticNode = staticNode;
	this->mass = mass;
	this->position = position;
	this->lastPosition = position;
	this->springConstantShrink = springConstantShrink;
	this->springConstantShear = springConstantShear;
	this->springConstantBend = springConstantBend;
	this->equilibriumDistance = equilibriumDistance;
	this->equilibriumDiagonalDistance = equilibriumDiagonalDistance;
	this->force = make_vector(0.0f, 0.0f, 0.0f);
	this->externalForce = make_vector(0.0f, 0.0f, 0.0f);
	this->velocity = make_vector(0.0f, 0.0f, 0.0f);
	this->acceleration = make_vector(0.0f, 0.0f, 0.0f);
}

Node::~Node(void)
{
}

void Node::updateInternalForces() {
	float3 sumOfForces = {0,0,0};
	for (Node* n : adjacesencenzyList) {
		float3 direction = n->getPosition() - position;
		float3 eqVector =  (direction / length(direction)) * equilibriumDistance;
		float3 diff = (direction - eqVector);
		if (length(diff) < 0.0001f) {
			diff = make_vector(0.0f, 0.0f, 0.0f);
		}
		sumOfForces += diff * springConstantShrink;
	}
	for (Node* n : diagonalList) {
		float3 direction = n->getPosition() - position;
		float3 eqVector =  (direction / length(direction)) * equilibriumDiagonalDistance;
		float3 diff = (direction - eqVector);
		if (length(diff) < 0.0001f) {
			diff = make_vector(0.0f, 0.0f, 0.0f);
		}
		sumOfForces += diff * springConstantShear;
	}
	for (Node* n : secondaryAdjacesencenzyList) {
		float3 direction = n->getPosition() - position;
		float3 eqVector =  (direction / length(direction)) * equilibriumDistance * 2;
		float3 diff = (direction - eqVector);
		if (length(diff) < 0.0001f) {
			diff = make_vector(0.0f, 0.0f, 0.0f);
		}
		sumOfForces += diff * springConstantBend;
	}
	force = sumOfForces + externalForce;
	externalForce = make_vector(0.0f, 0.0f, 0.0f);
}

void Node::updatePhysics(float time) {
	if (staticNode) {
		return;
	}
	acceleration = force / mass; // F = m * a
	molecularDynamicsMethod(time);
}

void Node:: molecularDynamicsMethod(float time) {
	float3 newPosition = 2 * position - lastPosition + time * time * acceleration;
	lastPosition = position;
	position = newPosition;
}

void Node:: euklidesMethod(float time) {
	float3 meanVelocity = velocity + (acceleration * time / 2.0f);
	position = position + meanVelocity * time;
	velocity += acceleration * time;
}

void Node:: addAdjacensentNode(Node* n){
	adjacesencenzyList.push_back(n);
}

void Node:: addSecondaryAdjacensentNode(Node* n){
	secondaryAdjacesencenzyList.push_back(n);
}

void Node:: addDiagonalNode(Node* n) {
	diagonalList.push_back(n);
}

void Node:: applyExternalForce(float3 externalForce){
	this->externalForce += externalForce;
}

float3 Node:: getForce(){
	return force;
}

float Node:: getMass(){
	return mass;
}

float3 Node:: getVelocity(){
	return velocity;
}

float3 Node:: getPosition(){
	return position;
}

void Node:: setPosition(float3 position){
	this->position = position;
}

float3 Node:: getAcceleration(){
	return acceleration;
}