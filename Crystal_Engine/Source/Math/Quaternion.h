#pragma once
//==============================================================================
/*
	A simple quaternion lib
*/
//==============================================================================
#include <format>
#include <glm/glm.hpp>
#include "../Core/TypeAndConcepts.h"
//==============================================================================
namespace Crystal::Math
{
	class Quaternion
	{
		public
		:
		//==============================================================================
		Quaternion();
		Quaternion(CRSTf32 real, CRSTf32 x, CRSTf32 y, CRSTf32 z);
		Quaternion(CRSTf32 scalar, const glm::vec3 & vec);
		~Quaternion();

		Quaternion & operator+=(const Quaternion & q);
		Quaternion operator+(const Quaternion& q) const;
		Quaternion & operator-=(const Quaternion & q);
		Quaternion operator-(const Quaternion& q) const;

		Quaternion & operator*=(const Quaternion & other);
		Quaternion operator*(const Quaternion& other) const;
		friend std::ostream& operator<<(std::ostream& os, const Quaternion& q)
		{
			os << std::format("Quat[{:.2f},({:.2f},{:.2f},{:.2f})]", q.s, q.v.x, q.v.y, q.v.z);
			return os;
		}

		static CRSTf64 modulo(const Quaternion& q);
		Quaternion get_normalize() const;
		static void normalize(Quaternion & q);
		Quaternion get_conjugate() const;
		static void conjugate(Quaternion & q);
		Quaternion get_inverse() const;
		static void inverse(Quaternion & q);

		static Quaternion gen_rotater(CRSTf32 angle, const glm::vec3& axis);
		Quaternion get_rotate(CRSTf32 angle, const glm::vec3& axis) const;
		static void rotate(Quaternion& q_to_rotate, CRSTf32 angle, const glm::vec3& axis);
		static void rotate(glm::vec3& vec_to_rotate, CRSTf32 angle, const glm::vec3& axis);

		CRSTf32 s;
		glm::vec3 v;
	};
}
//==============================================================================