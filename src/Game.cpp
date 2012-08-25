#include "Pch.h"
#include "Common.h"

/*

	Tiles

*/

struct Tile
{
	enum Kind
	{
		kEmpty,
		kFloor,
		kWall
	};

	Kind kind;

	Tile() : kind(kEmpty) { }
};

struct Map
{
	int _width;
	int _height;
	Tile* _tiles;

	Map() : _width(0), _height(0), _tiles(0) { }
	~Map() { Clear(); }

	bool Init(int width, int height)
	{
		Clear();

		if ((width < 1) || (height < 1))
			return false;

		if ((_tiles = new Tile[width * height]) == 0)
			return false;

		_width = width;
		_height = height;

		return true;
	}

	void Clear()
	{
		delete [] _tiles;

		_width = 0;
		_height = 0;
		_tiles = 0;
	}

	Tile* At(int x, int y)
	{
		if ((x < 0) || (y < 0) || (x >= _width) || (y >= _height))
			return 0;

		return &_tiles[x + (y * _width)];
	}

	bool CollidePt(int px, int py)
	{
		if (Tile* t = At(px >> 3, py >> 3))
			return t->kind != t->kFloor;

		return true;
	}

	bool CollideLine(int sx, int sy, int ex, int ey)
	{
		
	}
};

bool LoadMap(Map* map, const char* path)
{
	map->Clear();

	HANDLE h = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);

	if (h == INVALID_HANDLE_VALUE)
		return false;

	int length = GetFileSize(h, 0);

	const char* data = new char[length];

	if (!data)
	{
		CloseHandle(h);
		return false;
	}

	DWORD bytes;
	ReadFile(h, (void*)data, length, &bytes, 0);
	CloseHandle(h);

	int width = 0;
	int height = 1;

	for(int i = 0, x = 0; i < length; i++)
	{
		if ((data[i] == '\r') || (data[i] == '\n'))
		{
			width = Max(width, x);
			height++;
			x = 0;

			if (((i + 1) < length) && ((data[i + 1] == '\r') || (data[i + 1] == '\n')))
				i++;
		}
		else
			x++;
	}

	if (!map->Init(width, height))
	{
		delete [] data;
		return false;
	}

	for(int i = 0, x = 0, y = 0; i < length; i++)
	{
		if ((data[i] == '\r') || (data[i] == '\n'))
		{
			x = 0;
			y++;

			if (((i + 1) < length) && ((data[i + 1] == '\r') || (data[i + 1] == '\n')))
				i++;
		}
		else
		{
			if (Tile* tile = map->At(x, y))
			{
				switch(data[i])
				{
					case '#': tile->kind = Tile::kWall; break;
					case '.': tile->kind = Tile::kFloor; break;
				}
			}

			x++;
		}
	}

	delete [] data;

	return true;
}

Map gMap;

void GameInit()
{
	if (!LoadMap(&gMap, "data/map.txt"))
	{
		DebugLn("Failed to load map");
	}
}

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

	vel.x -= gKeyLeft * 10.0f;
	vel.x += gKeyRight * 10.0f;
	vel.y -= gKeyUp * 10.0f;
	vel.y += gKeyDown * 10.0f;

	pos += vel * (1.0f / 60.0f);
	vel *= 0.8f;

	Colour colour(1.0f);

	if (gKeyFire)
		colour = Colour(0.9f, 0.7f, 0.3f, 1.0f);

	static int anim =0;
	anim++;

	DrawRect(pos - Vector2(8), pos + Vector2(8), 1+((anim>>3)%3), colour);
}
