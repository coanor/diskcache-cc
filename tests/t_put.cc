#include <iostream>

#include "disk_cache.hpp"
#include "gtest/gtest.h"

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#include "spdlog/spdlog.h"

namespace fs = std::filesystem;
namespace l = spdlog;

namespace {

	std::string gen_random(const int len) {
		static const char alphanum[] =
			"0123456789"
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz";
		std::string tmp_s;
		tmp_s.reserve(len);

		for (int i = 0; i < len; ++i) {
			tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
		}

		return tmp_s;
	}

	TEST(diskcache, put) {
		l::set_level(l::level::debug);
		auto tmpp = fs::temp_directory_path();

		SPDLOG_DEBUG("temp dir: {}", tmpp.string());

		auto dc = disk_cache(tmpp, 0, 0);
		EXPECT_EQ(error::ok, dc.open());

		SPDLOG_DEBUG("open diskcache ok");

		auto len = 8;
		auto some_data = gen_random(len);

		SPDLOG_DEBUG("try put data...");
		EXPECT_EQ(error::ok, dc.put(some_data.c_str()));
		EXPECT_EQ(error::ok, dc.put(some_data.c_str()));

		EXPECT_EQ(error::ok, dc.rotate()); // rotate added a EOF hint

		// check 1st file size
		auto first_file = dc.data_files().front();
		EXPECT_EQ((len+4)*2+sizeof(disk_cache::eof_hint), fs::file_size(first_file));
	}

	TEST(diskcache, next_datafile_idx) {
		l::set_level(l::level::debug);
		auto tmpp = fs::temp_directory_path();

		SPDLOG_DEBUG("temp dir: {}", tmpp.string());
		auto dc = disk_cache(tmpp, 0, 0);
		EXPECT_EQ(error::ok, dc.open());

		auto files = std::vector<std::filesystem::path>{
			fs::path("data.001"),
			fs::path("data.002"),
			fs::path("data.003"),
		};

		auto idx = dc.next_datafile_idx(files);
		EXPECT_EQ(4, idx);
	}

	TEST(diskcache, fifo_on_put) {
		l::set_level(l::level::debug);
		auto tmpp = fs::temp_directory_path();

		SPDLOG_DEBUG("temp dir: {}", tmpp.string());

		auto total = 0;
		auto mb = 1<<20;
		auto max  = 100*mb;
		auto batch = mb;
		auto cap = 20*mb;
		auto some_data = gen_random(32*1024);

		auto dc = disk_cache(tmpp, batch, cap);
		EXPECT_EQ(error::ok, dc.open());

		for (;;) {
			EXPECT_EQ(error::ok, dc.put(some_data.c_str()));
			total += some_data.length();
			if (total > max) {
				break;
			}
		}

		EXPECT_EQ(error::ok, dc.rotate());

		SPDLOG_DEBUG("total put: {}", total);
		SPDLOG_DEBUG("fifo dropped: {}, size: {}", dc.fifo_dropped(), dc.size());

		for (auto n: dc.data_files()) {
			SPDLOG_DEBUG("datafile {}", n.string());
		}

		EXPECT_EQ(cap/mb, dc.data_files().size());
	}
}
