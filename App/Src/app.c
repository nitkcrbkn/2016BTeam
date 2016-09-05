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

static
int LEDSystem(void);
/*メモ
 * g_ab_h...ABのハンドラ
 * g_md_h...MDのハンドラ
 *
 * g_rc_data...RCのデータ
 */

static
int suspensionSystem(void);

static
int RotationArm(void);

int appInit(void){
  /* switch(checkFlashWrite()){ */
  /* case MW_FLASH_OK: */
  /*   message("msg","FLASH WRITE TEST SUCCESS\n%s",(const char*)WRITE_ADDR); */
  /*   break; */
  /* case MW_FLASH_LOCK_FAILURE: */
  /*   message("err","FLASH WRITE TEST LOCK FAILURE\n"); */
  /*   break; */
  /* case MW_FLASH_UNLOCK_FAILURE: */
  /*   message("err","FLASH WRITE TEST UNLOCK FAILURE\n"); */
  /*   break; */
  /* case MW_FLASH_ERASE_VERIFY_FAILURE: */
  /*   message("err","FLASH ERASE VERIFY FAILURE\n"); */
  /*   break; */
  /* case MW_FLASH_ERASE_FAILURE: */
  /*   message("err","FLASH ERASE FAILURE\n"); */
  /*   break; */
  /* case MW_FLASH_WRITE_VERIFY_FAILURE: */
  /*   message("err","FLASH WRITE TEST VERIFY FAILURE\n"); */
  /*   break; */
  /* case MW_FLASH_WRITE_FAILURE: */
  /*   message("err","FLASH WRITE TEST FAILURE\n"); */
  /*   break;         */
  /* default: */
  /*   message("err","FLASH WRITE TEST UNKNOWN FAILURE\n"); */
  /*   break; */
  /* } */
  /* flush(); */
  message("msg","hell");

  return EXIT_SUCCESS;
}

static
int ReelSystem(void);


static
int KickABSystem(void);

static
int ArmABSystem(void);

const tc_const_t g_tcon = {
  500,
  500
};

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
  ret = RotationArm();
  if( ret ){
    return ret;
  }
  
  ret = ReelSystem();
  if( ret ){
    return ret;
  }

  ret = KickABSystem();
  if( ret ){
    return ret;
  }

  ret = ArmABSystem();
  if( ret ){
    return ret;
  }

  ret = LEDSystem();
  if(ret){
    return ret;
  }
     
  return EXIT_SUCCESS;
}

static int LEDSystem(void){
  if(__RC_ISPRESSED_UP(g_rc_data)){
    g_led_mode = lmode_1;
  }
  if(__RC_ISPRESSED_DOWN(g_rc_data)){
    g_led_mode = lmode_2;
  }
  if(__RC_ISPRESSED_RIGHT(g_rc_data)){
    g_led_mode = lmode_3;
  }

  return EXIT_SUCCESS;
} /* appTask */

/*アーム回転*/ 
static
int RotationArm(void){
  if ( (__RC_ISPRESSED_L1(g_rc_data)) &&  
       (__RC_ISPRESSED_R1(g_rc_data)) &&  
       (__RC_ISPRESSED_RIGHT(g_rc_data)) ){
    g_md_h[ARM_ROTATE_MD].mode = D_MMOD_FORWARD;
    g_md_h[ARM_ROTATE_MD].duty = MD_ARM_ROTATE_DUTY;
    return EXIT_SUCCESS;
  }
  if ( (__RC_ISPRESSED_L1(g_rc_data)) &&  
       (__RC_ISPRESSED_R1(g_rc_data)) &&  
       (__RC_ISPRESSED_LEFT(g_rc_data)) ){
    g_md_h[ARM_ROTATE_MD].mode = D_MMOD_BACKWARD;
    g_md_h[ARM_ROTATE_MD].duty = MD_ARM_ROTATE_DUTY;
    return EXIT_SUCCESS;
  }
  g_md_h[ARM_ROTATE_MD].mode = D_MMOD_FREE;
  g_md_h[ARM_ROTATE_MD].duty = 0;
  return EXIT_SUCCESS;
}

/*リール機構*/
static
int ReelSystem(void){
  
  return EXIT_SUCCESS;
}

/*プライベート キック用シリンダ*/
static
int KickABSystem(void){
  static uint8_t had_pressed_lrc_s = 0; 
  if ( (__RC_ISPRESSED_L1(g_rc_data)) &&  
       (__RC_ISPRESSED_R1(g_rc_data)) &&  
       (__RC_ISPRESSED_CIRCLE(g_rc_data)) ) { 
    if (had_pressed_lrc_s == 0){ 
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
int ArmABSystem(void){
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
      if (target > MD_SUSPENSION_DUTY)
	target = MD_SUSPENSION_DUTY;
      if (target < -MD_SUSPENSION_DUTY)
	target = -MD_SUSPENSION_DUTY;
      
#if _IS_REVERSE_R
      target = -target;
#endif
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
      if (target > MD_SUSPENSION_DUTY)
	target = MD_SUSPENSION_DUTY;
      if (target < -MD_SUSPENSION_DUTY)
	target = -MD_SUSPENSION_DUTY;
      
#if _IS_REVERSE_L
      target = -target;
#endif
      TrapezoidCtrl(target, &g_md_h[idx], &g_tcon);
      break;
      
    default:
      message("err", "real MDs are fewer than defined idx:%d", i);
      return EXIT_FAILURE;
    } /* switch */
  }
  return EXIT_SUCCESS;
} /* suspensionSystem */

