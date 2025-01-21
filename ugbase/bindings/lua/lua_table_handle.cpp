// LICENSE HEADER
// (ø) todo license header?

#include <iostream>
#include <cassert>
#include "common/util/variant.h"

#include "externals/lua/src/lua.hpp"

#include "lua_table_handle.h"

#define untested() ( std::cerr <<  "@@#\n@@@:"<< __FILE__ << ":"<< __LINE__ \
          <<":" << __func__ << "\n" )

namespace ug {
namespace impl {

static Variant pop2var(lua_State* _L)
{
	const int t = lua_type(_L, -1);
	Variant ret;

	if(t == LUA_TTABLE){
		const LuaTableHandle h(_L, -1);
		ret = Variant(h);
	}else if(t == LUA_TNUMBER){
		const number n = lua_tonumber(_L, -1);
		ret = Variant(n);
	}else if(t == LUA_TBOOLEAN){
		const bool b = lua_toboolean(_L, -1);
		ret = Variant(b);
	}else if(lua_isstring(_L, -1)){
		const std::string s(lua_tostring(_L, -1));
		ret = Variant(s);
	}else if(t == LUA_TNIL){
	}else{
		std::cerr << "type not handled " << t << "\n";
	}

	return ret;
}

struct LuaTableHandle_{
	LuaTableHandle_(lua_State*, int);
	~LuaTableHandle_();
	lua_State* _L{nullptr};
	int _ref{0};
	int _index{0};

	bool operator==(const LuaTableHandle_ & o) const{
		return _ref == o._ref;
	}
	bool operator!=(const LuaTableHandle_ & o) const{
		return !operator==(o);
	}

	static void attach(LuaTableHandle_* c, LuaTableHandle_** to) {
		assert(to);
		if (c == *to) { untested();
		}else if (!c) { untested();
			detach(to);
		}else if (!*to) {
			++(c->_attach_count);
			*to = c;
		}else if (*c != **to) {
			// They are different, usually by edit.
			detach(to);
			++(c->_attach_count);
			*to = c;
		}else if (c->_attach_count == 0) { untested();
			// The new and old are identical.
			// Use the old one.
			// The new one is not used anywhere, so throw it away.
			delete c;
		}else{ untested();
			// The new and old are identical.
			// Use the old one.
			// The new one is also used somewhere else, so keep it.
		}
	}
	static void detach(LuaTableHandle_** from) {
		assert(from);
		if (*from) {
			assert((*from)->_attach_count > 0);
			--((*from)->_attach_count);
			if ((*from)->_attach_count == 0) {
				untested();
				delete *from;
			}else{
			}
			*from = nullptr;
		}else{
		}
	}

	size_t size() const{
		size_t n = 0;
#if 0 // count them. skip nils.
		lua_pushnil(_L);
		while (lua_next(_L, _index) != 0) { untested();
			lua_pop(_L, 1);
			++n;
		}
#else
		n = lua_rawlen(_L, _index);
#endif
		return n;
	}

	Variant get(const int & key) const{
		lua_rawgeti(_L, LUA_REGISTRYINDEX, _ref);
		lua_rawgeti(_L, _index, key+1); // lua starts at 1.

		Variant ret = pop2var(_L);

		lua_pop(_L, 1); // pop value
		lua_pop(_L, 1); // pop ref

		return ret;
	}
	Variant get(const std::string & key) const{
		lua_rawgeti(_L, LUA_REGISTRYINDEX, _ref);
		lua_getfield(_L, _index, key.c_str());

		// std::cerr << "getfield " << key << " type " << lua_type(_L, -1) << "\n";

		Variant ret = pop2var(_L);

		lua_pop(_L, 1); // pop value
		lua_pop(_L, 1); // pop ref
		return ret;
	}

//	int get_int() const { }

private:
	int _attach_count{0};
};

LuaTableHandle_::LuaTableHandle_(lua_State* L, int index)
{
	lua_pushvalue(L, index); // copy table to top of stack.
	int ref = luaL_ref(L, LUA_REGISTRYINDEX); // pops copy from stack

	_ref = ref;
	_L = L;
	_index = index;

	_attach_count = 0;
}

LuaTableHandle_::~LuaTableHandle_()
{ untested();
	assert(!_attach_count);

	// drop reference to table from index.
	luaL_unref(_L, LUA_REGISTRYINDEX, _ref);
}

} // impl

LuaTableHandle::LuaTableHandle(lua_State* L, int index)
	: _data(nullptr)
{
	auto d = new impl::LuaTableHandle_(L, index);
	impl::LuaTableHandle_::attach(d, &_data);
	//lua_settable(L, 0); // lapi.c?
	//lua_gettop(L); // lapi.c?
}

LuaTableHandle::LuaTableHandle(const LuaTableHandle & p)
    : _data(nullptr)
{
	impl::LuaTableHandle_::attach(p._data, &_data);
}

LuaTableHandle::LuaTableHandle(LuaTableHandle&& p) noexcept
	: _data(nullptr)
{
	impl::LuaTableHandle_::attach(p._data, &_data);
	impl::LuaTableHandle_::detach(&p._data);
}

LuaTableHandle::~LuaTableHandle()
{
	impl::LuaTableHandle_::detach(&_data);
}


LuaTableHandle& LuaTableHandle::operator=(const LuaTableHandle & p)
{
	untested();
	impl::LuaTableHandle_::attach(p._data, &_data);
	return *this;
}

LuaTableHandle& LuaTableHandle::operator=(LuaTableHandle&& p) noexcept {
	impl::LuaTableHandle_::attach(p._data, &_data);
	impl::LuaTableHandle_::detach(&p._data);
	return *this;
}

size_t LuaTableHandle::size() const
{
	assert(_data);
	return _data->size();
}

Variant LuaTableHandle::get(const std::string & key) const
{
	assert(_data);
	return _data->get(key);
}

Variant LuaTableHandle::get(int & key) const
{
	assert(_data);
	return _data->get(key);
}

}