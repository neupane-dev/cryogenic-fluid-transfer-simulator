#ifndef CRYO_CONTROLLER
#define CRYO_CONTROLLER

#include "cryo_defs.h"

//Public API functions
bool CRYO_check_safety_invariants(CryoSystem_t *sys);
void CRYO_run_control_logic(CryoSystem_t *sys);


#endif