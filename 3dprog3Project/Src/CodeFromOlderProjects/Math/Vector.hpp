#pragma once


namespace rfm
{
	class Vector3;
	class Vector4
	{
	public:
		Vector4(float x, float y, float z, float w);
		Vector4(float val = 0);
		Vector4(const Vector3& v3, float w);
		~Vector4() = default;

		float& operator[] (int index) noexcept;
		float operator[] (int index) const noexcept;
		float length2() const;
		float length3() const;
		float length4() const;
		float x, y, z, w;
	};


	float dot2(const Vector4& v, const Vector4& u);
	float dot3(const Vector4& v, const Vector4& u);
	float dot4(const Vector4& v, const Vector4& u);
	Vector4 Pow(Vector4 v, float exp);
	Vector4 operator +(const Vector4& l, const Vector4& r);
	Vector4 operator +(const Vector4& l, const Vector3& r);
	Vector4 operator +(const Vector3& l, const Vector4& r);

	Vector4 operator -(const Vector4& l, const Vector4& r);
	Vector4 operator -(const Vector4& l, const Vector3& r);
	Vector4 operator -(const Vector3& l, const Vector4& r);



	//Vector3------------------------------------
	class Vector2;
	class Vector3
	{
	public:
		Vector3(float x, float y, float z);
		Vector3(float val = 0);
		Vector3(const Vector4& v);
		Vector3(const Vector2& v, float z);
		~Vector3() = default;

		float& operator[] (int index) noexcept;
		float operator[] (int index) const noexcept;
		Vector3& operator +=(const Vector3& other);
		Vector3& operator -=(const Vector3& other);
		Vector3& operator *=(const float& other);
		//operator DirectX::XMVECTOR() const { return { x, y, z, 0 }; };
		float length() const;
		void normalize();
		std::string ToString(int precision = 2) const;
		float x, y, z;
	};
	float dot(const Vector3& v, const Vector3& u);
	Vector3 cross(const Vector3& v, const Vector3& u);
	Vector3 normalize(const Vector3& v);
	Vector3 Pow(Vector3 v, float exp);
	Vector3 operator +(const Vector3& l, const Vector3& r);
	Vector3 operator -(const Vector3& l, const Vector3& r);
	Vector3 operator -(const Vector3& v);
	Vector3 operator *(const Vector3& l, const Vector3& r);
	Vector3 operator *(float scale, const Vector3& v);
	Vector3 operator *(const Vector3& v, float scale);
	Vector3 operator /(const Vector3& v, float scale);


	//Vector2--------------------------------------

	class Vector2I;
	class Vector2
	{
	public:
		Vector2(float x, float y);
		Vector2(float val = 0);
		Vector2(const Vector3& v);
		Vector2(const Vector2I& v);
		~Vector2() = default;

		float& operator[] (int index) noexcept;
		Vector2& operator +=(const Vector2& other);
		Vector2& operator /=(float scale);
		float length() const;
		void normalize();
		float x = 0;
		float y = 0;
	};
	float dot(const Vector2& v, const Vector2& u);


	Vector2 operator +(const Vector2& l, const Vector2& r);
	Vector2 operator -(const Vector2& l, const Vector2& r);
	Vector2 operator *(float scale, const Vector2& v);
	Vector2 operator *(const Vector2& v, float scale);
	Vector2 operator /(const Vector2& v, float scale);

	class Vector2I
	{
	public:
		Vector2I(int x, int y);
		Vector2I(int val = 0);
		~Vector2I() = default;

		bool operator ==(const Vector2I& other) const;
		int& operator[] (int index) noexcept;
		Vector2I& operator +=(const Vector2I& other);
		float length() const;
		int x = 0;
		int y = 0;
	};

	Vector2I operator +(const Vector2I& l, const Vector2I& r);
	Vector2I operator -(const Vector2I& l, const Vector2I& r);
	Vector2I operator *(int scale, const Vector2I& v);
	Vector2I operator /(const Vector2I& v, int scale);


}


template <>
struct std::hash<rfm::Vector2I>
{
	std::size_t operator()(const rfm::Vector2I& v) const
	{
		return std::hash<int64_t>()(static_cast<int64_t>(v.x) ^ (static_cast<int64_t>(v.y) << 32));
	}
};