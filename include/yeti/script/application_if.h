//===-- yeti/script/application_if.h ----------------------*- mode: C++ -*-===//
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

#ifndef _YETI_SCRIPT_APPLICATION_IF_H_
#define _YETI_SCRIPT_APPLICATION_IF_H_

#include "yeti/config.h"
#include "yeti/linkage.h"
#include "yeti/foundation.h"

namespace yeti {

class Script;
class Application;

namespace application_if {

/// ...
extern YETI_PUBLIC void expose(Script *script, Application *app);

} // application_if

} // yeti

#endif // _YETI_SCRIPT_APPLICATION_IF_H_
