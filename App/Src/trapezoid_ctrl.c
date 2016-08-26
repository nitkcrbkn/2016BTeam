#include <stdlib.h>
#include "SystemTaskManager.h"
#include "DD_MD.h"
#include "trapezoid_ctrl.h"

/*(目標制御値, インデックス,Duty上昇時の変化量, Duty下降の変化量)*/
void TrapezoidCtrl(int target_duty, DD_MDHand_t *handle, int inc_c, int dec_c){
  int prev_duty;/*直前のDuty*/
  int sd_duty = 0;

  prev_duty = handle->duty;
  if( handle->mode == D_MMOD_FORWARD ){/*直前が正回転なら*/
    if( target_duty > prev_duty ){
      sd_duty = prev_duty + _MIN(inc_c, target_duty - prev_duty);
    } else{
      sd_duty = prev_duty + _MAX(-dec_c, target_duty - prev_duty);
    }
  }else {/*直前が逆回転 or Freeなら*/
    if( target_duty < -prev_duty ){
      sd_duty = -prev_duty + _MAX(-inc_c, target_duty - ( -prev_duty ));
    } else{
      sd_duty = -prev_duty + _MIN(dec_c, target_duty - ( -prev_duty ));
    }
  }
  if( sd_duty > 0 ){
    handle->mode = D_MMOD_FORWARD;
  } else if( sd_duty < 0 ){
    handle->mode = D_MMOD_BACKWARD;
  } else{
    handle->mode = D_MMOD_FREE;
  }
  handle->duty = abs(sd_duty);
}

