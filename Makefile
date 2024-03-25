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

clean:
	rm -rf build
