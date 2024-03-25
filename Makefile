all: release

release:
	mkdir -p ./build/release && \
	cd build/release && \
	cmake ../.. && \
	cmake --build . --config Release

clean:
	rm -rf build
