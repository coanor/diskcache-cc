#include <string>
#include <ctime>
#include <chrono>
#include <mutex>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <vector>

#include "flock.hpp"
#include "pos.hpp"
#include "err.hpp"

typedef int(*get_callback)(const char* data); 

class disk_cache {
public:
	disk_cache(std::filesystem::path dir, long bsize, long cap)
		: batch_size(bsize),capacity(cap) {
			no_sync = false;
			max_data_size = 0;
			wakeup = std::chrono::seconds(3);
			cur_write = dir / std::filesystem::path("data");
			dir_lock = nullptr;
			cur_pos = nullptr;
		};

	// TODO: should we define a copy constructor?
	
	error open();
	error close();
	error put(const char* data);
	error get(get_callback* callback);
	error rotate();

	void set_no_lock(bool on) { no_lock = on; }
	void set_no_pos(bool on) { no_pos = on; }
	void set_capacity(long cap) { capacity = cap; }

	~disk_cache();

private:
	error open_write_file();
	error load_exist_files();
	error seek_to_last_read();
	error fifo_drop();

private:
	std::filesystem::path dir; // dir of all datafiles 
	std::string cur_read;
	std::string cur_write;

	std::fstream* is;
	std::fstream* os;
	std::time_t last_write;
	std::chrono::seconds wakeup;

	std::mutex wlock; // exclude write
	std::mutex rlock; // exclude read
	std::mutex rwlock; // exclude switch/rotate/drop/Close
			
	flock *dir_lock;
	pos *cur_pos;

	long size; // current byte size
	long cur_batch_size; // current writing file's size
	long batch_size; // current batch size(static)
	long capacity; // capacity of the diskcache
		  
	std::vector<std::filesystem::path> data_files;

	int max_data_size; // max data size of single Put()

	bool no_sync; // NoSync if enabled, may cause data missing, default false
	bool no_fallback_on_error; // ignore Fn() error
	bool no_pos; // no position
	bool no_lock; // no file lock
};
