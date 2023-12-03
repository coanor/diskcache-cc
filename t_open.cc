#include <iostream>

#include "disk_cache.hpp"
#include "gtest/gtest.h"

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#include "spdlog/spdlog.h"

namespace fs = std::filesystem;

namespace {

	TEST(diskcache, basic) {
		spdlog::set_level(spdlog::level::debug);
		spdlog::set_pattern("[%H:%M:%S %z] [lvl: %L] [src: %s:%#] %v");

		auto tmpp = fs::temp_directory_path();

		spdlog::debug("temp dir: {}", tmpp.string());

		auto dc = disk_cache(tmpp, 1<<10, 1<<20);
		EXPECT_EQ(error::ok, dc.open());
	}

	TEST(diskcache, open_and_put) {
		spdlog::set_level(spdlog::level::debug);
		spdlog::set_pattern("[%H:%M:%S %z] [lvl: %L] [src: %s:%#] %v");

		auto tmpp = fs::temp_directory_path();

		spdlog::debug("temp dir: {}", tmpp.string());

		auto dc = disk_cache(tmpp, 1<<10, 1<<20);
		EXPECT_EQ(error::ok, dc.open());

		EXPECT_EQ(error::ok, dc.put("sample data"));
		EXPECT_EQ(error::ok, dc.put("other sample data"));
		EXPECT_EQ(28+8, dc.cur_batch_size());
	}
}
