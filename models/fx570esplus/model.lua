emu:model({
	model_name = "fx-570ES PLUS",
	interface_image_path = "interface.png",
	rom_path = "rom.bin",
	interface_width = 270,
	interface_height = 540
})

local break_targets = {}

local function get_real_pc()
	return (cpu.csr << 16) | cpu.pc
end

function break_at(addr)
	if not addr then
		addr = get_real_pc()
	end
	break_targets[addr] = true
end

function unbreak_at(addr)
	if not addr then
		addr = get_real_pc()
	end
	break_targets[addr] = nil
end

function cont()
	emu:set_paused(false)
end

emu:post_tick(function()
	local real_pc = get_real_pc()
	if break_targets[real_pc] then
		printf("********** breakpoint reached at %05X **********", real_pc)
		emu:set_paused(true)
	end
end)

function printf(...)
	print(string.format(...))
end

function ins()
	printf("%02X %02X %02X %02X | %01X:%04X | %02X %01X:%04X", cpu.r0, cpu.r1, cpu.r2, cpu.r3, cpu.csr,  cpu.pc, cpu.psw, cpu.lcsr, cpu.lr)
	printf("%02X %02X %02X %02X | S %04X | %02X %01X:%04X", cpu.r4, cpu.r5, cpu.r6, cpu.r7, cpu.sp, cpu.epsw1, cpu.ecsr1, cpu.elr1)
	printf("%02X %02X %02X %02X | A %04X | %02X %01X:%04X", cpu.r8, cpu.r9, cpu.r10, cpu.r11, cpu.ea, cpu.epsw2, cpu.ecsr2, cpu.elr2)
	printf("%02X %02X %02X %02X | ELVL %01X | %02X %01X:%04X", cpu.r12, cpu.r13, cpu.r14, cpu.r15, cpu.psw & 3, cpu.epsw3, cpu.ecsr3, cpu.elr3)
end

