#include "Pch.h"
#include "Common.h"

void GameUpdate()
{
	static Vector2 pos;
	static Vector2 vel;
	static float t;
	static bool beep;

	if (gKeyFire)
	{
		if (!beep)
		{
			SoundPlay(kSid_Select, 1.0f, 1.0f);
		}

		beep = true;
	}
	else
		beep = false;

	t += 0.05f;

	vel.x -= gKeyLeft * 20.0f;
	vel.x += gKeyRight * 20.0f;
	vel.y -= gKeyUp * 20.0f;
	vel.y += gKeyDown * 20.0f;

	pos += vel * (1.0f / 60.0f);
	vel *= 0.8f;

	Colour colour(0.8f, 0.2f, 0.3f, 1.0f);

	if (gKeyFire)
		colour = Colour(0.9f, 0.7f, 0.3f, 1.0f);

	DrawLine(pos + Rotation(t) * 8.0f, pos - Rotation(t) * 8.0f, colour);
	DrawLine(pos + Rotation(t + 1.5f) * 8.0f, pos - Rotation(t + 1.5f) * 8.0f, colour);
}
