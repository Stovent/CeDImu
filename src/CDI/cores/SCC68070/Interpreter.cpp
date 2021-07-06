#include "SCC68070.hpp"
#include "../../CDI.hpp"
#include "../../common/utils.hpp"

#include <algorithm>
#include <chrono>

void SCC68070::Interpreter()
{
    isRunning = true;
    std::chrono::time_point<std::chrono::steady_clock, std::chrono::duration<double, std::nano>> start = std::chrono::steady_clock::now();

    do
    {
        size_t executionCycles = 0;

        if(exceptions.size())
        {
            const SCC68070Exception exception = exceptions.top();
            exceptions.pop();
            if(cdi.callbacks.HasOnLogDisassembler())
            {
                const std::string str = "Exception vector " + std::to_string(exception.vector) + ": " + DisassembleException(exception);
                cdi.callbacks.OnLogDisassembler({0, "", str});
            }
//            DumpCPURegisters();
            executionCycles += Exception(exception.vector);
        }

        if(stop)
        {
            executionCycles += 25;
        }
        else
        {
            try {
                currentPC = PC;
                currentOpcode = GetNextWord(Trigger);
                if(cdi.callbacks.HasOnLogDisassembler())
                {
                    const Instruction inst = {currentPC, cdi.board->GetBIOS().GetModuleNameAt(currentPC - cdi.board->GetBIOS().base), (this->*DLUT[currentOpcode])(currentPC)};
                    cdi.callbacks.OnLogDisassembler(inst);
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
        cdi.board->slave->IncrementTime(ns);
        cdi.board->timekeeper->IncrementClock(ns);

        const uint32_t lineDisplayTime = cdi.board->GetLineDisplayTime();
        if(cycleCount * cycleDelay >= lineDisplayTime)
        {
            cdi.board->ExecuteVideoLine();
            cycleCount -= lineDisplayTime / cycleDelay;
        }

        if(find(breakpoints.begin(), breakpoints.end(), currentPC) != breakpoints.end())
            loop = false;

        start += std::chrono::duration<double, std::nano>(executionCycles * speedDelay);
        std::this_thread::sleep_until(start);
    } while(loop);

    isRunning = false;
}
