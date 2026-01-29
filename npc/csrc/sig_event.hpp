#pragma once
#include "vpi_user.h"
#include <vector>
#include <verilated.h>
#include <verilated_vpi.h>

#define CPU_VPI_PATH_PREFIX "TOP.asic.cpu.cpu."
struct ValidReadyBus {
	vpiHandle hValid;
	vpiHandle hReady;

	ValidReadyBus(vpiHandle hV, vpiHandle hR)
		:hValid(hV),hReady(hR){}

	bool shakeHappened() {
		s_vpi_value valValid, valReady;
		valValid.format = vpiIntVal;
		valReady.format = vpiIntVal;

		vpi_get_value(hValid, &valValid);
		vpi_get_value(hReady, &valReady);

		return (valValid.value.integer == 1) && (valReady.value.integer == 1);
	}
};

struct HandShakeDetector {
	std::vector<ValidReadyBus> bus_list;
	void add(std::string path){
	}
};
