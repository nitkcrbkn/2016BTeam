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

/*suspensionSystem*/
static
int suspensionSystem(void);

/*ABSystem*/
static
int KickABSystem(void);

/*メモ
 * g_ab_h...ABのハンドラ
 * g_md_h...MDのハンドラ
 *
 * g_rc_data...RCのデータ
 */

static
int ArmABSystem(void);

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

  ret = KickABSystem();
  if( ret ){
    return ret;
  }

  ret = ArmABSystem();
  if( ret ){
    return ret;
  }

  return EXIT_SUCCESS;
}

/*プライベート キック用シリンダ*/
static
int KickABSystem(void){
  static uint8_t prs_lrc_s = 0; 
  if ( (__RC_ISPRESSED_L1(g_rc_data)) &&  
       (__RC_ISPRESSED_R1(g_rc_data)) &&  
       (__RC_ISPRESSED_CIRCLE(g_rc_data)) ) { 
    if (prs_lrc_s == 0){ 
      g_ab_h[DRIVER_AB].dat ^= KICK_AB_R;
      g_ab_h[DRIVER_AB].dat ^= KICK_AB_L; 
      prs_lrc_s = 1;
    } 
  } else { 
    prs_lrc_s = 0; 
  } 
  return EXIT_SUCCESS;
}

/*プライベート アーム展開*/
static
int ArmABSystem(void){
  /*TODO*/
  return EXIT_SUCCESS;
}

/*プライベート MD制御*/
static
int suspensionSystem(void){
  const int num_of_motor = DD_NUM_OF_MD;/*モータの個数*/
  const int analog_max = 15;
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
      
    case ROTATE_MECHA_MD:	
      gain = MD_GAIN / 2;
      rc_analogdata = 0;
      if ( (__RC_ISPRESSED_L1(g_rc_data)) 	&&
	   (__RC_ISPRESSED_R1(g_rc_data)) 	&&
	   (__RC_ISPRESSED_CROSS(g_rc_data)) 	&&
	  !(__RC_ISPRESSED_TRIANGLE(g_rc_data)) )
	rc_analogdata = -analog_max;
      
      if ( (__RC_ISPRESSED_L1(g_rc_data)) 	&&
	   (__RC_ISPRESSED_R1(g_rc_data)) 	&&
	   (__RC_ISPRESSED_TRIANGLE(g_rc_data)) &&
	  !(__RC_ISPRESSED_CROSS(g_rc_data)) )
	rc_analogdata = analog_max;
      break;
      
    case REEL_MECHA_MD:		/*リール機構用モータ*/
      rc_analogdata = 0;
      /*TODO*/
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

