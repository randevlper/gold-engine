#include "LuaManager.h"
#include <string>
#include <sstream>

#include <lua.hpp>

#include "Engine/Core/Context.h"
#include "Engine/Core/Audio.h"

#include "Engine/AssetManagement/AssetManager.h"
#include "Engine/AssetManagement/AudioClip.h"

#include "Engine/Utilities/Debug.h"
#include "Engine/Core/WorldManager.h"

#define LUA_CFUNCTION(name) lua_pushcfunction(_L, name); lua_setglobal(_L, #name);

lua_State* LuaManager::_L = nullptr;
std::vector<std::function<void(lua_State*)>> LuaManager::bindings = std::vector<std::function<void(lua_State*)>>();

int printC(lua_State* L) {
	int nargs = lua_gettop(L);

	std::ostringstream string;
	string << "[LUA] [PRINT] ";
	for (int i = 1; i <= nargs; i++) {
		if (lua_isstring(L, i)) {
			/* Pop the next arg using lua_tostring(L, i) and do your print */
			string << lua_tostring(L, i);
		}
		else {
			/* Do something with non-strings if you like */
		}
	}
	Debug::Log(string.str());
	return 0;
}

int quit(lua_State* L) {
	Context::setShouldClose(true);
	return 0;
}

int loadWorld(lua_State* L)
{
	const char* path = lua_tostring(L, -1);
	Debug::Log(std::string("[LUA] Loading world: ") + path);
	me::WorldManager::loadWorld(path);
	return 0;
}

int playBGM(lua_State* L) {
	const char* path = lua_tostring(L, -1);
	Debug::Log(std::string("[LUA] Playing BGM: ") + path);
	AudioClip* clip = AssetManager::getLoad<AudioClip>(path, ""); 
	Audio::playBGM(clip);
	return 0;
}

int muteBGM(lua_State* L) {
	const bool value = lua_toboolean(L, -1);
	Audio::muteBGM(value);
	return 0;
}

int isBGMMuted(lua_State* L) {
	lua_pushboolean(L, Audio::getIsBGMMuted());
	return 1;
}

void LuaManager::init()
{
	_L = luaL_newstate();
	LUA_CFUNCTION(loadWorld)
	LUA_CFUNCTION(printC)
	LUA_CFUNCTION(quit)
	LUA_CFUNCTION(playBGM)
	LUA_CFUNCTION(muteBGM)
	LUA_CFUNCTION(isBGMMuted)

	for (size_t i = 0; i < bindings.size(); i++)
	{
		bindings[i](_L);
	}

	Debug::Log("[LUA] Lua State created.");
}

void LuaManager::destroy()
{
	lua_close(_L);
}

void LuaManager::loadLua(const char* lua)
{
	if (luaL_dostring(_L, lua) != 0) {
		std::string err[2] = { "[LUA] " , lua_tostring(_L, -1) };
		Debug::LogError(err, 2);
	}
}

void LuaManager::luaFunction(const char* functionName)
{
	lua_getglobal(_L, functionName);
	if (lua_isfunction(_L, -1)) {
		if (lua_pcall(_L, 0, 0, 0) != 0) {
			std::ostringstream err;
			err << "[LUA] error running function ";
			err << functionName;
			err << lua_tostring(_L, -1);
			Debug::LogError(err.str());
		}
	}
}

void LuaManager::addBindings(std::function<void(lua_State*)> func)
{
	bindings.push_back(func);
}
