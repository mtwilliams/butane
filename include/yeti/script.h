//===-- yeti/script.h -------------------------------------*- mode: C++ -*-===//
//
//                             __ __     _   _
//                            |  |  |___| |_|_|
//                            |_   _| -_|  _| |
//                              |_| |___|_| |_|
//
//       This file is distributed under the terms described in LICENSE.
//
//===----------------------------------------------------------------------===//
//
// TODO(mtwilliams): Document the purpose of this file.
//
//===----------------------------------------------------------------------===//

#ifndef _YETI_SCRIPT_H_
#define _YETI_SCRIPT_H_

#include "yeti/config.h"
#include "yeti/linkage.h"
#include "yeti/foundation.h"

extern "C" {
  #include "lua.h"
  #include "lauxlib.h"
  #include "lualib.h"
  #include "luajit.h"
}

namespace yeti {

/// ...
class YETI_PUBLIC Script {
 YETI_DISALLOW_COPYING(Script);

 public:
  Script();
  ~Script();

 private:
  static void *__alloc(Script *script, void *ptr, size_t old_sz, size_t new_sz);
  static int __error_handler(lua_State *L);

 public:
  /// Creates a module called @module.
  void add_module(const char *module);

  /// Adds @fn as @name to the @module.
  void add_module_function(const char *module,
                           const char *name,
                           const lua_CFunction fn);

 public:
  lua_State *state();

 private:
  lua_State *L;
};

YETI_INLINE lua_State *Script::state() {
  return L;
}

} // yeti

#endif // _YETI_SCRIPT_H_
