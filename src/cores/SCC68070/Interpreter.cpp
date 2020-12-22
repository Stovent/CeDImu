#include "SCC68070.hpp"
#include "../../utils.hpp"
#include "../../Boards/Board.hpp"

#include <algorithm>
#include <chrono>

void SCC68070::Interpreter()
{
    isRunning = true;
    std::chrono::time_point<std::chrono::steady_clock, std::chrono::duration<double, std::nano>> start = std::chrono::steady_clock::now();

    do
    {
        if(cycleCount == 0)
            FlushDisassembler();

        uint16_t executionCycles = 0;

        if(exceptions.size())
        {
            uint8_t vector = exceptions.top().vector;
            exceptions.pop();
            executionCycles += Exception(vector);
        }

        try {
            currentPC = PC;
            currentOpcode = GetNextWord(Trigger);
            if(disassemble)
                disassembledInstructions.push_back((this->*DLUT[currentOpcode])(currentPC));
            executionCycles += (this->*ILUT[currentOpcode])();
        }
        catch(const SCC68070Exception& e) {
            exceptions.push(e);
        }

        board->slave.Execute(executionCycles);
        board->timekeeper.IncrementClock(executionCycles * cycleDelay);

        cycleCount += executionCycles;
        totalCycleCount += executionCycles;

        const uint32_t lineDisplayTime = board->GetLineDisplayTime();
        if(cycleCount * cycleDelay >= lineDisplayTime)
        {
            board->DrawLine();
            cycleCount -= lineDisplayTime / cycleDelay;
        }

        if(find(breakpoints.begin(), breakpoints.end(), currentPC) != breakpoints.end())
            loop = false;

        start += std::chrono::duration<double, std::nano>(executionCycles * speedDelay);
        std::this_thread::sleep_until(start);
    } while(loop);

    isRunning = false;
}
