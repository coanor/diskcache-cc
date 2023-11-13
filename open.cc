#include "disk_cache.hpp"

namespace fs = std::filesystem;

error disk_cache::open() {
	if (batch_size <= 0) {
		batch_size=20*1024*1024;
	}

	if (max_data_size > batch_size) {
		max_data_size=static_cast<int>(batch_size/2);
	}

	// create dir if not exist: should we use try/cache here?
	if (!fs::exists(dir)) {
		if (fs::create_directories(dir)) {
			return error::create_path_failed;
		}
	}

	if (!no_lock) {
		// TODO: use file-locker to lock @dir
	}

	if (!no_pos) {
		// TODO: setup position of current cache
		auto p = pos(0, dir / fs::path(".pos"));
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
	return error::ok;
}

// load datafiles from a lagacy diskcache.
error disk_cache::load_exist_files() {
	return error::ok;
}

error disk_cache::seek_to_last_read() {
	return error::ok;
}
