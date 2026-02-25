module;
#include <iostream>
#include <cmath>
#include <algorithm>
#include <numbers>
#include <vector>

export module DSP;
import Complex;


/// <summary>
/// Computes the discrete convolution of input signal with response, returns a vector whose length matches the input signal.
/// </summary>
/// <typeparam name="mathT">Numeric element type for the signal and response.</typeparam>
/// <param name="signal">Input signal vector to be convolved</param>
/// <param name="response">Response vector convolved with the signal</param>
/// <returns>A vector of length signal.size() </returns>
export template <typename mathT>
std::vector<mathT> Convolution(const std::vector<mathT>& signal,
	const std::vector<mathT>& response)
{
	std::size_t N = signal.size();
	std::size_t M = response.size();
	std::vector<mathT> result(N, mathT(0));

	for (std::size_t i = 0; i < N; i++) {
		mathT sum(0);
		for (std::size_t j = 0; j <= i; j++) {
			if (j < M) {
				sum = sum + (signal[i - j] * response[j]);
			}
		}
		result[i] = sum;
	}
	return result;
}

export template <typename mathT>
Complex DiscreteFT(const std::vector<mathT>& signal, float freq, float sample_rate)
{
	Complex result(0, 0);
	float dt = 1 / sample_rate;
	float w = 2 * std::numbers::pi_v<float> * freq;

	float theta = 0;
	for (size_t n = 0; n < signal.size(); n++)
	{
		result += signal[n] * exp_C(theta);
		theta += w * dt;
	}
	return result;
}
