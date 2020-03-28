#pragma once

#include "Engine/Nodes/Node.h"

#include <vector>
#include <bgfx/bgfx.h>
#include <glm/vec2.hpp>

class Shader;
class Tilemap;

struct TilemapVertex
{
	float x;
	float y;
	float z;
	uint32_t abgr;
	int16_t m_u;
	int16_t m_v;
	static bgfx::VertexLayout pcvLayout;
	static TilemapVertex planeVerts[];
	static const uint16_t planeTriList[];
	static void init();
};


class TilemapRenderer : public Node
{
public:
	TilemapRenderer();
	~TilemapRenderer() override;

	void render();
	void setTilemap(Tilemap* tm);
	int worldToTile(glm::vec2 pos);

	bgfx::IndexBufferHandle _ibh;
	bgfx::UniformHandle _s_tilemap;

private:
	void destroyVerticies();
	Tilemap* tilemap;
	std::vector<bgfx::VertexBufferHandle> _tileVertexBuffers;

	static Shader* _shader;
	static bool init;
};