
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <GL/GL.h>

#include "Quaternion.h"
using namespace Crystal;
using namespace Crystal::Math;

Quaternion::Quaternion() : s(0), v(glm::vec3{ 0 })
{}

Quaternion::Quaternion(CRSTf32 real, CRSTf32 x, CRSTf32 y, CRSTf32 z)
	: s(real), v(glm::vec3(x, y, z))
{};

Quaternion::Quaternion(CRSTf32 scalar, const glm::vec3& vec)
	: s(scalar), v(vec)
{}

Quaternion::~Quaternion()
{}

Quaternion& Quaternion::operator+=(const Quaternion& q)
{
	this->s += q.s;
	this->v += q.v;
	return *this;
}

Quaternion Quaternion::operator+(const Quaternion& q) const
{
	return Quaternion(this->s + q.s, this->v + q.v);
}

Quaternion& Quaternion::operator-=(const Quaternion& q)
{
	this->s -= q.s;
	this->v -= q.v;
	return *this;
}

Quaternion Quaternion::operator-(const Quaternion& q) const
{
	return Quaternion(this->s - q.s, this->v - q.v);
}

Quaternion& Quaternion::operator*=(const Quaternion& other)
{
	CRSTf32 new_s{ this->s * other.s - glm::dot(this->v, other.v) };
	glm::vec3 new_v{ this->s * other.v + this->v * other.s + glm::cross(this->v, other.v) };
	this->s = new_s, this->v = new_v;
	return *this;
}

Quaternion Quaternion::operator*(const Quaternion& other) const
{
	Quaternion result{ *this };
	result *= other;
	return result;
}

CRSTf64 Quaternion::modulo(const Quaternion& q)
{
	CRSTf64 modulo{ 0 };
	CRSTf64 squared_modulo{ q.s * q.s + glm::dot(q.v, q.v) };
	if (squared_modulo > 1e-14) modulo = sqrt(squared_modulo);
	return modulo;
}

Quaternion Quaternion::get_normalize() const
{
	Quaternion q{ *this };
	CRSTf64 m{ Quaternion::modulo(*this) };
	if (m == 0) q.s = 1.0f, q.v = glm::vec3(0.0f);
	else
	{
		CRSTf64 inversed_modulo{ 1.0 / m };
		q.s *= static_cast<CRSTf32>(inversed_modulo);
		q.v *= static_cast<CRSTf32>(inversed_modulo);
	}
	return q;
}

void Quaternion::normalize(Quaternion& q)
{
	CRSTf64 m{ Quaternion::modulo(q) };
	if (m == 0) q.s = 1.0f, q.v = glm::vec3(0.0f);
	else
	{
		CRSTf64 inversed_modulo{ 1.0 / m };
		q.s *= static_cast<CRSTf32>(inversed_modulo);
		q.v *= static_cast<CRSTf32>(inversed_modulo);
	}
}

Quaternion Quaternion::get_conjugate() const
{
	Quaternion q{ *this };
	q.v = -q.v;
	return q;
}

void Quaternion::conjugate(Quaternion& q)
{
	q.v = -q.v;
}

Quaternion Quaternion::get_inverse() const
{
	Quaternion q{ *this };
	CRSTf64 m{ Quaternion::modulo(q) };
	if (m == 0) q.s = 1.0f, q.v = glm::vec3(0.0f);
	else
	{
		CRSTf64 invsquared_modulo{ 1.0 / (m * m) };
		q.s *= static_cast<CRSTf32>( invsquared_modulo);
		q.v *= static_cast<CRSTf32>(-invsquared_modulo);
	}
	return q;
}

void Quaternion::inverse(Quaternion& q)
{
	CRSTf64 m{ Quaternion::modulo(q) };
	if (m == 0) q.s = 1.0f, q.v = glm::vec3(0.0f);
	else
	{
		CRSTf64 invsquared_modulo{ 1.0 / (m * m) };
		q.s *= static_cast<CRSTf32>(invsquared_modulo);
		q.v *= static_cast<CRSTf32>(-invsquared_modulo);
	}
}

Quaternion Quaternion::gen_rotater(CRSTf32 angle, const glm::vec3& axis)
{
	return Quaternion{ cos(angle / 2.0f), sin(angle / 2.0f) * glm::normalize(axis) };
}

Quaternion Quaternion::get_rotate(CRSTf32 angle, const glm::vec3& axis) const
{
	Quaternion rotater{ Quaternion::gen_rotater(angle, axis) };
	return rotater * (*this) * rotater.get_conjugate();
}

void Quaternion::rotate(Quaternion& q_to_rotate, CRSTf32 angle, const glm::vec3& axis)
{
	Quaternion rotater{ Quaternion::gen_rotater(angle, axis) };
	q_to_rotate = rotater * q_to_rotate * rotater.get_conjugate();
}

void Quaternion::rotate(glm::vec3& vec_to_rotate, CRSTf32 angle, const glm::vec3& axis)
{
	Quaternion rotater{ Quaternion::gen_rotater(angle, axis) };
	Quaternion q_to_rotate{ 0.0f, vec_to_rotate };

	q_to_rotate = rotater * q_to_rotate * rotater.get_conjugate();
	vec_to_rotate = q_to_rotate.v;
}