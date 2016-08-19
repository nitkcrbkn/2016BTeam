#include "DD_Gene.h"
#include "app.h"
#include "SystemTaskManager.h"

/*Address Definition*/
#if DD_NUM_OF_MD
/*MD Definition*/
DD_MDHand_t g_md_h[DD_NUM_OF_MD] = {
  { 0x10, /* address (駆動Right)*/
    0, /* default duty */
    D_MMOD_FREE, /* mode */
  },
  { 0x11, /* address (駆動Left)*/
    0, /* default duty */
    D_MMOD_FREE, /* mode */
  },
  { 0x12,	/*回転機構**/
    0,
    D_MMOD_FREE,
  },
  { 0x13,	/*リール機構*/
    0,
    D_MMOD_FREE,
  },
};
#endif
#if DD_NUM_OF_AB
/*AB Definition*/
DD_ABHand_t g_ab_h[DD_NUM_OF_AB] = {
  { 0x20, /* address (シリンダ)*/
    0x00, /* data */
  },
  { 0x21,	/*真空モータ、ソレノイド*/
    0x00,
  },
};
#endif
