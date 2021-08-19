#pragma once

#include "sentry.h"

#include <string>

class SentryLogger {

public:
	SentryLogger(const std::string& dsn, const std::string& crashpad_path);

	virtual ~SentryLogger();

	void Info(const std::string& message);

	void Error(const std::string& message);
};