cmake_debug:
	cmake . -B cmake-build-debug  -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_BUILD_TYPE=DEBUG -G "Unix Makefiles"

# add phony
build_debug:
	cd cmake-build-debug; make all -j8

clean:
	cd cmake-build-debug; make clean

cmake_production:
	cmake . -B cmake-build-release  -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_BUILD_TYPE=RELEASE -G "Unix Makefiles"

build_production:
	cd cmake-build-release; make all -j8

clean_prod:
	cd cmake-build-release; make clean


# for build/{filename}, compile by `./cmake-build-debug/hoshi_lang examples/{filename}.hoshi -o build/{filename} --build-mode debug --preserve-intermediate`
build/%: examples/%.hoshi build_debug
	./cmake-build-debug/hoshi_lang $< -o $@ --build-mode debug --preserve-intermediate