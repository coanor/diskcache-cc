#include <iostream>
#include "disk_cache.hpp"
#include "spdlog/spdlog.h"

namespace fs = std::filesystem;
using namespace std;

error disk_cache::open() {
	if (batch_size <= 0) {
		batch_size=20*1024*1024;
	}

	if (max_data_size > batch_size) {
		max_data_size=static_cast<int>(batch_size/2);
	}

	// create dir if not exist: should we use try/cache here?
	if (!fs::exists(dir)) {
		spdlog::debug("dir {} not exist, try create...", dir.string());
		auto ec = std::error_code();
		fs::create_directories(dir, ec);
		if (ec.value() > 0) {
			spdlog::error("dir {} create failed", dir.string());
			return error::create_path_failed;
		}
	} else {
		spdlog::debug("dir {} exist", dir.string());
	}

	cur_write= dir/fs::path("data");

	if (!no_lock) {
		// TODO: use file-locker to lock @dir
	}

	if (!no_pos) {
		// TODO: setup position of current cache
		auto p = pos{
			.seek = 0,
			.name = dir / fs::path(".pos"),
		};
		cur_pos = &p;
	}

	// TODO: read envs to adjust parameters...
	
	if (auto res = open_write_file(); res != error::ok) {
		return res;
	}

	if (auto res = load_exist_files(); res != error::ok) {
		return res;
	}

	// seek to last read postion for get.
	if (!no_pos) {
		if (auto res = seek_to_last_read(); res != error::ok) {
			return res;
		}
	}

	return error::ok;
}

// open file "data" for writing.
error disk_cache::open_write_file() {
	is.open(cur_write,
			std::ios_base::in |
			std::ios_base::out |
			std::ios_base::app |
			std::ios_base::binary |
			std::ios_base::ate
			);

	if (!is.is_open()) {
		return error::open_file_failed;
	}

	return error::ok;
}

// load datafiles from a lagacy diskcache.
error disk_cache::load_exist_files() {
	return error::ok;
}

error disk_cache::seek_to_last_read() {
	return error::ok;
}
