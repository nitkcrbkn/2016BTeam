#ifndef __APP_H
#define __APP_H

/*NO Device mode*/
#define _NO_DEVICE 0

#include "DD_RC.h"
#include "DD_MD.h"

int appTask(void);
int appInit(void);

#define CENTRAL_THRESHOLD 4

/*Reverse MD direction*/
#define _IS_REVERSE_R 0
#define _IS_REVERSE_L 0

#define DD_NUM_OF_MD 5
#define DD_NUM_OF_AB 2

/*駆動部*/
#define DRIVE_MD_R 0
#define DRIVE_MD_L 1

#define STEER_MD_R 2
#define STEER_MD_L 3

/*アーム上下用*/
#define ARM_MOVE_MD 4

#define DRIVER_AB 0
#define DRIVER_VM 1

/*アーム開閉用*/
#define ARM_OC_AB (1<<0)

/*箱吸着用真空モータ*/
#define STICK_BOX_VM (1<<0)

#define MD_GAIN ( DD_MD_MAX_DUTY / DD_RC_ANALOG_MAX )

#endif
