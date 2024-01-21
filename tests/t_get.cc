#include <iostream>

#include "disk_cache.hpp"
#include "gtest/gtest.h"

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#include "spdlog/spdlog.h"
#include "spdlog/fmt/bin_to_hex.h"

namespace fs = std::filesystem;
namespace {

	TEST(diskcache, header_n) {

		spdlog::set_level(spdlog::level::debug);
		auto dc = disk_cache("", 1<<10, 1<<20);
		auto arr = dc.header_bytes(disk_cache::eof_hint);
		auto len = dc.header_n(arr);

		SPDLOG_DEBUG("arr: {}", spdlog::to_hex(arr));
		SPDLOG_DEBUG("len: {0:x}, eof_hint: {1:x}", len, dc.eof_hint);
	}

	TEST(diskcache, get_and_switch) {
		spdlog::set_level(spdlog::level::debug);

		auto tmpp = fs::temp_directory_path();

		SPDLOG_DEBUG("temp dir: {}", tmpp.string());

		auto dc = disk_cache(tmpp, 1<<10, 1<<20);
		EXPECT_EQ(error::ok, dc.open());

		std::string d1 = "sample data";
		std::string d2 = "sample data with long suffix 111111111111111111111111";

		// rotate 2 data files
		EXPECT_EQ(error::ok, dc.put(d1.c_str()));
		EXPECT_EQ(error::ok, dc.rotate());
		EXPECT_EQ(error::ok, dc.put(d2.c_str()));
		EXPECT_EQ(error::ok, dc.rotate());

		EXPECT_EQ(2, dc.data_files().size());

		EXPECT_EQ(error::ok, dc.get([&](std::vector<char> &data, int n) -> error {
					SPDLOG_DEBUG("get {} bytes: {}", n, data.data());
					EXPECT_STREQ(d1.data(), data.data());
					EXPECT_EQ(d1.length(), n);
					return error::ok;
					}));

		EXPECT_EQ(error::ok, dc.get([&](std::vector<char> &data, int n) -> error {
			SPDLOG_DEBUG("get {} bytes: {}", n, data.data());
			EXPECT_STREQ(d2.data(), data.data());
			EXPECT_EQ(d2.length(), n);
			return error::ok;
		}));

		EXPECT_EQ(error::ok, dc.get([](std::vector<char>&, int) -> error {
					EXPECT_EQ(0, 1); // should not been here
					return error::ok;
					}));

		SPDLOG_DEBUG("get without callback, current batch size: {}", dc.cur_batch_size());
		EXPECT_EQ(error::ok, dc.put(d1.c_str())); // put another data
		EXPECT_EQ(error::ok, dc.put(d2.c_str())); // put another data
		EXPECT_EQ(error::ok, dc.put(d1.c_str())); // put another data
		EXPECT_EQ(error::ok, dc.put(d2.c_str())); // put another data
		EXPECT_EQ(error::ok, dc.rotate());

		EXPECT_EQ(error::ok, dc.get([&d1](std::vector<char> &data, int n) -> error {
			SPDLOG_DEBUG("get {} bytes: {}", n, data.data());
			EXPECT_STREQ(d1.data(), data.data());
			EXPECT_EQ(d1.length(), n);
			return error::ok;
					})); // get without callback
	}

	TEST(diskcache, get) {
		spdlog::set_level(spdlog::level::debug);

		auto tmpp = fs::temp_directory_path();

		SPDLOG_DEBUG("temp dir: {}", tmpp.string());

		auto dc = disk_cache(tmpp, 1<<10, 1<<20);
		EXPECT_EQ(error::ok, dc.open());

		std::string d1 = "sample data";
		std::string d2 = "sample data with long suffix 111111111111111111111111";
		EXPECT_EQ(error::ok, dc.put(d1.c_str()));
		EXPECT_EQ(error::ok, dc.put(d2.c_str()));

		EXPECT_EQ(error::ok, dc.rotate());

		SPDLOG_DEBUG("rotate ok");

		EXPECT_EQ(error::ok, dc.get([&](std::vector<char> &data, int n) -> error {
					SPDLOG_DEBUG("get {} bytes: {}", n, data.data());
					EXPECT_STREQ(d1.data(), data.data());
					EXPECT_EQ(d1.length(), n);
					return error::ok;
					}));

		EXPECT_EQ(error::ok, dc.get([&](std::vector<char> &data, int n) -> error {
					SPDLOG_DEBUG("get {} bytes: {}", n, data.data());
					EXPECT_STREQ(d2.data(), data.data());
					EXPECT_EQ(d2.length(), n);
					return error::ok;
					}));
	}
}
