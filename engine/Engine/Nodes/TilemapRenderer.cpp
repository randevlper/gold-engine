#include "TilemapRenderer.h"

#include "Engine/AssetManagement/AssetManager.h"
#include "Engine/AssetManagement/Shader.h"
#include "Engine/AssetManagement/Tilemap.h"
#include "Engine/AssetManagement/Sprite.h"
#include "Engine/Utilities/Debug.h"

bgfx::VertexLayout TilemapVertex::pcvLayout;
TilemapVertex TilemapVertex::planeVerts[] = {
	{-1.0f, -1.0f, 0.0f, 0xffffffff, 0, 0x7fff},
	{1.0f, -1.0f, 0.0f, 0xffffffff, 0x7fff, 0x7fff},
	{1.0f, 1.0f, 0.0f, 0xffffffff, 0x7fff, 0},
	{-1.0f, 1.0f, 0.0f, 0xffffffff, 0, 0}
};
const uint16_t TilemapVertex::planeTriList[] = {
	0,1,2,
	0,2,3
};

void TilemapVertex::init()
{
	pcvLayout.begin()
		.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
		.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
		.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Int16, true, true)
		.end();
}

Shader* TilemapRenderer::_shader = nullptr;
bool TilemapRenderer::init = false;


TilemapRenderer::TilemapRenderer()
{
	if (!init) {
		TilemapVertex::init();
		_shader = AssetManager::get<Shader>("assets/shaders/vs_sprite.bin");
		init = true;
	}

	tilemap = nullptr;
	_s_tilemap = bgfx::createUniform("s_sprite", bgfx::UniformType::Sampler);
	_ibh = bgfx::createIndexBuffer(bgfx::makeRef(TilemapVertex::planeTriList, sizeof(TilemapVertex::planeTriList)));
}

TilemapRenderer::~TilemapRenderer()
{
	destroyVerticies();
	bgfx::destroy(_s_tilemap);
	bgfx::destroy(_ibh);
}

void TilemapRenderer::render()
{
	if (tilemap == nullptr) { return; }
	bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_BLEND_ALPHA, BGFX_STATE_BLEND_ADD);
	for (size_t i = 0; i < tilemap->size(); i++)
	{
		bgfx::setVertexBuffer(0, _tileVertexBuffers[i]);
		bgfx::setIndexBuffer(_ibh);
		bgfx::setTexture(0, _s_tilemap, tilemap->testSprite->handle);
		bgfx::submit(0, _shader->getHandle());
	}
}

void TilemapRenderer::setTilemap(Tilemap* tm)
{
	//Clear VBS

	tilemap = tm;
	_tileVertexBuffers = std::vector<bgfx::VertexBufferHandle>(tm->size());

	int x = 0;
	int y = 0;

	for (size_t i = 0; i < tm->size(); i++)
	{
		bgfx::VertexBufferHandle vbh;
		TilemapVertex vertex[4];
		memcpy(vertex, TilemapVertex::planeVerts, sizeof(TilemapVertex::planeVerts));


		x = (i % tm->getWidth()) * tm->tileWidth;
		y = (i / tm->getWidth()) * tm->tileHeight;

		for (size_t v = 0; v < 4; v++)
		{
			vertex[v].x *= (tm->tileWidth/2);
			vertex[v].x += x;
			vertex[v].y *= (tm->tileHeight/2);
			vertex[v].y += y;
			//vertex[v].y += y;
		}

		vbh = bgfx::createVertexBuffer(
			bgfx::copy(vertex,
				sizeof(TilemapVertex::planeVerts)),
			TilemapVertex::pcvLayout);

		_tileVertexBuffers[i] = vbh;
		//Debug::Log(std::to_string(i) + " Created VBH idx: " + std::to_string(vbh.idx));
	}
}

void TilemapRenderer::destroyVerticies()
{
	for (size_t i = 0; i < _tileVertexBuffers.size(); i++)
	{
		bgfx::destroy(_tileVertexBuffers[i]);
	}
	_tileVertexBuffers.clear();
}