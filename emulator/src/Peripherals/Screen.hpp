#pragma once
#include "../Config.hpp"

#include "../Peripheral.hpp"
#include "../MMURegion.hpp"
#include "../SpriteInfo.hpp"

namespace casioemu
{
	class Screen : public Peripheral
	{
		MMURegion region;
		uint8_t *screen_buffer;

	    SDL_Renderer *renderer;
	    SDL_Texture *interface_texture;

	    uint8_t ink_alpha_on, ink_alpha_off;

		enum Sprite
		{
			SPR_PIXEL,
			SPR_S,
			SPR_A,
			SPR_M,
			SPR_STO,
			SPR_RCL,
			SPR_STAT,
			SPR_CMPLX,
			SPR_MAT,
			SPR_VCT,
			SPR_D,
			SPR_R,
			SPR_G,
			SPR_FIX,
			SPR_SCI,
			SPR_MATH,
			SPR_DOWN,
			SPR_UP,
			SPR_DISP,
			SPR_MAX
		};

		struct SpriteBitmap
		{
			const char *name;
			uint8_t mask, offset;
		};

		static SpriteBitmap sprite_bitmap[];
		SpriteInfo sprite_info[SPR_MAX];

	public:
		using Peripheral::Peripheral;

		void Initialise();
		void Uninitialise();
		void Tick();
		void Frame();
		void UIEvent(SDL_Event &event);
	};
}

