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
int SteerCtrl(void);

static
int ArmOC(void);

static
int ArmRotate(void);

const tc_const_t g_tcon = {
  500,
  500
};

int appInit(void){
  message("msg", "Message");
  ad_init();
  /*GPIO の設定などでMW,GPIOではHALを叩く*/
  return EXIT_SUCCESS;
}

/*application tasks*/
int appTask(void){
  int ret=0;

  if(__RC_ISPRESSED_R1(g_rc_data)&&__RC_ISPRESSED_R2(g_rc_data)&&
     __RC_ISPRESSED_L1(g_rc_data)&&__RC_ISPRESSED_L2(g_rc_data)){
    while(__RC_ISPRESSED_R1(g_rc_data)||__RC_ISPRESSED_R2(g_rc_data)||
	  __RC_ISPRESSED_L1(g_rc_data)||__RC_ISPRESSED_L2(g_rc_data))
        SY_wait(10);
    ad_main();
  }

  /*それぞれの機構ごとに処理をする*/
  /*途中必ず定数回で終了すること。*/
  ret = suspensionSystem();
  if( ret ){
    return ret;
  }

  ret = SteerCtrl();
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
  return EXIT_SUCCESS;
} /* appTask */

/*Private ステア制御*/
static
int SteerCtrl(void){
  /* g_md_h[STEER_MD_R].mode = D_MMOD_FREE; */
  /* g_md_h[STEER_MD_R].duty = 0; */
  /* g_md_h[STEER_MD_L].mode = D_MMOD_FREE; */
  /* g_md_h[STEER_MD_L].duty = 0; */

  return EXIT_SUCCESS;
}

/*Private アーム開閉*/
static
int ArmOC(void){
  static int had_pressed_tri_s = 0;
  if(( __RC_ISPRESSED_L1(g_rc_data)) &&
     ( __RC_ISPRESSED_R1(g_rc_data)) &&
     ( __RC_ISPRESSED_TRIANGLE(g_rc_data))){
    if( had_pressed_tri_s == 0 ){
      g_ab_h[DRIVER_AB].dat ^= ARM_OC_AB;
      g_ab_h[DRIVER_VM].dat ^= STICK_BOX_VM;
      had_pressed_tri_s = 1;
    }
  } else {
    had_pressed_tri_s = 0;
  }
  return EXIT_SUCCESS;
}

/*Private アーム上下*/
static
int ArmRotate(void){
  /*アーム上昇*/
  if(( __RC_ISPRESSED_UP(g_rc_data)) &&
     !( __RC_ISPRESSED_DOWN(g_rc_data)) &&
      ( _SW_NOT_UPPER_LIMIT())){
    g_md_h[ARM_MOVE_MD].mode = D_MMOD_BACKWARD;
    g_md_h[ARM_MOVE_MD].duty = MD_ARM_DUTY;
    return EXIT_SUCCESS;
  }
  /*アーム下降*/
  if(( __RC_ISPRESSED_DOWN(g_rc_data)) &&
     !( __RC_ISPRESSED_UP(g_rc_data)) &&
      ( _SW_NOT_LOWER_LIMIT())){
    g_md_h[ARM_MOVE_MD].mode = D_MMOD_FORWARD;
    g_md_h[ARM_MOVE_MD].duty = MD_ARM_DUTY;
    return EXIT_SUCCESS;
  }

  g_md_h[ARM_MOVE_MD].duty = 0;
  g_md_h[ARM_MOVE_MD].mode = D_MMOD_BRAKE;;
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
      if(( __RC_ISPRESSED_R2(g_rc_data)) &&
         !( __RC_ISPRESSED_L2(g_rc_data))){
        target = -MD_SUSPENSION_DUTY;
      }

      if(( __RC_ISPRESSED_L2(g_rc_data)) &&
         !( __RC_ISPRESSED_R2(g_rc_data))){
        target = MD_SUSPENSION_DUTY;
      }

      #if _IS_REVERSE_R
            target = -target;
      #endif
      if (target > MD_SUSPENSION_DUTY)
        target = MD_SUSPENSION_DUTY;
      if (target < -MD_SUSPENSION_DUTY)
        target = -MD_SUSPENSION_DUTY;
      TrapezoidCtrl(target, &g_md_h[idx], &g_tcon);
      break;

    case 1:
      idx = DRIVE_MD_L;
      rc_analogdata = -( DD_RCGetRY(g_rc_data));
      /*これは中央か?±3程度余裕を持つ必要がある。*/
      if( abs(rc_analogdata) > CENTRAL_THRESHOLD ){
        target = rc_analogdata * MD_GAIN;
      }
      if(( __RC_ISPRESSED_R2(g_rc_data)) &&
         !( __RC_ISPRESSED_L2(g_rc_data))){
        target = MD_SUSPENSION_DUTY;
      }
      if(( __RC_ISPRESSED_L2(g_rc_data)) &&
         !( __RC_ISPRESSED_R2(g_rc_data))){
        target = -MD_SUSPENSION_DUTY;
      }

      #if _IS_REVERSE_L
            target = -target;
      #endif
      if (target > MD_SUSPENSION_DUTY)
        target = MD_SUSPENSION_DUTY;
      if (target < -MD_SUSPENSION_DUTY)
        target = -MD_SUSPENSION_DUTY;
      TrapezoidCtrl(target, &g_md_h[idx], &g_tcon);
      break;

    default:
      message("err", "real MDs are fewer than defined idx:%d", i);
      return EXIT_FAILURE;
    } /* switch */
  }
  return EXIT_SUCCESS;
} /* suspensionSystem */
