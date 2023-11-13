#include <fstream>
#include <ios>
#include <cstdio>
#include <algorithm>

#include "disk_cache.hpp"

using namespace std;
namespace fs=std::filesystem;

error disk_cache::put(const char* data) {
	lock_guard<mutex> guard(wlock);

	auto dsize = sizeof(data);

	// check if beyond capacity.
	if (capacity > 0 && (size + dsize > capacity)) {
		if (auto res = fifo_drop(); res != error::ok) {
			return res;
		}
	}

	// first open.
	if (is == nullptr) {
		auto x = fstream(cur_write, ios::in|ios::out|ios::binary);
		is = &x;
	}

	// write header(4 bytes) and data.
	is->write(reinterpret_cast<char*>(&dsize), sizeof dsize);
	is->write(data, size);

	// check write exceptions.
	try {
		is->exceptions(is->failbit);
	} catch(const ios_base::failure& e) {
		return error::write_failed;	
	}

	if (!no_sync) {
		if (is->sync() != 0) {
			return error::sync_failed;
		}
	}

	// update info
	cur_batch_size+=(dsize + 4);
	size += (dsize + 4);
	time(&last_write); // remember last write time.
	if (cur_batch_size >= batch_size) {
		if (auto res = rotate(); res != error::ok) {
			return res;
		}
	}

	return error::ok;
}

error disk_cache::fifo_drop() {
	return error::ok;
}

error disk_cache::rotate() {
	lock_guard<mutex> guard(rwlock);

	// TODO: we should make eof_hint the class static const.
	auto eof_hint = 0xdeadbeef;

	is->write(reinterpret_cast<char*>(&eof_hint), sizeof(eof_hint));

	// check write exceptions.
	try {
		is->exceptions(is->failbit);
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
	sprintf(buf.data(), fmt, idx);

	// comes new datafile
	auto new_file = dir/fs::path(buf.data());

	is->close();
	if (is->is_open()) {
		return error::is_close_failed;
	}
	is = nullptr;

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
