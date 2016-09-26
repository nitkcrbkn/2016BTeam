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
int suspensionSystem(void);

static
int ArmOC(void);

static
int KickABSystem(void);

static
int ArmRotate(void);

static
int WaistRotate(void);

static
int ExpandSystem(void);

int appInit(void){
  message("msg", "Message");
  ad_init();
  /*GPIO の設定などでMW,GPIOではHALを叩く*/
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

  ret = ArmRotate();
  if( ret ){
    return ret;
  }

  ret = ArmOC();
  if( ret ){
    return ret;
  }

  ret = KickABSystem();
  if ( ret ){
    return ret;
  }

  ret = WaistRotate();
  if( ret ){
    return ret;
  }

  ret = ExpandSystem();
  if( ret ){
    return ret;
  }

  return EXIT_SUCCESS;
} /* appTask */

/*キック用エアシリンダ*/
static
int KickABSystem(void){
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

/*Private アーム開閉*/
static
int ArmOC(void){
  static int had_pressed_cir_s = 0;
  if(( __RC_ISPRESSED_L1(g_rc_data)) &&
     ( __RC_ISPRESSED_R1(g_rc_data)) &&
     ( __RC_ISPRESSED_CIRCLE(g_rc_data))){
    if( had_pressed_cir_s == 0 ){
      g_ab_h[DRIVER_AB].dat ^= ARM_OC_AB;
      had_pressed_cir_s = 1;
    }
  } else {
    had_pressed_cir_s = 0;
  }
  return EXIT_SUCCESS;
}

/*Private アーム上下*/
static
int ArmRotate(void){
  int target;
  const tc_const_t tcon = {
    .inc_con = 200,
    .dec_con = 10000
  };

  /*アーム上昇*/
  if( ( __RC_ISPRESSED_UP(g_rc_data)) &&
      !( _IS_PRESSED_UPPER_LIMITSW()) ){
    target = MD_ARM_UP_DUTY;
  } else if( ( __RC_ISPRESSED_DOWN(g_rc_data)) &&
	     !( _IS_PRESSED_LOWER_LIMITSW()) ){
    target = MD_ARM_DOWN_DUTY;
  }else {
    target = 0;
  }
  TrapezoidCtrl(target, &g_md_h[ARM_MOVE_MD], &tcon);
  return EXIT_SUCCESS;
}

/*腰回転機構*/
static
int WaistRotate(void){
  int target;
  const tc_const_t tcon = {
    .inc_con = 200,
    .dec_con = 10000
  };

  if(__RC_ISPRESSED_LEFT(g_rc_data)){
    target = MD_WAIST_ROTATE_CCW_DUTY;
  } else if(__RC_ISPRESSED_RIGHT(g_rc_data)){
    target = MD_WAIST_ROTATE_CW_DUTY;
  }else {
    target = 0;
  }
  TrapezoidCtrl(target, &g_md_h[ROTATE_WAIST_MD], &tcon);
  return EXIT_SUCCESS;
}

/*展開機構*/
static
int ExpandSystem(void){
  static int had_pressed_sqare_s = 0;
  if(( __RC_ISPRESSED_L1(g_rc_data)) &&
     ( __RC_ISPRESSED_R1(g_rc_data)) &&
     ( __RC_ISPRESSED_SQARE(g_rc_data))){
    if( had_pressed_sqare_s == 0 ){
      g_ab_h[DRIVER_AB].dat ^= EXPAND_MECHA_AB;
      had_pressed_sqare_s = 1;
    }
  } else {
    had_pressed_sqare_s = 0;
  }

  return EXIT_SUCCESS;
}

/*プライベート 足回りシステム*/
static
int suspensionSystem(void){
  const int num_of_motor = 2;/*モータの個数*/
  int rc_analogdata;    /*コントローラから送られるアナログデータを格納*/
  int target;           /*目標となる制御値*/
  unsigned int idx;     /*インデックス*/
  int i;                /*カウンタ用*/
  const tc_const_t tcon = {
    .inc_con = 400,
    .dec_con = 500
  };

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
        target = rc_analogdata * MD_GAIN;
      }
      if( __RC_ISPRESSED_R2(g_rc_data)){
        target = -MD_SUSPENSION_DUTY / 2;
      } else if( __RC_ISPRESSED_L2(g_rc_data)){
        target = MD_SUSPENSION_DUTY / 2;
      }

      #if _IS_REVERSE_R
        target = -target;
      #endif
      break;

    case 1:
      idx = DRIVE_MD_L;
      rc_analogdata = -( DD_RCGetRY(g_rc_data));
      /*これは中央か?±3程度余裕を持つ必要がある。*/
      if( abs(rc_analogdata) > CENTRAL_THRESHOLD ){
        target = rc_analogdata * MD_GAIN;
      }
      if( __RC_ISPRESSED_R2(g_rc_data)){
        target = MD_SUSPENSION_DUTY / 2;
      }else if( ( __RC_ISPRESSED_L2(g_rc_data)) ){
        target = -MD_SUSPENSION_DUTY / 2;
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
    TrapezoidCtrl(target, &g_md_h[idx], &tcon);

  }
  return EXIT_SUCCESS;
} /* suspensionSystem */
