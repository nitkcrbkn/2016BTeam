#include "app.h"
#include "DD_Gene.h"
#include "SystemTaskManager.h"

/*Address Definition*/
#if DD_NUM_OF_MD
/*MD Definition*/
DD_MDHand_t g_md_h[DD_NUM_OF_MD] = {
  { .add = 0x10, /* address (駆動Right)*/
    .duty = 0, /* default duty */
    .mode = D_MMOD_FREE, /* mode */
},
  { .add = 0x11, /*駆動L*/
    .duty = 0,
    .mode = D_MMOD_FREE, },
  { .add = 0x12,        /*回転機構*/
    .duty = 0,
    .mode = D_MMOD_FREE, },
  { .add = 0x13,        /*リール機構*/
    .duty = 0,
    .mode = D_MMOD_FREE, },
  { .add = 0x14,        /*タイヤ的ななにか*/
    .duty = 0,
    .mode = D_MMOD_FREE, },
};
#endif
#if DD_NUM_OF_AB
/*AB Definition*/
DD_ABHand_t g_ab_h[DD_NUM_OF_AB] = {
  { .add = 0x20, /* address (シリンダ)*/
    .dat = 0x00, /* data */
},
  { .add = 0x30,        /*真空モータ*/
    .dat = 0x00, },
};
#endif
#if DD_NUM_OF_SV
DD_SV_t g_sv_h = {
  .i2cadd = 0x40,/*address*/
  .val = {
    SV_ORIGIN_VALUE, SV_ORIGIN_VALUE, SV_ORIGIN_VALUE, SV_ORIGIN_VALUE
  }
};
#endif
