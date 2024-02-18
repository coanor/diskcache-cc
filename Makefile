all:
	cmake -S . -B build/
	cmake --build build

test:
	cd build && ctest

ctags:
	ctags -R --c++-kinds=+p --fields=+iaS --extra=+q --language-force=C++
