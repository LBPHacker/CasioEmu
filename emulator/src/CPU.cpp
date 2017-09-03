#include "CPU.hpp"

#include "Emulator.hpp"
#include "Chipset.hpp"
#include "MMU.hpp"
#include "Logger.hpp"

namespace casioemu
{
	uint16_t CPU::Fetch()
	{
		if (reg_pc & 1)
		{
			logger::Info("warning: PC LSB set\n");
			reg_pc &= ~1;
		}
		uint16_t opcode = emulator.chipset.mmu.ReadCode(((size_t)reg_csr) << 16 | reg_pc, 2);
		reg_pc += 2;
		return opcode;
	}

	void CPU::Next()
	{
		/**
		 * `reg_dsr` only affects the current instruction. The old DSR is stored in
		 * `impl_last_dsr` and is recalled every time a DSR instruction is encountered
		 * that activates DSR addressing without actually changing DSR.
		 */
		reg_dsr	= 0;

		while (1)
		{
			uint16_t opcode = Fetch();
			OpcodeSource *handler = opcode_dispatch[opcode];

			if (!handler)
				PANIC("unrecognized instruction %04X at %06zX\n", opcode, ((size_t)reg_csr) << 16 | (reg_pc - 2));

			impl_long_imm = 0;
			if (handler->hint & H_TI)
				impl_long_imm = Fetch();

			for (size_t ix = 0; ix != sizeof(impl_operands) / sizeof(impl_operands[0]); ++ix)
			{
				impl_operands[ix].value = (opcode & handler->operands[ix].mask) << handler->operands[ix].shift;
				impl_operands[ix].register_index = impl_operands[ix].value;
				impl_operands[ix].register_size = handler->operands[ix].register_size;
				if (impl_operands[ix].register_size)
					;// * TODO: read reg[.register_index] into .value
			}
			impl_hint = handler->hint;

			(this->*(handler->handler_function))();

			if (!(handler->hint & H_DS))
				break;
		}
	}

	CPU::OpcodeSource CPU::opcode_sources[] = {
		//           function,                            hints, main mask, operand {size, mask, shift} x2
		// * Arithmetic Instructions
		{&CPU::OP_ADD        ,                                0, 0x8001, {{1, 0x000F,  8}, {1, 0x000F,  4}}},
		{&CPU::OP_ADD        ,                                0, 0x1000, {{1, 0x000F,  8}, {0, 0x00FF,  0}}},
		{&CPU::OP_ADD16      ,                                0, 0xF006, {{2, 0x000E,  8}, {2, 0x000E,  4}}},
		{&CPU::OP_ADD16      ,                      H_IE       , 0xE080, {{2, 0x000E,  8}, {0, 0x007F,  0}}},
		{&CPU::OP_ADDC       ,                                0, 0x8006, {{1, 0x000F,  8}, {1, 0x000F,  4}}},
		{&CPU::OP_ADDC       ,                                0, 0x6000, {{1, 0x000F,  8}, {0, 0x00FF,  0}}},
		{&CPU::OP_AND        ,                                0, 0x8002, {{1, 0x000F,  8}, {1, 0x000F,  4}}},
		{&CPU::OP_AND        ,                                0, 0x2000, {{1, 0x000F,  8}, {0, 0x00FF,  0}}},
		{&CPU::OP_CMP        ,                                0, 0x8007, {{1, 0x000F,  8}, {1, 0x000F,  4}}},
		{&CPU::OP_CMP        ,                                0, 0x7000, {{1, 0x000F,  8}, {0, 0x00FF,  0}}},
		{&CPU::OP_CMPC       ,                                0, 0x8005, {{1, 0x000F,  8}, {1, 0x000F,  4}}},
		{&CPU::OP_CMPC       ,                                0, 0x5000, {{1, 0x000F,  8}, {0, 0x00FF,  0}}},
		{&CPU::OP_MOV16      ,                                0, 0xF006, {{2, 0x000E,  8}, {2, 0x000E,  4}}},
		{&CPU::OP_MOV16      ,                      H_IE       , 0xE000, {{2, 0x000E,  8}, {0, 0x007F,  0}}},
		{&CPU::OP_MOV        ,                                0, 0x8000, {{1, 0x000F,  8}, {1, 0x000F,  4}}},
		{&CPU::OP_MOV        ,                                0, 0x0000, {{1, 0x000F,  8}, {0, 0x00FF,  0}}},
		{&CPU::OP_OR         ,                                0, 0x8003, {{1, 0x000F,  8}, {1, 0x000F,  4}}},
		{&CPU::OP_OR         ,                                0, 0x3000, {{1, 0x000F,  8}, {0, 0x00FF,  0}}},
		{&CPU::OP_XOR        ,                                0, 0x8004, {{1, 0x000F,  8}, {1, 0x000F,  4}}},
		{&CPU::OP_XOR        ,                                0, 0x4000, {{1, 0x000F,  8}, {0, 0x00FF,  0}}},
		{&CPU::OP_CMP16      ,                                0, 0xF007, {{2, 0x000E,  8}, {2, 0x000E,  4}}},
		{&CPU::OP_SUB        ,                                0, 0x8008, {{1, 0x000F,  8}, {1, 0x000F,  4}}},
		{&CPU::OP_SUBC       ,                                0, 0x8009, {{1, 0x000F,  8}, {1, 0x000F,  4}}},
		// * Shift Instructions
		{&CPU::OP_SLL        ,                                0, 0x800A, {{1, 0x000F,  8}, {1, 0x000F,  4}}},
		{&CPU::OP_SLL        ,                                0, 0x900A, {{1, 0x000F,  8}, {0, 0x0007,  4}}},
		{&CPU::OP_SLLC       ,                                0, 0x800B, {{1, 0x000F,  8}, {1, 0x000F,  4}}},
		{&CPU::OP_SLLC       ,                                0, 0x900B, {{1, 0x000F,  8}, {0, 0x0007,  4}}},
		{&CPU::OP_SRA        ,                                0, 0x800E, {{1, 0x000F,  8}, {1, 0x000F,  4}}},
		{&CPU::OP_SRA        ,                                0, 0x900E, {{1, 0x000F,  8}, {0, 0x0007,  4}}},
		{&CPU::OP_SRL        ,                                0, 0x800C, {{1, 0x000F,  8}, {1, 0x000F,  4}}},
		{&CPU::OP_SRL        ,                                0, 0x900C, {{1, 0x000F,  8}, {0, 0x0007,  4}}},
		{&CPU::OP_SRLC       ,                                0, 0x800D, {{1, 0x000F,  8}, {1, 0x000F,  4}}},
		{&CPU::OP_SRLC       ,                                0, 0x900D, {{1, 0x000F,  8}, {0, 0x0007,  4}}},
		// * Load/Store Instructions
		{&CPU::OP_LS_EA      ,                                0, 0x9032, {{2, 0x000E,  8}, {0,      0,  0}}},
		{&CPU::OP_LS_EA      ,               H_IA              , 0x9052, {{2, 0x000E,  8}, {0,      0,  0}}},
		{&CPU::OP_LS_R       ,                                0, 0x9002, {{2, 0x000E,  8}, {2, 0x000E,  4}}},
		{&CPU::OP_LS_I_R     ,                             H_TI, 0xA008, {{2, 0x000E,  8}, {2, 0x000E,  4}}},
		{&CPU::OP_LS_I_BP    ,                             H_TI, 0xB000, {{2, 0x000E,  8}, {0, 0x003F,  6}}},
		{&CPU::OP_LS_I_FP    ,                             H_TI, 0xB040, {{2, 0x000E,  8}, {0, 0x003F,  6}}},
		{&CPU::OP_LS_I       ,                             H_TI, 0x9012, {{2, 0x000E,  8}, {0,      0,  6}}},
		{&CPU::OP_LS_EA      ,                                0, 0x9030, {{2, 0x000E,  8}, {0,      0,  0}}},
		{&CPU::OP_LS_EA      ,               H_IA              , 0x9050, {{2, 0x000E,  8}, {0,      0,  0}}},
		{&CPU::OP_LS_R       ,                                0, 0x9000, {{2, 0x000E,  8}, {2, 0x000E,  4}}},
		{&CPU::OP_LS_I_R     ,                             H_TI, 0x9008, {{2, 0x000E,  8}, {2, 0x000E,  4}}},
		{&CPU::OP_LS_I_BP    ,                             H_TI, 0xD000, {{2, 0x000E,  8}, {0, 0x003F,  6}}},
		{&CPU::OP_LS_I_FP    ,                             H_TI, 0xD040, {{2, 0x000E,  8}, {0, 0x003F,  6}}},
		{&CPU::OP_LS_I       ,                             H_TI, 0x9010, {{2, 0x000E,  8}, {0,      0,  6}}},
		{&CPU::OP_LS_EA      ,                                0, 0x9034, {{4, 0x000C,  8}, {0,      0,  0}}},
		{&CPU::OP_LS_EA      ,               H_IA              , 0x9054, {{4, 0x000C,  8}, {0,      0,  0}}},
		{&CPU::OP_LS_EA      ,                                0, 0x9036, {{8, 0x0008,  8}, {0,      0,  0}}},
		{&CPU::OP_LS_EA      ,               H_IA              , 0x9056, {{8, 0x0008,  8}, {0,      0,  0}}},
		{&CPU::OP_LS_EA      ,                      H_ST       , 0x9033, {{2, 0x000E,  8}, {0,      0,  0}}},
		{&CPU::OP_LS_EA      ,               H_IA | H_ST       , 0x9053, {{2, 0x000E,  8}, {0,      0,  0}}},
		{&CPU::OP_LS_R       ,                      H_ST       , 0x9003, {{2, 0x000E,  8}, {2, 0x000E,  4}}},
		{&CPU::OP_LS_I_R     ,                      H_ST | H_TI, 0xA009, {{2, 0x000E,  8}, {2, 0x000E,  4}}},
		{&CPU::OP_LS_I_BP    ,                      H_ST | H_TI, 0xB080, {{2, 0x000E,  8}, {0, 0x003F,  6}}},
		{&CPU::OP_LS_I_FP    ,                      H_ST | H_TI, 0xB0C0, {{2, 0x000E,  8}, {0, 0x003F,  6}}},
		{&CPU::OP_LS_I       ,                      H_ST | H_TI, 0x9013, {{2, 0x000E,  8}, {0,      0,  6}}},
		{&CPU::OP_LS_EA      ,                      H_ST       , 0x9031, {{2, 0x000E,  8}, {0,      0,  0}}},
		{&CPU::OP_LS_EA      ,               H_IA | H_ST       , 0x9051, {{2, 0x000E,  8}, {0,      0,  0}}},
		{&CPU::OP_LS_R       ,                      H_ST       , 0x9001, {{2, 0x000E,  8}, {2, 0x000E,  4}}},
		{&CPU::OP_LS_I_R     ,                      H_ST | H_TI, 0x9009, {{2, 0x000E,  8}, {2, 0x000E,  4}}},
		{&CPU::OP_LS_I_BP    ,                      H_ST | H_TI, 0xD080, {{2, 0x000E,  8}, {0, 0x003F,  6}}},
		{&CPU::OP_LS_I_FP    ,                      H_ST | H_TI, 0xD0C0, {{2, 0x000E,  8}, {0, 0x003F,  6}}},
		{&CPU::OP_LS_I       ,                      H_ST | H_TI, 0x9011, {{2, 0x000E,  8}, {0,      0,  6}}},
		{&CPU::OP_LS_EA      ,                      H_ST       , 0x9035, {{4, 0x000C,  8}, {0,      0,  0}}},
		{&CPU::OP_LS_EA      ,               H_IA | H_ST       , 0x9055, {{4, 0x000C,  8}, {0,      0,  0}}},
		{&CPU::OP_LS_EA      ,                      H_ST       , 0x9037, {{8, 0x0008,  8}, {0,      0,  0}}},
		{&CPU::OP_LS_EA      ,               H_IA | H_ST       , 0x9057, {{8, 0x0008,  8}, {0,      0,  0}}},
		// * Control Register Access Instructions
		{&CPU::OP_ADDSP      ,                                0, 0xE100, {{0, 0x00FF,  0}, {0,      0,  0}}},
		{&CPU::OP_CTRL       ,                                0, 0xA00F, {{0,      0,  0}, {1, 0x000F,  4}}},
		{&CPU::OP_CTRL       ,                                0, 0xA00D, {{0,      0,  0}, {2, 0x000E,  8}}},
		{&CPU::OP_CTRL       ,                                0, 0xA00C, {{0,      0,  0}, {1, 0x000F,  4}}},
		{&CPU::OP_CTRL       ,                                0, 0xA005, {{2, 0x000E,  8}, {0,      0,  0}}},
		{&CPU::OP_CTRL       ,                                0, 0xA01A, {{2, 0x000E,  8}, {0,      0,  0}}},
		{&CPU::OP_CTRL       ,                                0, 0xA00B, {{0,      0,  0}, {1, 0x000F,  4}}},
		{&CPU::OP_CTRL       ,                                0, 0xE900, {{0,      0,  0}, {0, 0x00FF,  0}}},
		{&CPU::OP_CTRL       ,                                0, 0xA007, {{1, 0x000F,  8}, {0,      0,  0}}},
		{&CPU::OP_CTRL       ,                                0, 0xA004, {{1, 0x000F,  8}, {0,      0,  0}}},
		{&CPU::OP_CTRL       ,                                0, 0xA003, {{1, 0x000F,  8}, {0,      0,  0}}},
		{&CPU::OP_CTRL       ,                                0, 0xA10A, {{0,      0,  0}, {2, 0x000E,  4}}},
		// * PUSH/POP Instructions
		{&CPU::OP_PUSH       ,                                0, 0xF05E, {{0,      0,  0}, {2, 0x000E,  8}}},
		{&CPU::OP_PUSH       ,                                0, 0xF07E, {{0,      0,  0}, {8, 0x0008,  8}}},
		{&CPU::OP_PUSH       ,                                0, 0xF04E, {{0,      0,  0}, {1, 0x000F,  8}}},
		{&CPU::OP_PUSH       ,                                0, 0xF06E, {{0,      0,  0}, {4, 0x000C,  8}}},
		{&CPU::OP_PUSH2      ,                                0, 0xF0CE, {{0,      0,  0}, {0, 0x000F,  8}}},
		{&CPU::OP_POP        ,                                0, 0xF01E, {{2, 0x000E,  8}, {0,      0,  0}}},
		{&CPU::OP_POP        ,                                0, 0xF03E, {{8, 0x0008,  8}, {0,      0,  0}}},
		{&CPU::OP_POP        ,                                0, 0xF00E, {{1, 0x000F,  8}, {0,      0,  0}}},
		{&CPU::OP_POP        ,                                0, 0xF02E, {{4, 0x000C,  8}, {0,      0,  0}}},
		{&CPU::OP_POP2       ,                                0, 0xF08E, {{0, 0x000F,  8}, {0,      0,  0}}},
		// * Coprocessor Data Transfer Instructions
		{&CPU::OP_CR_R       ,                                0, 0xA00E, {{0, 0x000F,  8}, {1, 0x000F,  4}}},
		{&CPU::OP_CR_EA      ,                                0, 0xF02D, {{0, 0x000E,  8}, {0,      0,  0}}},
		{&CPU::OP_CR_EA      ,               H_IA              , 0xF03D, {{0, 0x000E,  8}, {0,      0,  0}}},
		{&CPU::OP_CR_EA      ,                                0, 0xF00D, {{0, 0x000F,  8}, {0,      0,  0}}},
		{&CPU::OP_CR_EA      ,               H_IA              , 0xF01D, {{0, 0x000F,  8}, {0,      0,  0}}},
		{&CPU::OP_CR_EA      ,                                0, 0xF04D, {{0, 0x000C,  8}, {0,      0,  0}}},
		{&CPU::OP_CR_EA      ,               H_IA              , 0xF05D, {{0, 0x000C,  8}, {0,      0,  0}}},
		{&CPU::OP_CR_EA      ,                                0, 0xF06D, {{0, 0x0008,  8}, {0,      0,  0}}},
		{&CPU::OP_CR_EA      ,               H_IA              , 0xF07D, {{0, 0x0008,  8}, {0,      0,  0}}},
		{&CPU::OP_CR_R       ,                      H_ST       , 0xA006, {{1, 0x000F,  8}, {0, 0x000F,  4}}},
		{&CPU::OP_CR_EA      ,                      H_ST       , 0xF0AD, {{0, 0x000E,  8}, {0,      0,  0}}},
		{&CPU::OP_CR_EA      ,               H_IA | H_ST       , 0xF0BD, {{0, 0x000E,  8}, {0,      0,  0}}},
		{&CPU::OP_CR_EA      ,                      H_ST       , 0xF08D, {{0, 0x000F,  8}, {0,      0,  0}}},
		{&CPU::OP_CR_EA      ,               H_IA | H_ST       , 0xF09D, {{0, 0x000F,  8}, {0,      0,  0}}},
		{&CPU::OP_CR_EA      ,                      H_ST       , 0xF0CD, {{0, 0x000C,  8}, {0,      0,  0}}},
		{&CPU::OP_CR_EA      ,               H_IA | H_ST       , 0xF0DD, {{0, 0x000C,  8}, {0,      0,  0}}},
		{&CPU::OP_CR_EA      ,                      H_ST       , 0xF0ED, {{0, 0x0008,  8}, {0,      0,  0}}},
		{&CPU::OP_CR_EA      ,               H_IA | H_ST       , 0xF0FD, {{0, 0x0008,  8}, {0,      0,  0}}},
		// * EA Register Data Transfer Instructions
		{&CPU::OP_LEA_R      ,                                0, 0xF00A, {{0,      0,  0}, {2, 0x000E,  4}}},
		{&CPU::OP_LEA_I_R    ,                             H_TI, 0xF00B, {{0,      0,  0}, {2, 0x000E,  4}}},
		{&CPU::OP_LEA_I      ,                             H_TI, 0xF00C, {{0,      0,  0}, {0,      0,  0}}},
		// * ALU Instructions
		{&CPU::OP_DAA        ,                                0, 0x801F, {{1, 0x000F,  8}, {0,      0,  0}}},
		{&CPU::OP_DAS        ,                                0, 0x803F, {{1, 0x000F,  8}, {0,      0,  0}}},
		{&CPU::OP_NEG        ,                                0, 0x805F, {{1, 0x000F,  8}, {0,      0,  0}}},
		// * Bit Access Instructions
		{&CPU::OP_BITMOD     ,                                0, 0xA000, {{1, 0x000F,  8}, {0, 0x0007,  4}}},
		{&CPU::OP_BITMOD     ,                             H_TI, 0xA080, {{0,      0,  0}, {0, 0x0007,  4}}},
		{&CPU::OP_BITMOD     ,                                0, 0xA002, {{1, 0x000F,  8}, {0, 0x0007,  4}}},
		{&CPU::OP_BITMOD     ,                             H_TI, 0xA082, {{0,      0,  0}, {0, 0x0007,  4}}},
		{&CPU::OP_BITMOD     ,                                0, 0xA001, {{1, 0x000F,  8}, {0, 0x0007,  4}}},
		{&CPU::OP_BITMOD     ,                             H_TI, 0xA081, {{0,      0,  0}, {0, 0x0007,  4}}},
		// * PSW Access Instructions
		{&CPU::OP_PSW_OR     ,                                0, 0xED08, {{0,      0,  0}, {0,      0,  0}}},
		{&CPU::OP_PSW_AND    ,                                0, 0xEBF7, {{0,      0,  0}, {0,      0,  0}}},
		{&CPU::OP_PSW_OR     ,                                0, 0xED80, {{0,      0,  0}, {0,      0,  0}}},
		{&CPU::OP_PSW_AND    ,                                0, 0xEB7F, {{0,      0,  0}, {0,      0,  0}}},
		{&CPU::OP_CPLC       ,                                0, 0xFECF, {{0,      0,  0}, {0,      0,  0}}},
		// * Conditional Relative Branch Instructions
		{&CPU::OP_BC         ,                                0, 0xC000, {{0, 0x00FF,  0}, {0,      0,  0}}},
		{&CPU::OP_BC         ,                                0, 0xC100, {{0, 0x00FF,  0}, {0,      0,  0}}},
		{&CPU::OP_BC         ,                                0, 0xC200, {{0, 0x00FF,  0}, {0,      0,  0}}},
		{&CPU::OP_BC         ,                                0, 0xC300, {{0, 0x00FF,  0}, {0,      0,  0}}},
		{&CPU::OP_BC         ,                                0, 0xC400, {{0, 0x00FF,  0}, {0,      0,  0}}},
		{&CPU::OP_BC         ,                                0, 0xC500, {{0, 0x00FF,  0}, {0,      0,  0}}},
		{&CPU::OP_BC         ,                                0, 0xC600, {{0, 0x00FF,  0}, {0,      0,  0}}},
		{&CPU::OP_BC         ,                                0, 0xC700, {{0, 0x00FF,  0}, {0,      0,  0}}},
		{&CPU::OP_BC         ,                                0, 0xC800, {{0, 0x00FF,  0}, {0,      0,  0}}},
		{&CPU::OP_BC         ,                                0, 0xC900, {{0, 0x00FF,  0}, {0,      0,  0}}},
		{&CPU::OP_BC         ,                                0, 0xCA00, {{0, 0x00FF,  0}, {0,      0,  0}}},
		{&CPU::OP_BC         ,                                0, 0xCB00, {{0, 0x00FF,  0}, {0,      0,  0}}},
		{&CPU::OP_BC         ,                                0, 0xCC00, {{0, 0x00FF,  0}, {0,      0,  0}}},
		{&CPU::OP_BC         ,                                0, 0xCD00, {{0, 0x00FF,  0}, {0,      0,  0}}},
		{&CPU::OP_BC         ,                                0, 0xCE00, {{0, 0x00FF,  0}, {0,      0,  0}}},
		// * Sign Extension Instruction
		{&CPU::OP_EXTBW      ,                                0, 0x810F, {{0,      0,  0}, {0,      0,  0}}},
		{&CPU::OP_EXTBW      ,                                0, 0x832F, {{0,      0,  0}, {0,      0,  0}}},
		{&CPU::OP_EXTBW      ,                                0, 0x854F, {{0,      0,  0}, {0,      0,  0}}},
		{&CPU::OP_EXTBW      ,                                0, 0x876F, {{0,      0,  0}, {0,      0,  0}}},
		{&CPU::OP_EXTBW      ,                                0, 0x898F, {{0,      0,  0}, {0,      0,  0}}},
		{&CPU::OP_EXTBW      ,                                0, 0x8BAF, {{0,      0,  0}, {0,      0,  0}}},
		{&CPU::OP_EXTBW      ,                                0, 0x8DCF, {{0,      0,  0}, {0,      0,  0}}},
		{&CPU::OP_EXTBW      ,                                0, 0x8FEF, {{0,      0,  0}, {0,      0,  0}}},
		// * Software Interrupt Instructions
		{&CPU::OP_SWI        ,                                0, 0xE500, {{0, 0x003F,  0}, {0,      0,  0}}},
		{&CPU::OP_BRK        ,                                0, 0xFFFF, {{0,      0,  0}, {0,      0,  0}}},
		// * Branch Instructions
		{&CPU::OP_B          ,                             H_TI, 0xF000, {{0,      0,  0}, {0, 0x000F,  8}}},
		{&CPU::OP_B          ,                                0, 0xF002, {{0,      0,  0}, {2, 0x000E,  4}}},
		{&CPU::OP_BL         ,                             H_TI, 0xF001, {{0,      0,  0}, {0, 0x000F,  8}}},
		{&CPU::OP_BL         ,                                0, 0xF003, {{0,      0,  0}, {2, 0x000E,  4}}},
		// * Multiplication and Division Instructions
		{&CPU::OP_MUL        ,                                0, 0xF004, {{2, 0x000E,  8}, {1, 0x000F,  4}}},
		{&CPU::OP_DIV        ,                                0, 0xF009, {{2, 0x000E,  8}, {1, 0x000F,  4}}},
		// * Miscellaneous Instructions
		{&CPU::OP_INC_EA     ,                                0, 0xFE2F, {{0,      0,  0}, {0,      0,  0}}},
		{&CPU::OP_DEC_EA     ,                                0, 0xFE3F, {{0,      0,  0}, {0,      0,  0}}},
		{&CPU::OP_RT         ,                                0, 0xFE1F, {{0,      0,  0}, {0,      0,  0}}},
		{&CPU::OP_RTI        ,                                0, 0xFE0F, {{0,      0,  0}, {0,      0,  0}}},
		{&CPU::OP_NOP        ,                                0, 0xFE8F, {{0,      0,  0}, {0,      0,  0}}},
		{&CPU::OP_DSR        ,                             H_DS, 0xFE9F, {{0,      0,  0}, {0,      0,  0}}},
		{&CPU::OP_DSR        ,                      H_DW | H_DS, 0xE300, {{0, 0x00FF,  0}, {0,      0,  0}}},
		{&CPU::OP_DSR        ,                      H_DW | H_DS, 0x900F, {{1, 0x000F,  4}, {0,      0,  0}}}
	};

	void CPU::OP_NOP()
	{
		/**
		 * NOP
		 * Literally do nothing.
		 */
	}

	void CPU::OP_DSR()
	{
		/**
		 * OP_DSR
		 * Recall `impl_last_dsr` into `reg_dsr`.
		 *
		 * Hint 1: Store an immediate DSR value into `impl_last_dsr` to be recalled by OP_DSR.
		 */

		if (impl_hint & H_DW)
			impl_last_dsr = impl_operands[0].value;

		reg_dsr = impl_last_dsr;
	}

	CPU::CPU(Emulator &_emulator) : emulator(_emulator), reg_lr(reg_elr[0]), reg_lcsr(reg_ecsr[0]), reg_psw(reg_epsw[0])
	{
		opcode_dispatch = new OpcodeSource *[0x10000];
		for (size_t ix = 0; ix != 0x10000; ++ix)
			opcode_dispatch[ix] = nullptr;

		uint16_t *permutation_buffer = new uint16_t[0x10000];
		for (size_t ix = 0; ix != sizeof(opcode_sources) / sizeof(opcode_sources[0]); ++ix)
		{
			OpcodeSource &handler_stub = opcode_sources[ix];

			uint16_t varying_bits = 0;
			for (size_t ox = 0; ox != sizeof(impl_operands) / sizeof(impl_operands[0]); ++ox)
				varying_bits |= handler_stub.operands[ox].mask << handler_stub.operands[ox].shift;

			size_t permutation_count = 1;
			permutation_buffer[0] = handler_stub.opcode;
			for (uint16_t checkbit = 0x8000; checkbit; checkbit >>= 1)
			{
				if (varying_bits & checkbit)
				{
					for (size_t px = 0; px != permutation_count; ++px)
						permutation_buffer[px + permutation_count] = permutation_buffer[px] | checkbit;
					permutation_count <<= 1;
				}
			}

			for (size_t px = 0; px != permutation_count; ++px)
				opcode_dispatch[permutation_buffer[px]] = &handler_stub;
		}
		delete[] permutation_buffer;
	}

	CPU::~CPU()
	{
		delete[] opcode_dispatch;
	}
}

