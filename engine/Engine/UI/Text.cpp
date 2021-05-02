#include "Text.h"

#include <algorithm>

#include <glm/gtc/type_ptr.hpp>

#include "Engine/AssetManagement/AssetManager.h"
#include "Engine/AssetManagement/Font.h"
#include "Engine/AssetManagement/Shader.h"

#include "Engine/Core/Context.h"
#include "Engine/Core/Renderer.h"

#include "Engine/Data/VertexTypes.h"
#include "Engine/Utilities/TypeConversion.h"
#include "Engine/Utilities/glmJson.h"

#include "Engine/Data/UniformTypes.h"

namespace me {
	namespace ui {
		
		Shader* Text::_shader = nullptr;
		bool Text::_isInit = false;
		
		Text::Text()
		{
			_type = "Text";
			if (!_isInit) {
				_shader = AssetManager::get<Shader>("assets/shaders/vs_uisprite.bin");
				me::data::PositionColorUVVertex::init();
				
				_isInit = true;
			}

			_ibh = bgfx::createIndexBuffer(
				bgfx::makeRef(me::data::PositionColorUVVertex::indices, 
					sizeof(me::data::PositionColorUVVertex::indices)));
			_u_sprite = me::data::getSpriteUniform();
			_u_color = me::data::getColorUniform();
			_font = AssetManager::get<Font>("assets/fonts/cmunrm.ttf");
			_text = "";

			setColor({ 255,255,255,255 });
			_percentRender = 1.0f;
		}

		Text::~Text()
		{
			clearVertexBuffers();
			bgfx::destroy(_ibh);
		}

		void Text::setColor(glm::vec4 value)
		{
			_color = value;
		}

		void Text::setPercentRender(float value)
		{
			_percentRender = std::clamp(value, 0.0f, 1.0f);
		}

		void Text::render()
		{
			for (size_t i = 0; i < _vbs.size() * _percentRender; i++)
			{
				if (!bgfx::isValid(_vbs[i])) { continue; }
				bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_BLEND_ALPHA, BGFX_STATE_BLEND_ADD);
				//bgfx::setTransform(glm::value_ptr(transform.getGlobalMatrix()));
				bgfx::setVertexBuffer(0, _vbs[i]);
				bgfx::setIndexBuffer(_ibh);
				bgfx::setTexture(0, _u_sprite, _font->getCharacter(_text[i]).Handle);
				//bgfx::setUniform(_u_color, glm::value_ptr(_color));
				bgfx::submit(RENDER_FRAME_BUFFER_INDEX, _shader->getHandle(), sort);
			}
		}

		void Text::setFont(Font* font)
		{
			_font = font;
			buildVertexBuffers();
		}

		void Text::setText(std::string value)
		{
			_text = value;
			buildVertexBuffers();
		}

		nlohmann::json Text::to_json()
		{
			nlohmann::json j = NodeUI::to_json();
			j["type"] = "Text";
			j["color"] = _color;
			j["text"] = _text;
			j["font"] = _font->to_json();
			return j;
		}

		void Text::from_json(const nlohmann::json& j)
		{
			NodeUI::from_json(j);
			nlohmann::json::value_type color = j.at("color");
			_color = { color[0], color[1], color[2], color[3] };
			setText(j.at("text"));
			setFont(AssetManager::get_json<Font>(j["font"]));
		}

		void Text::buildVertexBuffers()
		{
			if (_font == nullptr) { return; }
			if (_text == "") { return; }

			clearVertexBuffers();

			glm::vec2* corners = rectTransform.getWindowCorners();

			float x = corners[3].x;
			float y = corners[3].y - _font->getCharacter('T').size.y;

			for (size_t i = 0; i < _text.size(); i++)
			{
				if (_text[i] == '\n') {
					bgfx::VertexBufferHandle invVbh;
					invVbh.idx = BGFX_INVALID_HANDLE;
					_vbs.push_back(invVbh);

					//https://stackoverflow.com/questions/28009564/new-line-pixel-distance-in-freetype
					//Take a look at this to fix spacing

					Character ch = _font->getCharacter('T');
					x = corners[3].x;
					y -= (ch.size.y + 10);
				}
				else {
					Character ch = _font->getCharacter(_text[i]);

					float xpos = x + ch.bearing.x;
					float ypos = y - (ch.size.y - ch.bearing.y);

					float w = ch.size.x;
					float h = ch.size.y;

					me::data::PositionColorUVVertex lineData[4];
					memcpy(lineData, me::data::PositionColorUVVertex::verts, sizeof(me::data::PositionColorUVVertex::verts));
					
					lineData[0].xy(me::util::convertPixelToScreen({ xpos, ypos }));
					lineData[1].xy(me::util::convertPixelToScreen({ xpos + w, ypos }));
					lineData[2].xy(me::util::convertPixelToScreen({ xpos + w, ypos + h }));
					lineData[3].xy(me::util::convertPixelToScreen({ xpos, ypos + h }));

					bgfx::VertexBufferHandle vbh = bgfx::createVertexBuffer(bgfx::copy(lineData, sizeof(lineData)), me::data::PositionColorUVVertex::layout);

					_vbs.push_back(vbh);
					x += (ch.advance >> 6);
				}
			}
		}

		void Text::clearVertexBuffers()
		{
			for (size_t i = 0; i < _vbs.size(); i++)
			{
				if (bgfx::isValid(_vbs[i])) {
					bgfx::destroy(_vbs[i]);
				}
			}
			_vbs.clear();
		}
	}
}