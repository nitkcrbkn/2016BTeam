#include "app.h"
#include "DD_Gene.h"
#include "DD_RCDefinition.h"
#include "SystemTaskManager.h"
#include <stdlib.h>
#include "MW_GPIO.h"
#include "MW_IWDG.h"
#include "message.h"
#include "MW_GPIO.h"
#include "MW_flash.h"
#include "constManager.h"
#include "trapezoid_ctrl.h"

/*メモ
 * g_ab_h...ABのハンドラ
 * g_md_h...MDのハンドラ
 *
 * g_rc_data...RCのデータ
 */

static
int LEDSystem(void);

static
int suspensionSystem(void);

static
int rotationArm(void);

static
int reelSystem(void);

static
int kickABSystem(void);

static
int armABSystem(void);

static
int armVMSystem(void);

int appInit(void){
  message("msg", "hell");

  ad_init();
  return EXIT_SUCCESS;
}

/*application tasks*/
int appTask(void){
  int ret = 0;

  if( __RC_ISPRESSED_R1(g_rc_data) && __RC_ISPRESSED_R2(g_rc_data) &&
      __RC_ISPRESSED_L1(g_rc_data) && __RC_ISPRESSED_L2(g_rc_data)){
    while( __RC_ISPRESSED_R1(g_rc_data) || __RC_ISPRESSED_R2(g_rc_data) ||
           __RC_ISPRESSED_L1(g_rc_data) || __RC_ISPRESSED_L2(g_rc_data)){
      SY_wait(10);
    }
    ad_main();
  }

  /*それぞれの機構ごとに処理をする*/
  /*途中必ず定数回で終了すること。*/
  ret = suspensionSystem();
  if( ret ){
    return ret;
  }
  ret = rotationArm();
  if( ret ){
    return ret;
  }
  ret = reelSystem();
  if( ret ){
    return ret;
  }
  ret = kickABSystem();
  if( ret ){
    return ret;
  }
  ret = armABSystem();
  if( ret ){
    return ret;
  }
  ret = armVMSystem();
  if( ret ){
    return ret;
  }
  ret = LEDSystem();
  if( ret ){
    return ret;
  }

  return EXIT_SUCCESS;
} /* appTask */

static int LEDSystem(void){
  return EXIT_SUCCESS;
}

/*アーム回転*/
static
int rotationArm(void){
  const tc_const_t arm_tcon = {
    .inc_con = 100,
    .dec_con = 1000
  };
  int target;
  if( !( __RC_ISPRESSED_L1(g_rc_data)) &&
      !( __RC_ISPRESSED_R1(g_rc_data)) &&
      ( __RC_ISPRESSED_RIGHT(g_rc_data)) &&
      !( _IS_PRESSED_ARM_CW_LIMITSW())){
    target = MD_ARM_ROTATE_DUTY;
    trapezoidCtrl(target, &g_md_h[ARM_ROTATE_MD], &arm_tcon);
  } else if( !( __RC_ISPRESSED_L1(g_rc_data)) &&
             !( __RC_ISPRESSED_R1(g_rc_data)) &&
             ( __RC_ISPRESSED_LEFT(g_rc_data)) &&
             !( _IS_PRESSED_ARM_CCW_LIMITSW())){
    target = -MD_ARM_ROTATE_DUTY;
    trapezoidCtrl(target, &g_md_h[ARM_ROTATE_MD], &arm_tcon);
  }else {
    g_md_h[ARM_ROTATE_MD].mode = D_MMOD_BRAKE;
    g_md_h[ARM_ROTATE_MD].duty = 0;
  }
  return EXIT_SUCCESS;
}

/*リール機構*/
static
int reelSystem(void){
  const tc_const_t reel_tcon = {
    .inc_con = 100,
    .dec_con = 250
  };
  int target;

  if( !( __RC_ISPRESSED_R1(g_rc_data)) &&
      !( __RC_ISPRESSED_L1(g_rc_data)) &&
      ( __RC_ISPRESSED_UP(g_rc_data))){
    target = MD_REEL_ROLLUP_DUTY;
  } else if(( __RC_ISPRESSED_DOWN(g_rc_data)) &&
            !( __RC_ISPRESSED_L1(g_rc_data)) &&
            !( __RC_ISPRESSED_R1(g_rc_data))){
    target = MD_REEL_RELEASE_DUTY;
  } else{
    target = 0;
  }
  trapezoidCtrl(target, &g_md_h[REEL_MECHA_MD], &reel_tcon);
  return EXIT_SUCCESS;
}

/*プライベート キック用シリンダ*/
static
int kickABSystem(void){
  static uint8_t had_pressed_lrc_s = 0;
  if(( __RC_ISPRESSED_L1(g_rc_data)) &&
     ( __RC_ISPRESSED_R1(g_rc_data)) &&
     ( __RC_ISPRESSED_TRIANGLE(g_rc_data))){
    if( had_pressed_lrc_s == 0 ){
      g_ab_h[DRIVER_AB].dat ^= KICK_AB_R;
      g_ab_h[DRIVER_AB].dat ^= KICK_AB_L;
      had_pressed_lrc_s = 1;
    }
  } else {
    had_pressed_lrc_s = 0;
  }
  return EXIT_SUCCESS;
}

/*アーム展開機構*/
static
int armABSystem(void){
  static uint8_t had_pressed_lrc_s = 0;
  if(( __RC_ISPRESSED_L1(g_rc_data)) &&
     ( __RC_ISPRESSED_R1(g_rc_data)) &&
     ( __RC_ISPRESSED_UP(g_rc_data))){
    if( had_pressed_lrc_s == 0 ){
      g_ab_h[DRIVER_AB].dat ^= ARM_AB_0;
      g_ab_h[DRIVER_AB].dat ^= ARM_AB_1;
      had_pressed_lrc_s = 1;
    }
  } else {
    had_pressed_lrc_s = 0;
  }
  return EXIT_SUCCESS;
}

/*真空モータ*/
static
int armVMSystem(void){
  static uint8_t had_pressed_circle_s = 0;
  if(( __RC_ISPRESSED_CIRCLE(g_rc_data)) &&
     !( __RC_ISPRESSED_L1(g_rc_data)) &&
     !( __RC_ISPRESSED_R1(g_rc_data))){
    if( had_pressed_circle_s == 0 ){
      g_ab_h[DRIVER_VM].dat ^= STICK_BOX_VM_0;
      g_ab_h[DRIVER_VM].dat ^= STICK_BOX_VM_1;
      had_pressed_circle_s = 1;
    }
  } else {
    had_pressed_circle_s = 0;
  }
  return EXIT_SUCCESS;
}

/*プライベート 足回りシステム*/
static
int suspensionSystem(void){
  const tc_const_t suspension_tcon = {
    .inc_con = 150,
    .dec_con = 250
  };

  const int num_of_motor = 2;/*モータの個数*/
  int rc_analogdata;    /*コントローラから送られるアナログデータを格納*/
  int target;           /*目標となる制御値*/
  int gain = (int)( MD_SUSPENSION_DUTY / DD_RC_ANALOG_MAX );
  unsigned int idx;     /*インデックス*/
  int i;                /*カウンタ用*/

  /*for each motor*/
  for( i = 0; i < num_of_motor; i++ ){
    target = 0;
    /*それぞれの差分*/
    switch( i ){
    case 0:
      idx = DRIVE_MD_R;
      rc_analogdata = -( DD_RCGetRY(g_rc_data));

      /*これは中央か?±3程度余裕を持つ必要がある。*/
      if( abs(rc_analogdata) > CENTRAL_THRESHOLD ){
        target = rc_analogdata * gain;
      }
      if(( __RC_ISPRESSED_R2(g_rc_data)) &&
         !( __RC_ISPRESSED_L2(g_rc_data))){
        target = -MD_TURN_DUTY;
      } else if(( __RC_ISPRESSED_L2(g_rc_data)) &&
                !( __RC_ISPRESSED_R2(g_rc_data))){
        target = MD_TURN_DUTY;
      }
#if _IS_REVERSE_R
      target = -target;
#endif
      break;

    case 1:
      idx = DRIVE_MD_L;
      rc_analogdata = -( DD_RCGetLY(g_rc_data));

      /*これは中央か?±3程度余裕を持つ必要がある。*/
      if( abs(rc_analogdata) > CENTRAL_THRESHOLD ){
        target = rc_analogdata * gain;
      }
      if(( __RC_ISPRESSED_R2(g_rc_data)) &&
         !( __RC_ISPRESSED_L2(g_rc_data))){
        target = MD_TURN_DUTY;
      } else if(( __RC_ISPRESSED_L2(g_rc_data)) &&
                !( __RC_ISPRESSED_R2(g_rc_data))){
        target = -MD_TURN_DUTY;
      }
#if _IS_REVERSE_L
      target = -target;
#endif
      break;

    default:
      message("err", "real MDs are fewer than defined idx:%d", i);
      return EXIT_FAILURE;
    } /* switch */

    if( target > MD_SUSPENSION_DUTY ){
      target = MD_SUSPENSION_DUTY;
    } else if( target < -MD_SUSPENSION_DUTY ){
      target = -MD_SUSPENSION_DUTY;
    }
    trapezoidCtrl(target, &g_md_h[idx], &suspension_tcon);
  }
  return EXIT_SUCCESS;
} /* suspensionSystem */
