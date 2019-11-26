#pragma once

enum glob_flags
{
    GLOB_FLAGS_PERIOD = 1,
};

enum glob_status
{
    GLOB_MATCH = 0,
    GLOB_NOMATCH,
    GLOB_INVALID,
};

enum glob_status glob_match(const char *pattern, const char *meta, const char *string, int flags);
