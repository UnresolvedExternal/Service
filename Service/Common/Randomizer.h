#pragma once
#include <random>

class Randomizer
{
public:
	static Randomizer& Get();

	template <typename T>
		requires std::floating_point<T>
	T Between(T from, T to);

	template <typename T>
		requires std::integral<T>
	T Between(T from, T to);

	char Between(char from, char to);
	
private:
	Randomizer();
	Randomizer(Randomizer&) = delete;
	Randomizer& operator=(const Randomizer&) = delete;

	std::default_random_engine engine;
};

Randomizer& Randomizer::Get()
{
	static Randomizer randomizer;
	return randomizer;
}

template <typename T>
	requires std::floating_point<T>
T Randomizer::Between(T from, T to)
{
	return std::uniform_real_distribution<T>{ from, to }(engine);
}

template <typename T>
	requires std::integral<T>
T Randomizer::Between(T from, T to)
{
	return std::uniform_int_distribution<T>{ from, to }(engine);
}

char Randomizer::Between(char from, char to)
{
	return from + Between(0, to - from);
}

Randomizer::Randomizer() :
	engine{ std::random_device{}() }
{
	
}