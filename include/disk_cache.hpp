#include <string>
#include <ctime>
#include <chrono>
#include <mutex>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <vector>
#include <functional>

#include "flock.hpp"
#include "pos.hpp"
#include "err.hpp"

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#include "spdlog/spdlog.h"

//typedef error (*get_callback)(std::vector<char> &data, int n); 

class disk_cache {
public:
	disk_cache(std::filesystem::path dir, long bsize, long cap)
		: dir(dir), batch_size(bsize), capacity(cap) {
			no_sync = false;
			max_data_size = 0;
			wakeup = std::chrono::seconds(3);
			cur_write = dir / std::filesystem::path("data");
			dir_lock = nullptr;
			cur_pos = nullptr;
			_cur_batch_size = 0;
			_size = 0;
			_fifo_dropped = 0;
		};

	// TODO: should we define a copy constructor?
	
	error open();
	error close();
	error put(const char* data);
	error get(std::function<error(std::vector<char> &data, int n)> func);
	error rotate();

	inline void set_no_lock(bool on) { no_lock = on; }
	inline void set_no_pos(bool on) { no_pos = on; }
	inline void set_capacity(long cap) { capacity = cap; }
	inline int fifo_dropped() const { return _fifo_dropped; };
	inline std::vector<std::filesystem::path> data_files() const { return _data_files; }

	~disk_cache() {
		// TODO
	}

	inline long cur_batch_size() const {
		return _cur_batch_size;
	}

	inline long size() const {
		return _size;
	}

	int next_datafile_idx(std::vector<std::filesystem::path>) const;

	// convert 4 bytes header to int.
	static inline unsigned int header_n(std::array<char, 4> arr) {
		return int(static_cast<unsigned char>(arr[0]) << 24 |
               static_cast<unsigned char>(arr[1]) << 16 |
               static_cast<unsigned char>(arr[2]) << 8 |
               static_cast<unsigned char>(arr[3]));
	}

	// convert int to 4 bytes header.
	static inline auto header_bytes(unsigned int n) {
		std::array<char, 4> res;
		res[3] = n & 0x000000ff;
		res[2] = (n & 0x0000ff00)>> 8;
		res[1] = (n & 0x00ff0000)>> 16;
		res[0] = (n & 0xff000000)>> 24;
		return res;
	}

public:
	static constexpr unsigned int eof_hint = 0xdeadbeef;

private:
	error open_write_file();
	error load_exist_files();
	error seek_to_last_read();
	error fifo_drop();
	error switch_next_file();
	error remove_cur_reading_file();

private:

	char *read_buf;
	std::filesystem::path dir; // dir of all datafiles 
	std::filesystem::path cur_read;
	std::filesystem::path cur_write;

	std::ifstream gets;
	std::ofstream puts;
	std::time_t last_write;
	std::chrono::seconds wakeup;

	std::mutex wlock; // exclude write
	std::mutex rlock; // exclude read
	std::mutex rwlock; // exclude switch/rotate/drop/Close
			
	_flock *dir_lock;
	pos *cur_pos;

	int _fifo_dropped;
	unsigned long _size; // current byte size
	unsigned long _cur_batch_size; // current writing file's size
	unsigned long batch_size; // batch size(static)
	unsigned long capacity; // capacity of the diskcache
		  
	std::vector<std::filesystem::path> _data_files;

	unsigned int max_data_size; // max data size of single Put()

	bool no_sync; // NoSync if enabled, may cause data missing, default false
	bool no_fallback_on_error; // ignore Fn() error
	bool no_pos; // no position
	bool no_lock; // no file lock
};
