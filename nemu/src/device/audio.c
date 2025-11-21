/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <common.h>
#include <device/map.h>
#include <SDL2/SDL.h>

enum {
  reg_freq,
  reg_channels,
  reg_samples,
  reg_sbuf_size,
  reg_init,
  reg_count,
  nr_reg
};

static uint8_t *sbuf = NULL;
static uint32_t *audio_base = NULL;

static void audio_callback(void *userdata, Uint8 *stream, int len) {
	printf("Audio callback called, len=%d\n", len);
	int sbuf_size = audio_base[reg_sbuf_size];
	int cpy_len = sbuf_size < len ? sbuf_size : len;
	memcpy(stream, sbuf, cpy_len);
	if(cpy_len < len){
		memset(stream + cpy_len, 0, len - cpy_len);
	}
}

static void audio_io_handler(uint32_t offset, int len, bool is_write) {
	if(is_write){
		if(offset == reg_init * 4)
		{
			SDL_AudioSpec s={};
			s.format = AUDIO_S16SYS;
			s.freq = audio_base[reg_freq];
			s.channels = audio_base[reg_channels];
			s.samples = audio_base[reg_samples];
			s.callback = audio_callback;
			assert(SDL_InitSubSystem(SDL_INIT_AUDIO)==0);
			assert(SDL_OpenAudio(&s, NULL) == 0);
			SDL_PauseAudio(0);
			printf("Audio initialized: freq=%d, channels=%d, samples=%d\n", s.freq, s.channels, s.samples);
		}
	}
	else {
		if(offset == reg_count * 4)
		{
			// A successful call to SDL_OpenAudio() is always device id 1, and legacy SDL audio APIs assume you want this device ID.
			int queued = SDL_GetQueuedAudioSize(1);
			printf("Audio queued size: %d bytes\n", queued);
			audio_base[reg_count] = queued;
		}
	}

}

void init_audio() {
  uint32_t space_size = sizeof(uint32_t) * nr_reg;
  audio_base = (uint32_t *)new_space(space_size);

	audio_base[reg_sbuf_size] = CONFIG_SB_SIZE;

#ifdef CONFIG_HAS_PORT_IO
  add_pio_map ("audio", CONFIG_AUDIO_CTL_PORT, audio_base, space_size, audio_io_handler);
#else
  add_mmio_map("audio", CONFIG_AUDIO_CTL_MMIO, audio_base, space_size, audio_io_handler);
#endif

  sbuf = (uint8_t *)new_space(CONFIG_SB_SIZE);
  add_mmio_map("audio-sbuf", CONFIG_SB_ADDR, sbuf, CONFIG_SB_SIZE, NULL);
}
