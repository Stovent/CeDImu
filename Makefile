CXX = g++
CXXFLAGS = -Wall -std=c++11 -pipe -mthreads
linkerFLAGS = -static-libstdc++ -static-libgcc -static -mthreads

defines = -D__GNUWIN32__ -D__WXMSW__ -DWXUSINGDLL -DwxUSE_UNICODE
wxLib = -lwxmsw30u_adv -lwxmsw30u_core -lwxbase30u -lwxpng -lwxzlib -lole32 -luuid -loleaut32 -lwinspool -lcomctl32 -mwindows

wxLibPath = C:\wxWidgets-3.0.4\lib\gcc_lib
wxIPath = C:\wxWidgets-3.0.4\include
wxIMSWU = C:\wxWidgets-3.0.4\lib\gcc_lib\mswu


obj = bin/obj/Main.o bin/obj/CeDImu.o bin/obj/utils.o \
bin/obj/MainFrame.o bin/obj/DisassemblerFrame.o bin/obj/GamePanel.o bin/obj/RAMWatchFrame.o \
bin/obj/SCC68070.o bin/obj/Interpreter.o bin/obj/ConditionalTests.o bin/obj/InstructionSet.o bin/obj/AddressingModes.o bin/obj/MemoryAccess.o \
bin/obj/SCC66470.o bin/obj/DRAMInterface.o

GUI:
	$(CXX) $(CXXFLAGS) -O3 -I$(wxIPath) -I$(wxIMSWU) $(defines) $(wxLib) -c src/GUI/DisassemblerFrame.cpp -o bin/obj/DisassemblerFrame.o
	$(CXX) $(CXXFLAGS) -O3 -I$(wxIPath) -I$(wxIMSWU) $(defines) $(wxLib) -c src/GUI/MainFrame.cpp -o bin/obj/MainFrame.o
	$(CXX) $(CXXFLAGS) -O3 -I$(wxIPath) -I$(wxIMSWU) $(defines) $(wxLib) -c src/GUI/GamePanel.cpp -o bin/obj/GamePanel.o
	$(CXX) $(CXXFLAGS) -O3 -I$(wxIPath) -I$(wxIMSWU) $(defines) $(wxLib) -c src/GUI/RAMWatchFrame.cpp -o bin/obj/RAMWatchFrame.o
	$(CXX) -L$(wxLibPath) -o bin/CeDImu.exe $(obj) bin/obj/ressource.res $(linkerFLAGS) $(wxLib)

Cores:
	$(CXX) $(CXXFLAGS) -O3 -I$(wxIPath) -I$(wxIMSWU) $(defines) $(wxLib) -c src/Cores/SCC68070/SCC68070.cpp -o bin/obj/SCC68070.o
	$(CXX) $(CXXFLAGS) -O3 -I$(wxIPath) -I$(wxIMSWU) $(defines) $(wxLib) -c src/Cores/SCC68070/Interpreter.cpp -o bin/obj/Interpreter.o
	$(CXX) $(CXXFLAGS) -O3 -I$(wxIPath) -I$(wxIMSWU) $(defines) $(wxLib) -c src/Cores/SCC68070/MemoryAccess.cpp -o bin/obj/MemoryAccess.o
	$(CXX) $(CXXFLAGS) -O3 -I$(wxIPath) -I$(wxIMSWU) $(defines) $(wxLib) -c src/Cores/SCC68070/InstructionSet.cpp -o bin/obj/InstructionSet.o
	$(CXX) $(CXXFLAGS) -O3 -I$(wxIPath) -I$(wxIMSWU) $(defines) $(wxLib) -c src/Cores/SCC68070/AddressingModes.cpp -o bin/obj/AddressingModes.o
	$(CXX) $(CXXFLAGS) -O3 -I$(wxIPath) -I$(wxIMSWU) $(defines) $(wxLib) -c src/Cores/SCC68070/ConditionalTests.cpp -o bin/obj/ConditionalTests.o
	$(CXX) $(CXXFLAGS) -O3 -I$(wxIPath) -I$(wxIMSWU) $(defines) $(wxLib) -c src/Cores/SCC66470/DRAMInterface.cpp -o bin/obj/DRAMInterface.o
	$(CXX) $(CXXFLAGS) -O3 -I$(wxIPath) -I$(wxIMSWU) $(defines) $(wxLib) -c src/Cores/SCC66470/SCC66470.cpp -o bin/obj/SCC66470.o
	$(CXX) -L$(wxLibPath) -o bin/CeDImu.exe $(obj) bin/obj/ressource.res $(linkerFLAGS) $(wxLib)

CeDImu :
	windres.exe -I$(wxIPath) -I$(wxIMSWU) -J rc -O coff -i Ressources/ressource.rc -o bin/obj/ressource.res
	$(CXX) $(CXXFLAGS) -O3 -I$(wxIPath) -I$(wxIMSWU) $(defines) $(wxLib) -c src/utils.cpp -o bin/obj/utils.o
	$(CXX) $(CXXFLAGS) -O3 -I$(wxIPath) -I$(wxIMSWU) $(defines) $(wxLib) -c src/Cores/SCC68070/SCC68070.cpp -o bin/obj/SCC68070.o
	$(CXX) $(CXXFLAGS) -O3 -I$(wxIPath) -I$(wxIMSWU) $(defines) $(wxLib) -c src/Cores/SCC68070/Interpreter.cpp -o bin/obj/Interpreter.o
	$(CXX) $(CXXFLAGS) -O3 -I$(wxIPath) -I$(wxIMSWU) $(defines) $(wxLib) -c src/Cores/SCC68070/MemoryAccess.cpp -o bin/obj/MemoryAccess.o
	$(CXX) $(CXXFLAGS) -O3 -I$(wxIPath) -I$(wxIMSWU) $(defines) $(wxLib) -c src/Cores/SCC68070/InstructionSet.cpp -o bin/obj/InstructionSet.o
	$(CXX) $(CXXFLAGS) -O3 -I$(wxIPath) -I$(wxIMSWU) $(defines) $(wxLib) -c src/Cores/SCC68070/AddressingModes.cpp -o bin/obj/AddressingModes.o
	$(CXX) $(CXXFLAGS) -O3 -I$(wxIPath) -I$(wxIMSWU) $(defines) $(wxLib) -c src/Cores/SCC68070/ConditionalTests.cpp -o bin/obj/ConditionalTests.o
	$(CXX) $(CXXFLAGS) -O3 -I$(wxIPath) -I$(wxIMSWU) $(defines) $(wxLib) -c src/Cores/SCC66470/DRAMInterface.cpp -o bin/obj/DRAMInterface.o
	$(CXX) $(CXXFLAGS) -O3 -I$(wxIPath) -I$(wxIMSWU) $(defines) $(wxLib) -c src/Cores/SCC66470/SCC66470.cpp -o bin/obj/SCC66470.o
	$(CXX) $(CXXFLAGS) -O3 -I$(wxIPath) -I$(wxIMSWU) $(defines) $(wxLib) -c src/GUI/MainFrame.cpp -o bin/obj/MainFrame.o
	$(CXX) $(CXXFLAGS) -O3 -I$(wxIPath) -I$(wxIMSWU) $(defines) $(wxLib) -c src/GUI/GamePanel.cpp -o bin/obj/GamePanel.o
	$(CXX) $(CXXFLAGS) -O3 -I$(wxIPath) -I$(wxIMSWU) $(defines) $(wxLib) -c src/GUI/RAMWatchFrame.cpp -o bin/obj/RAMWatchFrame.o
	$(CXX) $(CXXFLAGS) -O3 -I$(wxIPath) -I$(wxIMSWU) $(defines) $(wxLib) -c src/GUI/DisassemblerFrame.cpp -o bin/obj/DisassemblerFrame.o
	$(CXX) $(CXXFLAGS) -O3 -I$(wxIPath) -I$(wxIMSWU) $(defines) $(wxLib) -c src/CeDImu.cpp -o bin/obj/CeDImu.o
	$(CXX) $(CXXFLAGS) -O3 -I$(wxIPath) -I$(wxIMSWU) $(defines) $(wxLib) -c src/Main.cpp -o bin/obj/Main.o
	$(CXX) -L$(wxLibPath) -o bin/CeDImu.exe $(obj) bin/obj/ressource.res $(linkerFLAGS) $(wxLib)

