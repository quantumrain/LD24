#ifndef COMMON_H
#define COMMON_H

struct Vector2
{
	float x, y;

	Vector2(float xy = 0) : x(xy), y(xy) { }
	Vector2(float x_, float y_) : x(x_), y(y_) { }
};

inline Vector2 operator+(const Vector2& lhs, const Vector2& rhs) { return Vector2(lhs.x + rhs.x, lhs.y + rhs.y); }
inline Vector2 operator-(const Vector2& lhs, const Vector2& rhs) { return Vector2(lhs.x - rhs.x, lhs.y - rhs.y); }
inline Vector2 operator*(const Vector2& lhs, const Vector2& rhs) { return Vector2(lhs.x * rhs.x, lhs.y * rhs.y); }
inline Vector2 operator/(const Vector2& lhs, const Vector2& rhs) { return Vector2(lhs.x / rhs.x, lhs.y / rhs.y); }

inline Vector2& operator+=(Vector2& lhs, const Vector2& rhs) { lhs.x += rhs.x; lhs.y += rhs.y; return lhs; }
inline Vector2& operator-=(Vector2& lhs, const Vector2& rhs) { lhs.x -= rhs.x; lhs.y -= rhs.y; return lhs; }
inline Vector2& operator*=(Vector2& lhs, const Vector2& rhs) { lhs.x *= rhs.x; lhs.y *= rhs.y; return lhs; }
inline Vector2& operator/=(Vector2& lhs, const Vector2& rhs) { lhs.x /= rhs.x; lhs.y /= rhs.y; return lhs; }

inline Vector2 Rotation(float a) { return Vector2(cosf(a), sinf(a)); }

struct Colour
{
	float r, g, b, a;

	Colour(float rgba = 1) : r(rgba), g(rgba), b(rgba), a(rgba) { }
	Colour(float rgb, float a) : r(rgb), g(rgb), b(rgb), a(a) { }
	Colour(float r_, float g_, float b_, float a_) : r(r_), g(g_), b(b_), a(a_) { }
};

extern bool gHasFocus;
extern bool gKeyUp;
extern bool gKeyDown;
extern bool gKeyLeft;
extern bool gKeyRight;
extern bool gKeyFire;

void DebugLn(const char* txt, ...);
void ClearColour(Colour clearColour);
void DrawLine(Vector2 start, Vector2 end, Colour colour);

enum SoundId
{
	kSid_Select,
	kSid_Back,
	kSid_Max
};

void SoundInit();
void SoundShutdown();
void SoundPlay(SoundId sid, float freq, float volume);

#endif // COMMON_H