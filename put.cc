#include <algorithm>
#include <cstdio>
#include <fstream>
#include <ios>

#include "disk_cache.hpp"
#include "spdlog/spdlog.h"

using namespace std;
namespace fs=std::filesystem;

error disk_cache::put(const char* data) {

	lock_guard<mutex> guard(wlock);

	auto dsize = strlen(data);

	// check if beyond capacity.
	if (capacity > 0 && (_size + dsize > capacity)) {
		if (auto res = fifo_drop(); res != error::ok) {
			return res;
		}
		_fifo_dropped++;
	}

	// first open.
	if (!is.is_open()) {
		spdlog::error("input stream not open, should not been here");
		return error::write_failed;
	}

	// write header(4 bytes) and data.
	char header[4];

	// check write exceptions.
	try {
		header[0] = (dsize>>24)&0xFF;
		header[1] = (dsize>>16)&0xFF;
		header[2] = (dsize>>8)&0xFF;
		header[3] = (dsize)&0xFF;

		is.write(header, sizeof(header));

		is.write(data, dsize);

		is.exceptions(is.failbit);
	} catch(const ios_base::failure& e) {
		return error::write_failed;	
	}

	if (!no_sync) {
		if (is.sync() != 0) {
			return error::sync_failed;
		}
	}

	// update info
	_cur_batch_size+=(dsize + sizeof(header));
	_size += (dsize + sizeof(header));
	time(&last_write); // remember last write time.
	if (_cur_batch_size >= batch_size) {
		spdlog::debug("try rotate, cur batch {}, batch size {}", _cur_batch_size, batch_size);
		if (auto res = rotate(); res != error::ok) {
			return res;
		}
		_cur_batch_size = 0; // reset
	}

	return error::ok;
}

error disk_cache::fifo_drop() {
	lock_guard<mutex> guard(this->rwlock);
	if (_data_files.size() == 0) {
		return error::ok;
	}

	auto fname = _data_files.front();
	// reset current reading file
	if (os.is_open() && cur_read == fname) {
		os.close();
		if (os.is_open()) {
			return error::close_file_failed;
		}
	}

	_size -= fs::file_size(fname);
	_data_files.erase(_data_files.begin());

	spdlog::debug("try fifo drop {}...", fname.string());
	remove(fname);

	return error::ok;
}

error disk_cache::rotate() {
	lock_guard<mutex> guard(rwlock);

	is.write(reinterpret_cast<const char*>(&eof_hint), sizeof(eof_hint));

	// check write exceptions.
	try {
		is.exceptions(is.failbit);
	} catch(const ios_base::failure& e) {
		return error::write_failed;	
	}

	auto idx = next_datafile_name(_data_files);

	// comes new datafile
	auto basename = fmt::format("data.{}", idx);
	auto new_file = dir/fs::path(basename);

	is.close();
	if (is.is_open()) {
		return error::is_close_failed;
	}

	// rename data -> data.N
	if (rename((dir/fs::path("data")).c_str(), new_file.c_str()) != 0) {
		return error::rename_failed;
	}

	// add new datafile
	_data_files.push_back(new_file);
	spdlog::debug("add datafile {}, datafiles {}...", new_file.string(), _data_files.size());

	return open_write_file();
}

int disk_cache::next_datafile_name(vector<fs::path> files)  const {
	auto idx = 0;
	auto dfiles = files.size();

	if (dfiles > 0) {
		auto last = files[dfiles-1];
		auto ext = last.extension().c_str();

		if (strlen(ext) > 0) {
			auto last_idx = atol(++ext);
			if (last_idx >= 0) {
				idx = last_idx + 1;
			}

			spdlog::debug("last {}, ext {}, idx {}...", last.string(), ext, idx);
		}
	}

	return idx;
}
