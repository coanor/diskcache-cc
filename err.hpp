#ifndef __ERR_H__
#define __ERR_H__ 

enum class error {
	ok,
	not_ok,
	too_large_data,
	create_path_failed,
	write_failed,
	sync_failed,
	is_close_failed,
	rename_failed,
	close_file_failed,
	open_file_failed,
	read_header_failed,
	read_data_failed,
};

#endif
