#ifndef __APP_H
#define __APP_H

/*NO Device mode*/
#define _NO_DEVICE 0

#include "DD_RC.h"
#include "DD_MD.h"

int appTask(void);
int appInit(void);

#define DD_NUM_OF_MD 4
#define DD_NUM_OF_AB 2

/*駆動部*/
#define MECHA1_MD1 0x00
#define MECHA1_MD2 0x01

/*回転機構用モータ,リール機構用モータ*/
#define MECHA2_MD1 0x02
#define MECHA2_MD2 0x03

#define CENTRAL_THRESHOLD 4

/* Reverse MD direction */
#define _IS_REVERSE_R 0
#define _IS_REVERSE_L 0

/*キック用シリンダ*/
#define AB0 (1<<0)
#define AB1 (1<<1)

/*アーム伸縮用エアシリンダ*/
#define AB2 (1<<2)
#define AB3 (1<<3)

/*箱吸着用真空モータ*/
#define VM0 (1<<0)
#define VM1 (1<<1)

/*クラッチ機構用ソレノイド*/
#define SN0 (1<<0)

#define MD_GAIN ( DD_MD_MAX_DUTY / DD_RC_ANALOG_MAX )

#endif
