#pragma once

#include "utils/error.h"

/**
** \details runtime_error is thrown when an error preventing the script from continuing occurs,
** such as fork failing.
*/
extern struct ex_class g_runtime_error;
