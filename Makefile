#-msse -msse2
CXXFLAGS = -O2 -static -static-libgcc -static-libstdc++
INCLUDES = -I include -I include/GLFW -I src/imgui -I src
LIBS = -L./lib -lglfw3dll -lglfw3 -lopengl32 -lbox2d -lws2_32 -liphlpapi
 
 all:
	i686-w64-mingw32-g++ -w $(CXXFLAGS) $(INCLUDES) src/*.cpp src/imgui/*.cpp src/scenes/*.cpp src/shapes/*.cpp src/*.c -o Game $(LIBS)