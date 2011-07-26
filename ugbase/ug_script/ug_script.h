// created by Sebastian Reiter
// s.b.reiter@googlemail.com
// y10 m01 d20

#ifndef __H__UG__UG_SCRIPT__
#define __H__UG__UG_SCRIPT__
#include <vector>
#include <string>

extern "C" {
#include "externals/lua/lua.h"
#include "externals/lua/lauxlib.h"
#include "externals/lua/lualib.h"
}

#include "common/common.h"

namespace ug
{

namespace script
{

///	Error class thrown if an error occurs during parsing.
class LuaError : public UGError
{
	public:
		LuaError(const char* msg) : UGError(msg)	{}
};



///	loads and parses a file. Several paths are tried if the file is not found.
/**	Throws an instance of LuaError, if a parse error occurs.
 * This method first tries to load the file specified with filename relative
 * to the path of the currently parsed file (if LoadUGScript is called from
 * within a load-script). If this failed, the file is tried to be loaded
 * with the raw specified filename. If this fails too, the method tries to
 * load the file from ugs scripting directory.
 *
 * Note that this method pushes the path of the currently parsed script to
 * PathProvider when parsing starts, and pops it when parsing is done.*/
bool LoadUGScript(const char* filename);

/// checks if given file exists.
bool FileExists(const char* filename);

///	returns the default lua state
/**	When called for the first time, a new state is created and
 *	the methods and classes in ugs default registry
 *	(ug::bridge::GetUGRegistry) are registered. Furthermore a callback
 *	is registered, which registers new methods whenever
 *	Registry::registry_changed() is called on the default registry.*/
lua_State* GetDefaultLuaState();

///	parses and executes a buffer
/**	Throws an instance of LuaError, if a parse error occurs.*/
bool ParseBuffer(const char* buffer, const char *bufferName="buffer");

///	parses and executes a file
/**	Throws an instance of LuaError, if a parse error occurs.*/
bool ParseFile(const char* filename);

/// UGLuaPrint. Redirects LUA prints to UG_LOG
int UGLuaPrint(lua_State *L);

/// UGLuaWrite. prints LUA output to UG_LOG without adding std::endl automatically
int UGLuaWrite(lua_State *L);

}//	end of namespace
}//	end of namespace


#endif
