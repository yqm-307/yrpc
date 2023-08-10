#pragma once
#include <bbt/Logger/Logger.hpp>

#define TRACE(fmt, ...) BBT_BASE_LOG_TRACE(fmt, ##__VA_ARGS__)
#define DEBUG(fmt, ...) BBT_BASE_LOG_DEBUG(fmt, ##__VA_ARGS__)
#define WARN(fmt,  ...) BBT_BASE_LOG_WARN(fmt, ##__VA_ARGS__)
#define INFO(fmt,  ...) BBT_BASE_LOG_INFO(fmt, ##__VA_ARGS__)
#define ERROR(fmt, ...) BBT_BASE_LOG_ERROR(fmt, ##__VA_ARGS__)
#define FATAL(fmt, ...) BBT_BASE_LOG_FATAL(fmt, ##__VA_ARGS__)