#include "DD_Gene.h"
#include "app.h"
#include "SystemTaskManager.h"

/*Address Definition*/
#if DD_NUM_OF_MD
/*MD Definition*/
DD_MDHand_t g_md_h[DD_NUM_OF_MD] = {
  { 0x10, /* address 駆動(Right)*/
    0, /* default duty */
    D_MMOD_FREE, /* mode */
},
  { 0x11,       /*駆動(Left)*/
    0,
    D_MMOD_FREE, },
  { 0x12,       /*ステア用EC付(Right)*/
    0,
    D_MMOD_FREE, },
  { 0x13,       /*ステア用EC付(Left)*/
    0,
    D_MMOD_FREE, },
  { 0x14,       /*アーム上下**/
    0,
    D_MMOD_FREE, },
};
#endif
#if DD_NUM_OF_AB
/*AB Definition*/
DD_ABHand_t g_ab_h[DD_NUM_OF_AB] = {
  { 0x20, /* address (シリンダ)*/
    0x00, /* data */
},
  { 0x30,       /*真空モータ*/
    0x00, },
};
#endif
