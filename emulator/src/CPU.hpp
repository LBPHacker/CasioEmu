#pragma once

#include "Config.hpp"

#include <cstdint>

namespace casioemu
{
	class Emulator;

	class CPU
	{
		Emulator &emulator;

	public:
		CPU(Emulator &emulator);

		/*
		 * See 1.2.1 in the nX-U8 manual.
		 */
		uint8_t reg_r[16];
		uint16_t reg_pc, reg_elr[4], &reg_lr;
		uint16_t reg_csr, reg_ecsr[4], &reg_lcsr;
		uint16_t reg_epsw[4], &reg_psw;
		uint16_t reg_sp, reg_ea;
		uint8_t reg_dsr;

		/*
		 * See 1.2.2.1 in the nX-U8 manual.
		 */
		enum StatusFlag
		{
			PSW_C = 0x80,
			PSW_Z = 0x40,
			PSW_S = 0x20,
			PSW_OV = 0x10,
			PSW_MIE = 0x8,
			PSW_HC = 0x4,
			PSW_ELEVEL = 0x3
		};

		/*
		 * See 1.3.6 in the nX-U8 manual.
		 */
		enum MemoryModel
		{
			MM_SMALL,
			MM_LARGE
		} memory_model;
	};
}

