#include <iostream>

#include "disk_cache.hpp"
#include "gtest/gtest.h"

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#include "spdlog/spdlog.h"

namespace fs = std::filesystem;
namespace l = spdlog;

namespace {

	TEST(diskcache, fifo_on_put) {
		l::set_level(l::level::debug);
		l::set_pattern("[%H:%M:%S %z] [lvl: %L] [src: %s:%#] %v");
		//auto tmpp = fs::temp_directory_path();
		auto tmpp = fs::path("fifo_on_put");

		l::debug("temp dir: {}", tmpp.string());

		auto dc = disk_cache(tmpp, 1<<10, 2<<10);
		EXPECT_EQ(error::ok, dc.open());

		l::debug("open diskcache ok");

		auto some_data = "sample data";
		auto some_other_data = "other sample data";
		auto total  = 0;

		l::debug("try put data...");
		for (;;) {
			EXPECT_EQ(error::ok, dc.put(some_data));
			l::debug("try put {} ok", some_data);

			EXPECT_EQ(error::ok, dc.put(some_other_data));
			l::debug("try put {} ok", some_other_data);

			break;
			total += (std::strlen(some_data) + std::strlen(some_other_data));
			if (total > 32*1024) {
				break;
			} else {
				l::debug("total put: {}", total);
			}
		}
	}
}
