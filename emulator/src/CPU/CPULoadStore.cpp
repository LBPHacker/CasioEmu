#include "../CPU.hpp"

#include "../Emulator.hpp"
#include "../Chipset.hpp"
#include "../MMU.hpp"

namespace casioemu
{
	// * Load/Store Instructions
	void CPU::OP_LS_EA()
	{
		LoadStore(reg_ea, impl_hint >> 8);
	}

	void CPU::OP_LS_R()
	{
		LoadStore(impl_operands[1].value, impl_hint >> 8);
	}

	void CPU::OP_LS_I_R()
	{
		LoadStore(impl_operands[1].value + impl_long_imm, impl_hint >> 8);
	}

	void CPU::OP_LS_BP()
	{
		impl_operands[1].value |= (impl_operands[1].value & 0x20) ? 0xFF40 : 0;
		impl_operands[1].value += reg_r[12] | (((uint16_t)reg_r[13]) << 8);
		LoadStore(impl_operands[1].value, impl_hint >> 8);
	}

	void CPU::OP_LS_FP()
	{
		impl_operands[1].value |= (impl_operands[1].value & 0x20) ? 0xFF40 : 0;
		impl_operands[1].value += reg_r[14] | (((uint16_t)reg_r[15]) << 8);
		LoadStore(impl_operands[1].value, impl_hint >> 8);
	}

	void CPU::OP_LS_I()
	{
		LoadStore(impl_long_imm, impl_hint >> 8);
	}

	void CPU::LoadStore(uint16_t offset, size_t length)
	{
		if (impl_hint & H_ST)
			for (size_t ix = 0; ix != length; ++ix)
				emulator.chipset.mmu.WriteData((((size_t)reg_dsr) << 16) | (offset + ix), reg_r[impl_operands[0].value + ix]);
		else
			for (size_t ix = 0; ix != length; ++ix)
				reg_r[impl_operands[0].value + ix] = emulator.chipset.mmu.ReadData((((size_t)reg_dsr) << 16) | (offset + ix));

		if (impl_hint & H_IA)
		{
			reg_ea += length;
			if (length != 1)
				reg_ea &= ~1;
		}
	}
}

