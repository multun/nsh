#pragma once

#define ATTR(Att) __attribute__((Att))
#define __unused ATTR(unused)
#define __malloc ATTR(malloc)
#define __noreturn ATTR(noreturn)
