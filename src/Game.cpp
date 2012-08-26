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
		kWall
	};

	Kind kind;

	Tile() : kind(kEmpty) { }
};

struct CollisionResult
{
	Tile* tile;
	int px, py;

	CollisionResult() : tile(0), px(-1), py(-1) { }
	CollisionResult(Tile* tile_, int px_, int py_) : tile(tile_), px(px_), py(py_) { }

	operator bool() const { return tile != 0; }
};

struct Map
{
	int _width;
	int _height;
	Tile* _tiles;
	Vector2 _playerStart;

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
		_playerStart = Vector2();
	}

	Tile* At(int x, int y)
	{
		if ((x < 0) || (y < 0) || (x >= _width) || (y >= _height))
			return 0;

		return &_tiles[x + (y * _width)];
	}

	Tile* AtPx(int x, int y)
	{
		return At(x >> 4, y >> 4);
	}

	bool CollidePt(int px, int py)
	{
		if (Tile* t = AtPx(px, py))
			return t->kind != Tile::kEmpty;

		return false;
	}

	CollisionResult HCollide(int px0, int px1, int py)
	{
		int dist = px1 - px0;
		int dlt = 1;

		if (dist < 0)
		{
			dist = -dist;
			dlt = -1;
		}

		for(int x = px0; dist >= 0; dist--, x += dlt) // TODO: Test only tile edges
		{
			if (CollidePt(x, py))
				return CollisionResult(AtPx(x, py), x, py);
		}

		return CollisionResult();
	}

	CollisionResult VCollide(int px, int py0, int py1)
	{
		int dist = py1 - py0;
		int dlt = 1;

		if (dist < 0)
		{
			dist = -dist;
			dlt = -1;
		}

		for(int y = py0; dist >= 0; dist--, y += dlt) // TODO: Test only tile edges
		{
			if (CollidePt(px, y))
				return CollisionResult(AtPx(px, y), px, y);
		}

		return CollisionResult();
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
					case 'P': map->_playerStart = Vector2(x * 16.0f + 8.0f, y * 16.0f + 15.0f); break;
				}
			}

			x++;
		}
	}

	delete [] data;

	return true;
}

struct Player
{
	Vector2 pos;
	Vector2 vel;
};

Map gMap;
Player gPlayer;

void GameInit()
{
	if (!LoadMap(&gMap, "data/map.txt"))
	{
		DebugLn("Failed to load map");
	}
	else
	{
		gPlayer.pos = gMap._playerStart;
	}
}

void GameUpdate()
{
	static bool dir;
	static bool jump;
	static bool start;
	static bool onGround;
	static int groundTime;
	static int jumpTime;

	gPlayer.vel.x -= gKeyLeft * 30.0f;
	gPlayer.vel.x += gKeyRight * 30.0f;
	
	if (gKeyUp)
	{
		if (groundTime > 4)
		{
			if (!jump)
			{
				SoundPlay(kSid_Select, 1.0f, 0.5f);

				gPlayer.vel.y -= 90.0f;
				jumpTime = 15;
				jump = true;
			}
		}
		else
			jump = false;
	}
	else
	{
		jump = false;
		jumpTime = 0;
	}

	if (gKeyUp && (jumpTime > 0))
	{
		gPlayer.vel.y -= 5.0f;
		jumpTime--;
	}
	else
		jumpTime = 0;

	bool isMoving = gKeyLeft || gKeyRight || gKeyUp || gKeyDown;

	if (gKeyLeft ^ gKeyRight)
		dir = gKeyLeft ? 0 : 1;

	gPlayer.vel.x *= 0.7f;
	gPlayer.vel.y = Clamp(gPlayer.vel.y + 5.0f, -200.0f, 200.0f);

	gPlayer.pos.x += gPlayer.vel.x * (1.0f / 60.0f);

	{ // X
		float px = floorf(gPlayer.pos.x + 0.5f);
		float py = floorf(gPlayer.pos.y + 0.5f);

		if (CollisionResult cr = gMap.HCollide((int)px, (int)(px - 6.0f), (int)(py - 7.0f))) { gPlayer.pos.x = cr.px + 6.5f; gPlayer.vel.x = 0.0f; }
		if (CollisionResult cr = gMap.HCollide((int)px, (int)(px + 6.0f), (int)(py - 7.0f))) { gPlayer.pos.x = cr.px - 6.5f; gPlayer.vel.x = 0.0f; }

		if (CollisionResult cr = gMap.HCollide((int)px, (int)(px - 6.0f), (int)(py - 1.0f))) { gPlayer.pos.x = cr.px + 6.5f; gPlayer.vel.x = 0.0f; }
		if (CollisionResult cr = gMap.HCollide((int)px, (int)(px + 6.0f), (int)(py - 1.0f))) { gPlayer.pos.x = cr.px - 6.5f; gPlayer.vel.x = 0.0f; }
	}

	onGround = false;
	gPlayer.pos.y += gPlayer.vel.y * (1.0f / 60.0f);

	{ // Y
		float px = floorf(gPlayer.pos.x + 0.5f);
		float py = floorf(gPlayer.pos.y + 0.5f);

		if (CollisionResult cr = gMap.VCollide((int)(px - 5.0f), (int)(py - 4.0f), (int)(py - 9.0f))) { gPlayer.pos.y = cr.py + 9.5f; gPlayer.vel.y = 0.0f; jumpTime = 0; }
		if (CollisionResult cr = gMap.VCollide((int)(px - 5.0f), (int)(py - 4.0f), (int)(py + 1.0f))) { gPlayer.pos.y = cr.py - 1.5f; gPlayer.vel.y = 0.0f; onGround = true; }

		if (CollisionResult cr = gMap.VCollide((int)(px + 5.0f), (int)(py - 4.0f), (int)(py - 9.0f))) { gPlayer.pos.y = cr.py + 9.5f; gPlayer.vel.y = 0.0f; jumpTime = 0; }
		if (CollisionResult cr = gMap.VCollide((int)(px + 5.0f), (int)(py - 4.0f), (int)(py + 1.0f))) { gPlayer.pos.y = cr.py - 1.5f; gPlayer.vel.y = 0.0f; onGround = true; }
	}

	if (onGround)
		groundTime++;
	else
		groundTime = 0;

	Colour colour(1.0f);

	if (gKeyFire)
		colour = Colour(0.9f, 0.7f, 0.3f, 1.0f);

	static int anim =0;
	anim++;

	int frame = 0;

	if (isMoving)
		frame = 4 + ((anim >> 3) % 3);
	else
		frame = ((anim >> 5) % 4);

	if (!onGround)
	{
		if (fabsf(gPlayer.vel.y) < 30.0f)
			frame = 8;
		else
			frame = (gPlayer.vel.y < 0.0f) ? 7 : 9;
	}
	
	Vector2 pan = -gPlayer.pos;

	for(int y = 0; y < gMap._height; y++)
	{
		for(int x = 0; x < gMap._width; x++)
		{
			if (Tile* t = gMap.At(x, y))
			{
				if (t->kind == Tile::kEmpty)
					continue;

				Vector2 p0(x * 16.0f, y * 16.0f);
				Vector2 p1((x + 1) * 16.0f, (y + 1) * 16.0f);

				DrawRect(p0 + pan, p1 + pan, 16, 0, Colour());
			}
		}
	}

	DrawSprite(gPlayer.pos + pan - Vector2(dir ? 1.0f : -1.0f, 7.0f), Vector2(1.0f), frame, dir ? kFlipX : 0, colour);
}
