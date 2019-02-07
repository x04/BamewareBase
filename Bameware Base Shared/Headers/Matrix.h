#pragma once


namespace BAMEWARE
{
	template <typename T, size_t Rows, size_t Columns>
	class Matrix
	{
	public:
		Matrix() { SetAll(0); }
		Matrix(const Matrix<T, Rows, Columns>& other) { memcpy(m_elements, other.Get(), Rows * Columns * sizeof(T)); }

	public:
		void SetAll(T value)
		{
			for (size_t i = 0; i < Rows; i++)
			{
				for (size_t x = 0; x < Columns; x++)
					m_elements[i][x] = value;
			}
		}

		const T* Get() const { return &m_elements[0][0]; }
		T* Get() { return &m_elements[0][0]; }

	public:
		const T* operator[](const size_t& row) const { return m_elements[row]; }
		T* operator[](const size_t& row) { return m_elements[row]; }

	private:
		T m_elements[Rows][Columns];
	};

	typedef Matrix<float, 3, 4> Matrix3x4;
	typedef Matrix<float, 4, 4> Matrix4x4;
}