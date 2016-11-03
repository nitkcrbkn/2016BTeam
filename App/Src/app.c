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
int steerCtrl(void);

static
int armOC(void);

static
int armRotate(void);

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

  ret = steerCtrl();
  if( ret ){
    return ret;
  }

  ret = armRotate();
  if( ret ){
    return ret;
  }

  ret = armOC();
  if( ret ){
    return ret;
  }
  return EXIT_SUCCESS;
} /* appTask */

/*Private ステア制御*/
static
int steerCtrl(void){
  /* g_md_h[STEER_MD_R].mode = D_MMOD_FREE; */
  /* g_md_h[STEER_MD_R].duty = 0; */
  /* g_md_h[STEER_MD_L].mode = D_MMOD_FREE; */
  /* g_md_h[STEER_MD_L].duty = 0; */

  return EXIT_SUCCESS;
}

/*Private アーム開閉*/
static
int armOC(void){
  static int had_pressed_tri_s = 0;
  if(( __RC_ISPRESSED_L1(g_rc_data)) &&
     ( __RC_ISPRESSED_R1(g_rc_data)) &&
     ( __RC_ISPRESSED_TRIANGLE(g_rc_data))){
    if( had_pressed_tri_s == 0 ){
      g_ab_h[DRIVER_AB].dat ^= ARM_OC_AB;
      had_pressed_tri_s = 1;
    }
  } else {
    had_pressed_tri_s = 0;
  }
  return EXIT_SUCCESS;
}

/*Private アーム上下*/
static
int armRotate(void){
  const tc_const_t arm_tcon = {
    .inc_con = 150,
    .dec_con = 10000
  };
  int arm_target;       /*アーム部のduty*/
  static arm_status_t arm_mod = _ARM_NOMOVE_NOAUTO;
  const int arm_down_duty = MD_ARM_DOWN_DUTY * g_adjust.arm_rotate_duty.value / 100;
  const int arm_up_duty = MD_ARM_UP_DUTY * g_adjust.arm_rotate_duty.value / 100;
  static int press_count = 0;
 
  /*コントローラのボタンは押されているか*/
  if( __RC_ISPRESSED_L1(g_rc_data)){
    arm_mod = _ARM_UP_NOAUTO;
    if( press_count++ >= 80 ){
      arm_mod = _ARM_UP_AUTO;
    }
  }else if( __RC_ISPRESSED_L2(g_rc_data)){
    arm_mod = _ARM_DOWN_NOAUTO;
    if( press_count++ >= 80 ){
      arm_mod = _ARM_DOWN_AUTO;
    }
  }else  {
    if( arm_mod == _ARM_UP_NOAUTO || arm_mod == _ARM_DOWN_NOAUTO ){
      arm_mod = _ARM_NOMOVE_NOAUTO;
    }
    press_count = 0;
  }
  /*リミットスイッチは押されているか*/
  if( _IS_PRESSED_UPPER_LIMITSW() &&
      ( arm_mod == _ARM_UP_NOAUTO || arm_mod == _ARM_UP_AUTO )){
    arm_mod = _ARM_NOMOVE_NOAUTO;
  }else if( _IS_PRESSED_LOWER_LIMITSW() &&
            ( arm_mod == _ARM_DOWN_NOAUTO || arm_mod == _ARM_DOWN_AUTO )){
    arm_mod = _ARM_NOMOVE_NOAUTO;
  }

  switch( arm_mod ){
  case _ARM_NOMOVE_NOAUTO:
    arm_target = 0;
    g_led_mode = lmode_1;
    break;
  case _ARM_UP_NOAUTO:
    arm_target = arm_up_duty;
    g_led_mode = lmode_1;
    break;
  case _ARM_DOWN_NOAUTO:
    arm_target = arm_down_duty;
    g_led_mode = lmode_1;
    break;
  case _ARM_UP_AUTO:
    arm_target = arm_up_duty;
    g_led_mode = lmode_2;
    break;
  case _ARM_DOWN_AUTO:
    arm_target = arm_down_duty;
    g_led_mode = lmode_2;
    break;
  default:
    arm_target = 0;
    break;
  }

  trapezoidCtrl(arm_target, &g_md_h[ARM_MOVE_MD], &arm_tcon);

  return EXIT_SUCCESS;
} /* armRotate */

/*プライベート 足回りシステム*/
static
int suspensionSystem(void){
  const int num_of_motor = 2;/*モータの個数*/
  int rc_analogdata;    /*コントローラから送られるアナログデータを格納*/
  int target;           /*目標となる制御値*/
  int gain = (int)( (MD_SUSPENSION_DUTY / DD_RC_ANALOG_MAX) * g_adjust.suspension_duty.value / 100);
  unsigned int idx;     /*インデックス*/
  int i;                /*カウンタ用*/

  const tc_const_t suspension_tcon = {
    .inc_con = 100,
    .dec_con = 225
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
        target = rc_analogdata * gain;
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

