do
	local buttons = {}
	local function generate(px, py, w, h, nx, ny, sx, sy, code)
		local cp = 0
		for iy = 0, ny - 1 do
			for ix = 0, nx - 1 do
				cp = cp + 1
				table.insert(buttons, {px + ix * sx, py + iy * sy, w, h, code[cp]})
			end
		end
	end
	generate(46, 544, 58, 41, 5, 4, 65, 57, {
		0x0104, 0x0204, 0x0404, 0x0804, 0x1004,
		0x0102, 0x0202, 0x0402, 0x0802, 0x1002,
		0x0101, 0x0201, 0x0401, 0x0801, 0x1001,
		0x4010, 0x4008, 0x4004, 0x4002, 0x4001
	})
	generate(46, 406, 48, 31, 6, 3, 54, 46, {
		0x0120, 0x0220, 0x0420, 0x0820, 0x1020, 0x2020,
		0x0110, 0x0210, 0x0410, 0x0810, 0x1010, 0x2010,
		0x0108, 0x0208, 0x0408, 0x0808, 0x1008, 0x2008
	})
	generate(40, 359, 48, 31, 2, 1, 54, 0, {0x0140, 0x0240})
	generate(268, 359, 48, 31, 2, 1, 54, 0, {0x1040, 0x2040})
	generate(44, 290, 49, 39, 2, 1, 273, 0, {0x0180, 0})
	generate(100, 298, 48, 38, 2, 1, 162, 0, {0x0280, 0x1080})
	generate(155, 319, 33, 32, 2, 1, 67, 0, {0x0440, 0x0880})
	generate(188, 289, 34, 30, 1, 2, 0, 72, {0x0480, 0x0840})

	emu:model({
		model_name = "fx-570ES PLUS",
		interface_image_path = "interface.png",
		rom_path = "rom.bin",
		rsd_interface = {0, 0, 410, 810, 0, 0},
		rsd_pixel = {410, 252, 3, 3, 61, 141},
		rsd_s = {410, 0, 10, 14, 61, 127},
		rsd_a = {410, 14, 11, 14, 70, 127},
		rsd_m = {410, 28, 10, 14, 81, 127},
		rsd_sto = {410, 42, 20, 14, 91, 127},
		rsd_rcl = {410, 56, 19, 14, 110, 127},
		rsd_stat = {410, 70, 24, 14, 130, 127},
		rsd_cmplx = {410, 84, 32, 14, 154, 127},
		rsd_mat = {410, 98, 20, 14, 186, 127},
		rsd_vct = {410, 112, 20, 14, 205, 127},
		rsd_d = {410, 126, 12, 14, 225, 127},
		rsd_r = {410, 140, 10, 14, 236, 127},
		rsd_g = {410, 154, 11, 14, 246, 127},
		rsd_fix = {410, 168, 17, 14, 257, 127},
		rsd_sci = {410, 182, 16, 14, 273, 127},
		rsd_math = {410, 196, 24, 14, 289, 127},
		rsd_down = {410, 210, 10, 14, 313, 127},
		rsd_up = {410, 224, 10, 14, 319, 127},
		rsd_disp = {410, 238, 20, 14, 329, 127},
		ink_color_on = {30, 52, 90, 255},
		ink_color_off = {30, 52, 90, 63},
		button_map = buttons
	})
end

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

