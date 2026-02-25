module;
#include <cmath>
#include <format>
#include <iostream>

export module Complex;

export class Complex
{
public:
	Complex(float re = 0.0f, float im = 0.0f) : Re(re), Im(im) {};
	~Complex() = default;
	float Re;
	float Im;

	float norm() const
	{
		return Re * Re + Im * Im;
	}
	Complex operator*(const float& c) const
	{
		return Complex(Re * c, Im * c);
	}
	Complex operator/(const float& c) const
	{
		return Complex(Re / c, Im / c);
	}
	void operator/=(const float& c)
	{
		Re /= c; Im /= c;
	}
	void operator*=(const float& c)
	{
		Re *= c; Im *= c;
	}

	Complex operator+(const Complex& z) const
	{
		return Complex(Re + z.Re, Im + z.Im);
	}
	void operator+=(const Complex& z)
	{
		Re += z.Re; Im += z.Im;
	}
	Complex operator-(const Complex& z) const
	{
		return Complex(Re - z.Re, Im - z.Im);
	}

	Complex operator*(const Complex& z) const
	{
		//(a+bi) * (c+di) = (ac-bd) + (ad+bc)i
		return Complex(Re * z.Re - Im * z.Im, Re * z.Im + Im * z.Re);
	}
	Complex operator/(const Complex& z) const
	{
		if (z.norm() >= 1e-9)
		{
			return Complex(Re * z.Re + Im * z.Im, Im * z.Re - Re * z.Im) / z.norm();
		}
		std::cout << "Complex division by zero (norm too small)" << std::endl;
		return Complex();
	}
};
export inline Complex operator*(const float& c, const Complex& z)
{
	return z * c;
}
export inline std::ostream& operator<<(std::ostream& out, const Complex& z)
{
	out << std::format("({:.2f},{:.2f}i)", z.Re, z.Im);
	return out;
}

export template <typename mathT>
Complex exp_C(mathT theta)
{
	return Complex(std::cos(theta), std::sin(theta));
}
