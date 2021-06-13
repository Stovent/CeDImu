#include "SCC68070.hpp"
#include "../../boards/Board.hpp"
#include "../../common/utils.hpp"

#include <algorithm>
#include <chrono>

void SCC68070::Interpreter()
{
    isRunning = true;
    std::chrono::time_point<std::chrono::steady_clock, std::chrono::duration<double, std::nano>> start = std::chrono::steady_clock::now();

    do
    {
        uint16_t executionCycles = 0;

        if(exceptions.size())
        {
            const SCC68070Exception exception = exceptions.top();
            exceptions.pop();
            if(OnDisassembler)
            {
                const std::string str = "Exception vector " + std::to_string(exception.vector) + ": " + DisassembleException(exception);
                OnDisassembler({0, "", str});
            }
//            DumpCPURegisters();
            executionCycles += Exception(exception.vector);
        }

        if(stop)
        {
            executionCycles = 25;
        }
        else
        {
            try {
                currentPC = PC;
                currentOpcode = GetNextWord(Trigger);
                if(OnDisassembler)
                {
                    const Instruction inst = {currentPC, board.GetBIOS().GetModuleNameAt(currentPC - board.GetBIOS().base), (this->*DLUT[currentOpcode])(currentPC)};
                    OnDisassembler(inst);
                }
                executionCycles += (this->*ILUT[currentOpcode])();
            }
            catch(const SCC68070Exception& e) {
                exceptions.push(e);
            }
        }

        cycleCount += executionCycles;
        totalCycleCount += executionCycles;

        const double ns = executionCycles * cycleDelay;
        IncrementTimer(ns);
        board.slave->IncrementTime(ns);
        board.timekeeper.IncrementClock(ns);

        const uint32_t lineDisplayTime = board.GetLineDisplayTime();
        if(cycleCount * cycleDelay >= lineDisplayTime)
        {
            board.ExecuteVideoLine();
            cycleCount -= lineDisplayTime / cycleDelay;
        }

        if(find(breakpoints.begin(), breakpoints.end(), currentPC) != breakpoints.end())
            loop = false;

        start += std::chrono::duration<double, std::nano>(executionCycles * speedDelay);
        std::this_thread::sleep_until(start);
    } while(loop);

    isRunning = false;
}
