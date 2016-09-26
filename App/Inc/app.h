#ifndef __APP_H
#define __APP_H

/*NO Device mode*/
#define _NO_DEVICE 0

int appTask(void);
int appInit(void);

/*上段のリミットスイッチは押されているか*/
#define _SW_UPPER_LIMIT_GPIOxID GPIOBID
#define _SW_UPPER_LIMIT_GPIOPIN GPIO_PIN_15
#define _IS_PRESSED_UPPER_LIMITSW() (( MW_GPIORead(_SW_UPPER_LIMIT_GPIOxID, _SW_UPPER_LIMIT_GPIOPIN)) ^ 1)

/*下段のリミットスイッチは押されているか*/
#define _SW_LOWER_LIMIT_GPIOxID GPIOCID
#define _SW_LOWER_LIMIT_GPIOPIN GPIO_PIN_0
#define _IS_PRESSED_LOWER_LIMITSW() (( MW_GPIORead(_SW_LOWER_LIMIT_GPIOxID, _SW_LOWER_LIMIT_GPIOPIN)) ^ 1)

/*アーム上下用モータのduty*/
#define MD_ARM_DUTY 9500
#define MD_ARM_UP_DUTY -MD_ARM_DUTY
#define MD_ARM_DOWN_DUTY MD_ARM_DUTY

#define MD_SUSPENSION_DUTY 4000

#define MD_WAIST_ROTATE_DUTY 4000
#define MD_WAIST_ROTATE_CW_DUTY MD_WAIST_ROTATE_DUTY
#define MD_WAIST_ROTATE_CCW_DUTY -MD_WAIST_ROTATE_DUTY

#define CENTRAL_THRESHOLD 4

/*Reverse MD direction Flags*/
#define _IS_REVERSE_R 1
#define _IS_REVERSE_L 0

/*Num of device*/
#define DD_NUM_OF_MD 4
#define DD_NUM_OF_AB 1
#define DD_NUM_OF_SV 0
#define DD_USE_ENCODER1 0
#define DD_USE_ENCODER2 0

/*駆動部*/
#define DRIVE_MD_R 0
#define DRIVE_MD_L 1
/*アーム上下用*/
#define ARM_MOVE_MD 2
/*腰回転用*/
#define ROTATE_WAIST_MD 3

#define DRIVER_AB 0
/*キック用シリンダ*/
#define KICK_AB_R ( 1 << 4 )
#define KICK_AB_L ( 1 << 5 )
/*アーム開閉用*/
#define ARM_OC_AB ( 1 << 0 )
/*展開機構*/
#define EXPAND_MECHA_AB ( 1 << 3 )

#define MD_GAIN ( DD_MD_MAX_DUTY / DD_RC_ANALOG_MAX )

#endif
