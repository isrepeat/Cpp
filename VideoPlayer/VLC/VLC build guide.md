# **Build for windows**
Official build guide - https://code.videolan.org/videolan/vlc/-/blob/master/doc/BUILD-win32.md

## **Precodnitions**
- WSL (with installed Ubuntu 20.0)
- Docker desktop - https://docs.docker.com/desktop/install/windows-install/


1. Run Docker instance and Ubuntu (WSL)

2. Pull videolan image and run container
	```bash
	docker run -it registry.videolan.org/vlc-debian-llvm-msvcrt:20240212151604 /bin/bash
	```

3. You can access read/write only in /home/videolan dir, so go to there
	```bash
	cd ~/
	```


4. Get vlc sources
	```bash
	git config --global core.autocrlf false
	git clone https://code.videolan.org/videolan/vlc.git
	mkdir build && cd build

	```

5. Building with LLVM prebuilt contribs (~20min)
	```bash
	export VLC_SOURCES=/home/videolan/vlc
	export VLC_CONTRIB_SHA="$(cd $VLC_SOURCES; extras/ci/get-contrib-sha.sh win32)"
	export VLC_PREBUILT_CONTRIBS_URL="https://artifacts.videolan.org/vlc/win64-llvm/vlc-contrib-x86_64-w64-mingw32-$VLC_CONTRIB_SHA.tar.bz2"
	
	# You can also make sure that the paths are correct:
	# echo $VLC_SOURCES && echo $VLC_CONTRIB_SHA && $VLC_PREBUILT_CONTRIBS_URL
	
	# '-p' - specify that need build with prebuild contributes
	# '-D' - your location on windows where will be placed built results with .pdb and sources, for example "D:/WORK/Videolan/vlc"
	time ../vlc/extras/package/win32/build.sh -a x86_64 -p -D "<PATH_TO_WINDOWS_VIDEOLAN_FOLDER>/vlc"
	```

6. Build vlc dev (sdk) package
	```bash
	cd win64
	make package-win-common
	```
	> NOTE: Do not close Ubutnu instanse where docker container now is running until all results not copied.

7. Run another instance Ubuntu and prepare folder where will be copied results:
	```bash
	# You can use another path
	sudo su
	mkdir /home/videolan
	```

8. In Docker desktop find the hash of the currently running container. Copy build results from docker container to Ubuntu host
	```bash
	docker cp <container_hash>:/home/videolan/build /home/videolan/build
	docker cp <container_hash>:/home/videolan/vlc /home/videolan/vlc
	```

9. Copy from Ubutnu (WSL) to Windows:
	```bash
	cp -r /home/videolan/build /mnt/<PATH_TO_WINDOWS_VIDEOLAN_FOLDER>/build
	cp -r /home/videolan/vlc /mnt/<PATH_TO_WINDOWS_VIDEOLAN_FOLDER>/vlc
	```


10. Set some environment variables before start debug:
	- Add to PATH:
		```bash
		<PATH_TO_WINDOWS_VIDEOLAN_FOLDER>\build\win64\lib\.libs
		<PATH_TO_WINDOWS_VIDEOLAN_FOLDER>\build\win64\src\.libs
		C:\Windows
		C:\Windows\System32
		C:\Windows\System32\downlevel
		```
	- Add VLC_PLUGIN_PATH variable:
		```bash
		VLC_PLUGIN_PATH=<PATH_TO_WINDOWS_VIDEOLAN_FOLDER>\build\win64\modules
		```


# **Debug**
## Debug via attaching to process
1. Open "<PATH_TO_WINDOWS_VIDEOLAN_FOLDER>\vlc" folder in Visual Studio
2. Run vlc.exe from "<PATH_TO_WINDOWS_VIDEOLAN_FOLDER>\build\win64\bin"
3. Attach to process

## Debug from libvlc samlpes projects
1. Copy "<PATH_TO_WINDOWS_VIDEOLAN_FOLDER>\vlc\doc\libvlc" folder to another place "<PATH_TO_WINDOWS_VLC_SAMPLES>"
2. Copy sdk folder "<PATH_TO_WINDOWS_VIDEOLAN_FOLDER>\build\win64\vlc-4.0.0-dev\sdk" to "<PATH_TO_WINDOWS_VLC_SAMPLES>"
3. Copy vlc folder (sources) "<PATH_TO_WINDOWS_VIDEOLAN_FOLDER>\vlc" to "<PATH_TO_WINDOWS_VLC_SAMPLES>"
4. Open "<PATH_TO_WINDOWS_VLC_SAMPLES>\CMakeLists.txt" via Visual Studio (File->Open->CMake)
5. Switch to "CMake targets view"
5. Set LIBVLC_SDK_INC and LIBVLC_SDK_LIB variables to corresponding paths in CMakeLists.txt:
	```bash
	set(LIBVLC_SDK_INC "${PROJECT_SOURCE_DIR}/sdk/include")
	set(LIBVLC_SDK_LIB "${PROJECT_SOURCE_DIR}/sdk/lib")
	```
6. Start debug for example d3d11_player