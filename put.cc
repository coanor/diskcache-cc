#include <fstream>
#include <ios>
#include <cstdio>
#include <algorithm>

#include "disk_cache.hpp"
#include "spdlog/spdlog.h"

using namespace std;
namespace fs=std::filesystem;

error disk_cache::put(const char* data) {

	lock_guard<mutex> guard(wlock);

	auto dsize = strlen(data);

	// check if beyond capacity.
	if (capacity > 0 && (size + dsize > capacity)) {
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

	// check write exceptions.
	try {
		// write header(4 bytes) and data.
		char header[4];
		header[0] = (dsize>>24)&0xFF;
		header[1] = (dsize>>16)&0xFF;
		header[2] = (dsize>>8)&0xFF;
		header[3] = (dsize)&0xFF;

		spdlog::debug("try put header({} => {}) to {}...", dsize, header, cur_write.string());
		is.write(header, sizeof dsize);

		spdlog::debug("try put {} bytes data to {}...", dsize, cur_write.string());
		is.write(data, dsize);

		spdlog::debug("put {} bytes data ok", dsize);
		is.exceptions(is.failbit);
	} catch(const ios_base::failure& e) {
		return error::write_failed;	
	}

	if (!no_sync) {
		spdlog::debug("try sync...");
		if (is.sync() != 0) {
			return error::sync_failed;
		}
	}

	// update info
	_cur_batch_size+=(dsize + 4);
	size += (dsize + 4);
	time(&last_write); // remember last write time.
	if (_cur_batch_size >= batch_size) {
		spdlog::debug("try rotate, cur batch {}, batch size {}", _cur_batch_size, batch_size);
		if (auto res = rotate(); res != error::ok) {
			return res;
		}
	}

	return error::ok;
}

error disk_cache::fifo_drop() {

	spdlog::debug("try fifo drop...");

	lock_guard<mutex> guard(this->rwlock);
	if (data_files.size() == 0) {
		return error::ok;
	}

	auto fname = data_files.front();
	// reset current reading file
	if (os.is_open() && cur_read == fname) {
		os.close();
		if (os.is_open()) {
			return error::close_file_failed;
		}
	}

	size -= fs::file_size(fname);
	data_files.erase(data_files.begin());
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

	auto idx = 0;
	if (data_files.size() > 0) {
		// TODO: should we remember the last index?
		auto last = data_files[data_files.size()-1];
		auto ext = last.extension().c_str();
		auto last_idx = atol(ext);
		if (last_idx >= 0) {
			idx = last_idx + 1;
		}
	}

	auto fmt = "data.%032d";
	auto sz = snprintf(nullptr, 0, fmt, idx);
	vector<char> buf(sz+1); // +1 for null terminator
	snprintf(buf.data(), sizeof(buf), fmt, idx);

	// comes new datafile
	auto new_file = dir/fs::path(buf.data());

	is.close();
	if (is.is_open()) {
		return error::is_close_failed;
	}

	// rename data -> data.000N
	if (rename((dir/fs::path("data")).c_str(), new_file.c_str()) != 0) {
		return error::rename_failed;
	}

	// add new datafile
	data_files.push_back(new_file);

	// TODO: I think the sort are unnecessary.
	sort(data_files.begin(), data_files.end());

	return open_write_file();
}
