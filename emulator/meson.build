project('emulator', 'cpp', version: '1.0.0')

executable('emulator', sources: [
	'src/Logger.cpp',
	'src/Peripheral/ROMWindow.cpp',
	'src/Peripheral/Timer.cpp',
	'src/Peripheral/Screen.cpp',
	'src/Peripheral/StandbyControl.cpp',
	'src/Peripheral/Keyboard.cpp',
	'src/Peripheral/Miscellaneous.cpp',
	'src/Peripheral/Peripheral.cpp',
	'src/Peripheral/BatteryBackedRAM.cpp',
	'src/Data/ModelInfo.cpp',
	'src/casioemu.cpp',
	'src/Emulator.cpp',
	'src/Chipset/CPUPushPop.cpp',
	'src/Chipset/CPU.cpp',
	'src/Chipset/InterruptSource.cpp',
	'src/Chipset/Chipset.cpp',
	'src/Chipset/CPUArithmetic.cpp',
	'src/Chipset/MMU.cpp',
	'src/Chipset/CPUControl.cpp',
	'src/Chipset/MMURegion.cpp',
	'src/Chipset/CPULoadStore.cpp',
], dependencies: [
	dependency('threads'),
	dependency('SDL2'),
	dependency('SDL2_image'),
	dependency('lua53'),
])
