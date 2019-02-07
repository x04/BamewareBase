#pragma once

#include <iostream>


namespace BAMEWARE
{
	template <typename T, size_t NumElements>
	class Vector
	{
	public:
		Vector() { SetAll(T{}); }
		Vector(const Vector<T, NumElements>& vec) { memcpy(m_elements, vec.m_elements, sizeof(T) * NumElements); }

		Vector(const std::initializer_list<T>& init_list)
		{
			SetAll(T{});
			memcpy(m_elements, data(init_list), sizeof(T) * init_list.size());
		}

	public:
		Vector<T, NumElements>& operator=(const Vector<T, NumElements>& vec)
		{
			memcpy(m_elements, vec.m_elements, sizeof(T) * NumElements);
			return *this;
		}

		T& operator[](size_t index) { return m_elements[index]; }
		const T& operator[](size_t index) const { return m_elements[index]; }

		bool operator==(const Vector<T, NumElements>& vec) const
		{
			return !memcmp(m_elements, vec.m_elements, sizeof(T) * NumElements);
		}

		bool operator!=(const Vector<T, NumElements>& vec) const { return !(*this == vec); }

		const Vector<T, NumElements> operator+(const Vector<T, NumElements>& vec) const
		{
			Vector<T, NumElements> ret = *this;

			for (size_t i = 0; i < NumElements; i++)
				ret.m_elements[i] += vec.m_elements[i];

			return ret;
		}

		const Vector<T, NumElements> operator-(const Vector<T, NumElements>& vec) const
		{
			Vector<T, NumElements> ret = *this;

			for (size_t i = 0; i < NumElements; i++)
				ret.m_elements[i] -= vec.m_elements[i];

			return ret;
		}

		const Vector<T, NumElements> operator*(const Vector<T, NumElements>& vec) const
		{
			Vector<T, NumElements> ret = *this;

			for (size_t i = 0; i < NumElements; i++)
				ret.m_elements[i] *= vec.m_elements[i];

			return ret;
		}

		const Vector<T, NumElements> operator/(const Vector<T, NumElements>& vec) const
		{
			Vector<T, NumElements> ret = *this;

			for (size_t i = 0; i < NumElements; i++)
				ret.m_elements[i] /= vec.m_elements[i];

			return ret;
		}

		const Vector<T, NumElements> operator+(T value) const
		{
			Vector<T, NumElements> ret = *this;

			for (size_t i = 0; i < NumElements; i++)
				ret.m_elements[i] += value;

			return ret;
		}

		const Vector<T, NumElements> operator-(T value) const
		{
			Vector<T, NumElements> ret = *this;

			for (size_t i = 0; i < NumElements; i++)
				ret.m_elements[i] -= value;

			return ret;
		}

		const Vector<T, NumElements> operator*(T value) const
		{
			Vector<T, NumElements> ret = *this;

			for (size_t i = 0; i < NumElements; i++)
				ret.m_elements[i] *= value;

			return ret;
		}

		const Vector<T, NumElements> operator/(T value) const
		{
			Vector<T, NumElements> ret = *this;

			for (size_t i = 0; i < NumElements; i++)
				ret.m_elements[i] /= value;

			return ret;
		}

		Vector<T, NumElements>& operator+=(const Vector<T, NumElements>& vec) { return *this = *this + vec; }
		Vector<T, NumElements>& operator-=(const Vector<T, NumElements>& vec) { return *this = *this - vec; }
		Vector<T, NumElements>& operator*=(const Vector<T, NumElements>& vec) { return *this = *this * vec; }
		Vector<T, NumElements>& operator/=(const Vector<T, NumElements>& vec) { return *this = *this / vec; }

		Vector<T, NumElements>& operator+=(T value) { return *this = *this + value; }
		Vector<T, NumElements>& operator-=(T value) { return *this = *this - value; }
		Vector<T, NumElements>& operator*=(T value) { return *this = *this * value; }
		Vector<T, NumElements>& operator/=(T value) { return *this = *this / value; }

	public:
		void SetAll(T value)
		{
			for (auto& e : m_elements)
				e = value;
		}

		T& At(size_t index) { return m_elements[index]; }
		const T& At(size_t index) const { return m_elements[index]; }
		const T* Get() const { return m_elements; }

		double Sum(size_t num_elems = NumElements) const
		{
			double sum = 0.f;
			for (size_t i = 0; i < num_elems; i++)
				sum += m_elements[i];
			return sum;
		}

		double Length(size_t num_elems = NumElements) const
		{
			double total = 0.f;
			for (size_t i = 0; i < num_elems; i++)
				total += double(m_elements[i]) * double(m_elements[i]);
			return T(sqrt(total));
		}

		/// usage: Print(std::cout)
		void Print(std::ostream& stream) const
		{
			if (NumElements == 0)
				return;

			stream << double(m_elements[0]);
			for (size_t i = 1; i < NumElements; i++)
				stream << ", " << double(m_elements[i]);
			stream << std::endl;
		}

	private:
		T m_elements[NumElements];
	};

	typedef Vector<float, 4> Vector4DF;
	typedef Vector<int, 4> Vector4DI;
	typedef Vector<float, 3> Vector3DF;
	typedef Vector<int, 3> Vector3DI;
	typedef Vector<float, 2> Vector2DF;
	typedef Vector<int, 2> Vector2DI;
}
