emu:model({
	model_name = "fx-570ES PLUS",
	interface_image_path = "interface.png",
	rom_path = "rom.bin",
	interface_width = 270,
	interface_height = 540
})

local break_targets = {}

function break_at(addr)
	if not addr then
		local real_pc = (cpu.csr << 16) | cpu.pc
		addr = real_pc
	end
	break_targets[addr] = true
end

function unbreak_at(addr)
	break_targets[addr] = nil
end

function cont()
	emu:set_paused(false)
end

emu:post_tick(function()
	local real_pc = (cpu.csr << 16) | cpu.pc
	if break_targets[real_pc] then
		printf("********** breakpoint reached at %05X **********", real_pc)
		emu:set_paused(true)
	end
end)

function printf(...)
	print(string.format(...))
end

