#ifndef __APP_H
#define __APP_H

/*NO Device mode*/
#define _NO_DEVICE 0

int appTask(void);
int appInit(void);

#define DD_USE_ENCODER1 0
#define DD_USE_ENCODER2 0
#define DD_NUM_OF_SV 0

#include "DD_RC.h"
#include "DD_MD.h"
#include "DD_SV.h"

#define CENTRAL_THRESHOLD 4

/*Reverse MD direction*/
#define _IS_REVERSE_R 1
#define _IS_REVERSE_L 0

#define DD_NUM_OF_MD 4
#define DD_NUM_OF_AB 2

#define MD_SUSPENSION_DUTY 5000
#define MD_ARM_ROTATE_DUTY 5000
#define MD_REEL_DUTY 5000

/*駆動部*/
#define DRIVE_MD_R 0
#define DRIVE_MD_L 1
/*回転機構用モータ,リール機構用モータ*/
#define ARM_ROTATE_MD 2
#define REEL_MECHA_MD 3

#define DRIVER_AB 0
#define DRIVER_VM 1

/*キック用シリンダ*/
#define KICK_AB_R (1<<0)
#define KICK_AB_L (1<<1)
/*アーム伸縮用エアシリンダ*/
#define ARM_AB_0 (1<<2)
#define ARM_AB_1 (1<<3)

/*箱吸着用真空モータ*/
#define STICK_BOX_VM_0 (1<<0)
#define STICK_BOX_VM_1 (1<<1)

/*クラッチ機構用ソレノイド*/
#define CLUTCH_SN (1<<0)

#define MD_GAIN ( DD_MD_MAX_DUTY / DD_RC_ANALOG_MAX )

#endif
