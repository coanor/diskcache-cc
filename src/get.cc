#include <chrono>
#include <cstdio>

#include "disk_cache.hpp"
#include "spdlog/spdlog.h"
#include "spdlog/fmt/bin_to_hex.h"

using namespace std;
namespace fs=std::filesystem;

error disk_cache::get(function<error (vector<char> &data, int n)> callback) {
	lock_guard<mutex> guard(rlock);

	auto diff = chrono::system_clock::now() -
		chrono::system_clock::from_time_t(last_write);
	if (_cur_batch_size > 0 && diff > wakeup) {
		// TODO: do rotate
	}

	if (!gets.is_open()) {
		SPDLOG_DEBUG("gets not open, switch next file...");
		if (auto res = switch_next_file(); res != error::ok) {
			return res;
		}
	}

retry:
	if (!gets.is_open()) {
		SPDLOG_DEBUG("nothing left, it's ok");
		return error::ok; // EOF reached
	}

	std::array<char, 4> hdr;
	try {
		gets.read(hdr.data(), hdr.size());
		if (gets.gcount() != 4) {
			return error::read_header_failed;
		}
		gets.exceptions(gets.failbit);
	} catch(const ios_base::failure& e) {
		return error::read_header_failed;	
	}

	auto dsize = header_n(hdr);
	if (dsize == eof_hint) {
		if (auto res = switch_next_file(); res != error::ok) {
			return res;
		}
		//SPDLOG_DEBUG("on eof hint, switch next file...");
		goto retry;
	}

	SPDLOG_DEBUG("get header {}, datasize {}", spdlog::to_hex(hdr), dsize);

	std::vector<char> data;
	data.resize(dsize);

	try {
		gets.read(&data[0], dsize);
		if (gets.gcount() != dsize) {
			return error::read_data_failed;
		}
		gets.exceptions(gets.failbit);
	} catch(const ios_base::failure& e) {
		return error::read_data_failed;
	}

	if (callback != nullptr) {
		SPDLOG_DEBUG("pass {} bytes({}) to callback", dsize, data.data());
		if (auto res = callback(data, dsize); res != error::ok) {
			// TODO: seek back to re-read for the next time...
			return res;
		}
	} else {
		SPDLOG_DEBUG("no callback on {} bytes", dsize);
	}

	if (!no_pos) {
		cur_pos->seek += (4 + dsize);
		if (auto res = cur_pos->dumpFile(); res != error::ok) {
			return res;
		}
	}

	return error::ok;
}

error disk_cache::remove_cur_reading_file() {
	lock_guard<mutex> guard(rwlock);

	remove(cur_read);

	SPDLOG_DEBUG("remove {} ok", cur_read.string());

	if (gets.is_open()) {
		gets.close();

		if (gets.is_open()) {
			return error::get_close_failed;
		}

		// TODO: adjust size
	}

	cur_read = fs::path("");
	if (_data_files.size() > 0) {
		cur_read = _data_files[0];
		_data_files.erase(_data_files.begin());

		SPDLOG_DEBUG("switch to {}", cur_read.string());
	} 

	return error::ok;
}

error disk_cache::switch_next_file() {

	if (cur_read.string() != "") {
		if (auto res = remove_cur_reading_file(); res != error::ok) {
			return res;
		}
	}

	lock_guard<mutex> guard(rwlock);

	if (!no_pos) {
		cur_pos->reset();
	}

	if (_data_files.size() == 0) {
		return error::ok; // no file to switch, it's ok!
	} else {
		cur_read = _data_files[0]; // select 1st data file for reading
	}

	gets.open(cur_read,
			std::ios_base::in |
			std::ios_base::binary
			);

	if (!gets.is_open()) {
		return error::open_file_failed;
	}

	if (!no_pos) {
		cur_pos->seek = 0;
		cur_pos->name = cur_read;
		cur_pos->dumpFile();
	}

	SPDLOG_DEBUG("switch to {} ok", cur_read.string());
	return error::ok;
}
