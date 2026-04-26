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

static uint32_t sbuf_pos = 0; //标记当前读取位置
static uint8_t *sbuf = NULL;  //循环使用的音频缓冲区
static uint32_t *audio_base = NULL;

void sdl_audio_callback(void *userdata, uint8_t *stream, int len){
  SDL_memset(stream, 0, len);
  //printf("%d\n",len);
  uint32_t used_cnt = audio_base[reg_count];  //剩余数据量
  len = len > used_cnt ? used_cnt : len;  //保证读取不超过可用数据量
  uint32_t sbuf_size = audio_base[reg_sbuf_size];
  //如果剩余数据(sbuf_pos + len)超过缓冲区大小(sbuf_size)，需要分两次播放，需要指针回到开头再输入
  if( (sbuf_pos + len) > sbuf_size ){
    SDL_MixAudio(stream, sbuf + sbuf_pos, sbuf_size - sbuf_pos , SDL_MIX_MAXVOLUME);
    SDL_MixAudio(stream + sbuf_size - sbuf_pos, sbuf + sbuf_size - sbuf_pos, 
                    len - sbuf_size + sbuf_pos, SDL_MIX_MAXVOLUME);
  }
  else 
    SDL_MixAudio(stream, sbuf + sbuf_pos, len , SDL_MIX_MAXVOLUME);
  sbuf_pos = (sbuf_pos + len) % sbuf_size; 
  audio_base[reg_count] -= len;
  //printf("%u\n",audio_base[reg_count]);
}

int init_sound() {
  SDL_AudioSpec s = {};
  s.format = AUDIO_S16SYS;  // 假设系统中音频数据的格式总是使用16位有符号数来表示
  s.userdata = NULL;  
  s.freq = audio_base[reg_freq];
  s.channels = audio_base[reg_channels];
  s.samples = audio_base[reg_samples];
  //printf("%d\n",s.samples);
  s.callback = sdl_audio_callback;
  int ret = SDL_InitSubSystem(SDL_INIT_AUDIO);
  if (ret == 0) {
    SDL_OpenAudio(&s, NULL);
    SDL_PauseAudio(0);
  }       
  return 0;
}

static void audio_io_handler(uint32_t offset, int len, bool is_write) {
  if(audio_base[reg_init] == 1){
    init_sound();
    audio_base[reg_init] = 0;
  }
}

void init_audio() {
  uint32_t space_size = sizeof(uint32_t) * nr_reg;
  audio_base = (uint32_t *)new_space(space_size);
#ifdef CONFIG_HAS_PORT_IO
  add_pio_map ("audio", CONFIG_AUDIO_CTL_PORT, audio_base, space_size, audio_io_handler);
#else
  add_mmio_map("audio", CONFIG_AUDIO_CTL_MMIO, audio_base, space_size, audio_io_handler);
#endif

  sbuf = (uint8_t *)new_space(CONFIG_SB_SIZE);
  add_mmio_map("audio-sbuf", CONFIG_SB_ADDR, sbuf, CONFIG_SB_SIZE, NULL);
  audio_base[reg_sbuf_size] = CONFIG_SB_SIZE;    //确定流缓冲区的大小
}
