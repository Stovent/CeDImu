CXX = g++
CXXFLAGS = -Wall -std=c++11 -pipe -mthreads
linkerFLAGS = -s -static-libstdc++ -static-libgcc -static -mthreads

defines = -D__GNUWIN32__ -D__WXMSW__ -DWXUSINGDLL -DwxUSE_UNICODE
wxLib = -lwxmsw30u_core -lwxbase30u -lwxpng -lwxzlib -lole32 -luuid -loleaut32 -lwinspool -lcomctl32 -mwindows

wxLibPath = C:\wxWidgets-3.0.4\lib\gcc_lib
wxIPath = C:\wxWidgets-3.0.4\include
wxIMSWU = C:\wxWidgets-3.0.4\lib\gcc_lib\mswu


obj = bin/obj/Main.o bin/obj/CeDImu.o bin/obj/MainFrame.o bin/obj/GamePanel.o


CeDImu.exe :
	windres.exe -I$(wxIPath) -I$(wxIMSWU) -J rc -O coff -i Ressources/ressource.rc -o bin/obj/ressource.res
	$(CXX) $(CXXFLAGS) -O3 -I$(wxIPath) -I$(wxIMSWU) $(defines) $(wxLib) -c src/GUI/GamePanel.cpp -o bin/obj/GamePanel.o
	$(CXX) $(CXXFLAGS) -O3 -I$(wxIPath) -I$(wxIMSWU) $(defines) $(wxLib) -c src/GUI/MainFrame.cpp -o bin/obj/MainFrame.o
	$(CXX) $(CXXFLAGS) -O3 -I$(wxIPath) -I$(wxIMSWU) $(defines) $(wxLib) -c src/CeDImu.cpp -o bin/obj/CeDImu.o
	$(CXX) $(CXXFLAGS) -O3 -I$(wxIPath) -I$(wxIMSWU) $(defines) $(wxLib) -c src/Main.cpp -o bin/obj/Main.o
	$(CXX) -L$(wxLibPath) -o bin/CeDImu.exe $(obj) bin/obj/ressource.res $(linkerFLAGS) $(wxLib)

