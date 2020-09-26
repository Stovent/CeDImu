#include "SCC68070.hpp"

#include <algorithm>
#include <chrono>

#include "../../Boards/Board.hpp"
#include "../../utils.hpp"

void SCC68070::Interpreter()
{
    isRunning = true;
    std::chrono::time_point<std::chrono::steady_clock, std::chrono::duration<long double, std::nano>> start = std::chrono::steady_clock::now();

    do
    {
        if(cycleCount == 0)
            FlushDisassembler();

        uint16_t executionCycles = 0;
        try {
            currentPC = PC;
            currentOpcode = GetNextWord(Trigger);
            executionCycles += (this->*ILUT[currentOpcode])();

            if(disassemble)
                disassembledInstructions.push_back((this->*DLUT[currentOpcode])(currentPC));
        }
        catch(const SCC68070Exception& e) {
            Exception(e.vector);
        }

        cycleCount += executionCycles;
        totalCycleCount += executionCycles;
        board->slave->Execute(executionCycles);

        if(cycleCount * cycleDelay >= board->GetLineDisplayTime())
        {
            board->DrawLine();
            cycleCount = 0;
        }

        start += std::chrono::duration<long double, std::nano>(executionCycles * cycleDelay);
        std::this_thread::sleep_until(start);
    } while(loop);

    isRunning = false;
}
