#include "CPU.hpp"

namespace casioemu
{
	CPU::CPU(Emulator &_emulator) : emulator(_emulator), reg_lr(reg_elr[0]), reg_lcsr(reg_ecsr[0]), reg_psw(reg_epsw[0])
	{
	}
}

