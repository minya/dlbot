#include "sentry_logger.h"

using namespace std;


SentryLogger::SentryLogger(const std::string& dsn, const std::string& crashpad_path) {
	sentry_options_t *options = sentry_options_new();
	sentry_options_set_dsn(options, dsn.c_str());
	sentry_options_set_handler_path(options, crashpad_path.c_str());
	sentry_options_set_database_path(options, ".sentrydb");
	sentry_init(options);
}

SentryLogger::~SentryLogger() {
	sentry_close();
}

void SentryLogger::Info(const std::string& message) {
	sentry_capture_event(
		sentry_value_new_message_event(SENTRY_LEVEL_INFO, "custom", message.c_str()));
}

void SentryLogger::Error(const std::string& message) {
	sentry_capture_event(
		sentry_value_new_message_event(SENTRY_LEVEL_ERROR, "custom", message.c_str()));
}