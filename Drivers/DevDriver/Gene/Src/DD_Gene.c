 /* ===Kisarazu RBKN Library===
 *
 * autor          : Oishi
 * version        : v0.10
 * last update    : 20160703
 *
 * **overview***
 * 共通部分のコードと他のレイヤーとの通信部分
 *
 * ・I2Cのみのサポート
 */
#include <stdio.h>
#include <stdlib.h>
#include "app.h"
#include "MW_I2C.h"
#include "DD_Gene.h"
#include "DD_MD.h"
#include "DD_AB.h"
#include "message.h"

/*I2Cのサポート用関数*/
int DD_I2CSend(uint8_t add, const uint8_t *data, uint8_t size){
  return MW_I2C1Transmit(add, data, size);
}

/*DeviceDriverのタスク*/
int DD_doTasks(void){
  int i;
  int ret;
#if DD_NUM_OF_MD
  for( i = 0; i < DD_NUM_OF_MD; i++ ){
    ret = DD_send2MD(&g_md_h[i]);
    if( ret ){
      return ret;
    }
  }
#endif
#if DD_NUM_OF_AB
  for( i = 0; i < DD_NUM_OF_AB; i++ ){
    ret = DD_send2AB(&g_ab_h[i]);
    if( ret ){
      return ret;
    }
  }
#endif
  return EXIT_SUCCESS;
}

/*Deviceのハンドラー表示用関数*/
void DD_print(void){
  int i;
#if DD_NUM_OF_MD
  for( i = 0; i < DD_NUM_OF_MD; i++ ){
    DD_MDHandPrint(&g_md_h[i]);
  }
#endif
#if DD_NUM_OF_AB
  for( i = 0; i < DD_NUM_OF_AB; i++ ){
    DD_ABHandPrint(&g_ab_h[i]);
  }
#endif
}

/*初期化関数*/
int DD_initialize(void){
  int ret;
  
  /* Initialize all configured peripherals */
  MW_SetI2CClockSpeed(I2C1ID, _I2C_SPEED_BPS);
  ret = MW_I2CInit(I2C1ID);
  if( ret ){
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
