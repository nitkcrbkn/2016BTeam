#ifndef __TRAPEZOID_CTRL_H
#define __TRAPEZOID_CTRL_H

typedef struct {
  int inc_con;	/*上昇時の変化量*/
  int dec_con;	/*下降時の変化量*/
} tc_const_t;

void TrapezoidCtrl(int target_duty, DD_MDHand_t *handle, tc_const_t idval);

#endif
