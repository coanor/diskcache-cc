#include <iostream>

#include "disk_cache.hpp"
#include "gtest/gtest.h"

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_sinks.h"

namespace fs = std::filesystem;

namespace {

	TEST(diskcache, spdlog_simple) {
		SPDLOG_INFO("this is a info log should show file:line");
		SPDLOG_DEBUG("this is a debug log should show file:line");
	}

	TEST(diskcache, spdlog) {
		spdlog::set_level(spdlog::level::debug);
		auto console = spdlog::stdout_logger_mt("console");
		spdlog::set_default_logger(console);
		spdlog::set_pattern("[%s:%#.%!] %v");

		SPDLOG_INFO("this is a info log should show file:line");
		SPDLOG_DEBUG("this is a debug log should show file:line");
	}

	TEST(diskcache, basic) {
		spdlog::set_level(spdlog::level::debug);

		auto tmpp = fs::temp_directory_path();

		SPDLOG_DEBUG("temp dir: {}", tmpp.string());

		auto dc = disk_cache(tmpp, 1<<10, 1<<20);
		EXPECT_EQ(error::ok, dc.open());
	}

	TEST(diskcache, open_and_put) {
		spdlog::set_level(spdlog::level::debug);

		auto tmpp = fs::temp_directory_path();

		SPDLOG_DEBUG("temp dir: {}", tmpp.string());

		auto dc = disk_cache(tmpp, 1<<10, 1<<20);
		EXPECT_EQ(error::ok, dc.open());

		EXPECT_EQ(error::ok, dc.put("sample data"));
		EXPECT_EQ(error::ok, dc.put("other sample data"));
		EXPECT_EQ(28+8, dc.cur_batch_size());
	}
}
