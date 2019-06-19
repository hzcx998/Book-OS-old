#include "api.h"

void NNOSMain(){
	int i, timer;
	timer = api_alloctimer();
	api_inittimer(timer, 128);
	for (i = 20000000; i >= 20000; i -= i / 100) { //20KHz~20Hz，人?可听到的声音范?
		api_beep(i);							   //i以1%的速度??
		api_settimer(timer, 1);					   ////?隔?0.01秒
		if (api_getkey(1) != 128) {
			break;
		}
	}
	api_beep(0);
	api_return();
}
