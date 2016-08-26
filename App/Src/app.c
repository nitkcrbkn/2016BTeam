#include "app.h"
#include "DD_Gene.h"
#include "DD_RCDefinition.h"
#include "SystemTaskManager.h"
#include <stdlib.h>
#include "message.h"
#include "MW_GPIO.h"
#include "MW_flash.h"
#include "constManager.h"
#include "trapezoid_ctrl.h"

static
int suspensionSystem(void);
/*メモ
 * g_ab_h...ABのハンドラ
 * g_md_h...MDのハンドラ
 * 
 * g_rc_data...RCのデータ
 */

static
int SteerCtrl(void);

static
int ArmSystem(void);

static
int ArmRotate(void);

int appInit(void){
  message("msg", "Message");
  /*GPIO の設定などでMW,GPIOではHALを叩く*/
  return EXIT_SUCCESS;
}

/*application tasks*/
int appTask(void){
  int ret = 0;
  if(__RC_ISPRESSED_R1(g_rc_data)&&__RC_ISPRESSED_R2(g_rc_data)&&
     __RC_ISPRESSED_L1(g_rc_data)&&__RC_ISPRESSED_L2(g_rc_data)){
    while(__RC_ISPRESSED_R1(g_rc_data)||__RC_ISPRESSED_R2(g_rc_data)||
	  __RC_ISPRESSED_L1(g_rc_data)||__RC_ISPRESSED_L2(g_rc_data));
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
  
  ret = ArmSystem();
  if( ret ){
    return ret;
  }
  return EXIT_SUCCESS;
}

/*Private ステア制御*/
static
int SteerCtrl(void){
  /* T o D o */
  g_md_h[STEER_MD_R].mode = D_MMOD_FREE;
  g_md_h[STEER_MD_R].duty = 0;
  g_md_h[STEER_MD_L].mode = D_MMOD_FREE;
  g_md_h[STEER_MD_L].duty = 0;
  return EXIT_SUCCESS;
}

/*Private アーム開閉*/
static
int ArmSystem(void){
  static int prs_tri_s = 0;
  if ( (__RC_ISPRESSED_L1(g_rc_data)) &&
       (__RC_ISPRESSED_R1(g_rc_data)) &&
       (__RC_ISPRESSED_TRIANGLE(g_rc_data)) ) {
    if (prs_tri_s == 0) {
      g_ab_h[DRIVER_AB].dat ^= ARM_OC_AB;
      g_ab_h[DRIVER_VM].dat ^= STICK_BOX_VM;
      prs_tri_s = 1;
    }
  } else {
    prs_tri_s = 0;
  } 
  return EXIT_SUCCESS;
}

/*Private アーム上下*/
static
int ArmRotate(void){
  const int fakean = 8;
  if ( (__RC_ISPRESSED_UP(g_rc_data)) &&
      !(__RC_ISPRESSED_DOWN(g_rc_data)) &&
       (MW_GPIORead(GPIOBID, GPIO_PIN_15) == 0) ){
    g_md_h[ARM_MOVE_MD].mode = D_MMOD_FORWARD;
    g_md_h[ARM_MOVE_MD].duty = fakean * MD_GAIN;
    return EXIT_SUCCESS;
  }
  if ( (__RC_ISPRESSED_DOWN(g_rc_data)) &&
      !(__RC_ISPRESSED_UP(g_rc_data)) &&
       (MW_GPIORead(GPIOCID, GPIO_PIN_0) == 0) ){
    g_md_h[ARM_MOVE_MD].mode = D_MMOD_BACKWARD;
    g_md_h[ARM_MOVE_MD].duty = fakean * MD_GAIN;
    return EXIT_SUCCESS;
  }
  
  g_md_h[ARM_MOVE_MD].duty = 0;
  g_md_h[ARM_MOVE_MD].mode = D_MMOD_FREE;
  return EXIT_SUCCESS;
}

/*Private MD制御*/
static
int suspensionSystem(void){
  const int num_of_motor = 2;/*モータの個数*/
  const int analog_max = 13;
  int rc_analogdata;	/*コントローラから送られるアナログデータを格納*/
  int target;		/*目標となる制御値*/
  int gain;		/*アナログデータと掛け合わせて使うgain値*/
  unsigned int idx;	/*インデックス*/
  const int incr = 500;	/*Duty上昇時の変化量*/
  const int decr = 500;	/*Duty下降時の変化量*/
  int i;		/*カウンタ用*/

  /*for each motor*/
  for( i = 0; i < num_of_motor; i++ ){
    gain = MD_GAIN;
    idx = i;
    /*それぞれの差分*/
    switch( i ){
    case DRIVE_MD_R:
      rc_analogdata = -(DD_RCGetRY(g_rc_data));
      if ( (__RC_ISPRESSED_R2(g_rc_data)) &&
	  !(__RC_ISPRESSED_L2(g_rc_data)) )
	rc_analogdata = -analog_max;
      
      if ( (__RC_ISPRESSED_L2(g_rc_data)) &&
	  !(__RC_ISPRESSED_R2(g_rc_data)) )
	rc_analogdata = analog_max;
      
      if (_IS_REVERSE_R)
	rc_analogdata = -rc_analogdata;
      break;
     
    case DRIVE_MD_L:
      rc_analogdata = -(DD_RCGetRY(g_rc_data));
      if ( (__RC_ISPRESSED_R2(g_rc_data)) &&
	  !(__RC_ISPRESSED_L2(g_rc_data)) )
	rc_analogdata = analog_max;
      
      if ( (__RC_ISPRESSED_L2(g_rc_data)) &&
	  !(__RC_ISPRESSED_R2(g_rc_data)) )
	rc_analogdata = -analog_max;
      
      if (_IS_REVERSE_L)
	rc_analogdata = -rc_analogdata;
      break;
      
    default:
       message("err", "real MDs are fewer than defined");
      return EXIT_FAILURE;
    }
    
    /*これは中央か?±3程度余裕を持つ必要がある。*/
    if( abs(rc_analogdata) > CENTRAL_THRESHOLD ) {
      target = rc_analogdata * gain;
    }
    else { 
      target = 0;
    }
    TrapezoidCtrl(target, &(g_md_h[idx]), incr, decr);
  }
  return EXIT_SUCCESS;
} /* suspensionSystem */

