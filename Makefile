all: release

release:
	mkdir -p build/release && \
	cd build/release && \
	cmake $(GENERATOR) -DCMAKE_BUILD_TYPE=Release ../.. && \
	cmake --build . --config Release && \
	cmake --build . --target install

debug:
	mkdir -p build/debug && \
	cd build/debug && \
	cmake $(GENERATOR) -DCMAKE_BUILD_TYPE=Debug ../.. && \
	cmake --build . --config Debug && \
	cmake --build . --target install

format:
	cp src/quack/third_party/duckdb/.clang-format .
	find src/ -iname "*.hpp" -o -iname "*.cpp" | xargs clang-format --sort-includes=0 -style=file -i
	cmake-format -i CMakeLists.txt
	rm .clang-format

clean:
	rm -rf build
