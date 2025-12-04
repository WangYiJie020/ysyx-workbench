#pragma once
#include "vpi_user.h"
#include <cstdint>
#include <format>
#include <iostream>
#include <string>
#include <vector>
#include <verilated.h>
#include <verilated_vpi.h>

class SProbe {
public:
  std::vector<std::string> _fullnames;

  std::vector<vpiHandle> _watched_handles;

  ~SProbe() {
    for (auto &h : _watched_handles) {
      vpi_release_handle(h);
    }
  }

  void load_inside(vpiHandle top) {
    // std::cout<<"SProbe load inside
    // "<<vpi_get_str(vpiFullName,top)<<std::endl;

    vpiHandle iter;
    vpiHandle it;
    int type_begin = 0, type_end = 136;
    for (int type = type_begin; type < type_end; type++) {
      iter = vpi_iterate(type, top);
      if (iter != NULL) {
        // printf("SProbe scanning type %d\n",type);
        while ((it = vpi_scan(iter)) != NULL) {
          _fullnames.push_back(std::string(vpi_get_str(vpiFullName, it)));
          // printf("SProbe found %d  %s\n", type, _fullnames.back().c_str());
          if (type == vpiModule)
            load_inside(it);
          vpi_release_handle(it);
          // printf("SProbe back to %s\n",vpi_get_str(vpiFullName,top));
        }
      }
    }
  }
  void load_inside(std::string_view top) {
    load_inside(vpi_handle_by_name((PLI_BYTE8 *)top.data(), NULL));
  }

  bool add_watch(std::string_view sig_name) {
    vpiHandle vh1 = vpi_handle_by_name((PLI_BYTE8 *)(sig_name.data()), NULL);
    if (!vh1) {
      std::cout << "SProbe add_watch no handle found for " << sig_name
                << std::endl;
      return false;
    }
		auto type = vpi_get(vpiType, vh1);
		if(type==vpiModule){
			std::cout<<"SProbe add_watch cannot watch module "<<sig_name<<std::endl;
			vpi_release_handle(vh1);
			return false;
		}
    _watched_handles.push_back(vh1);
    return true;
  }

  void dump_watched() {
    if (_watched_handles.empty())
      return;

#define ANSIFMT_NONE "\e[0m"
#define ANSIFMT_GRAY "\e[38;2;90;90;90m"
#define ANSIFMT_SIGNAL_NAME "\e[38;2;156;220;254m"
#define ANSIFMT_SIGNAL_WIDTH "\e[38;2;181;206;168m"
#define ANSIFMT_SIGNAL_TYPE "\e[38;2;78;201;176m"

    // std::cout << "===== SProbe Watched Signals =====" << std::endl;
    for (auto &h : _watched_handles) {
      s_vpi_value v;
      v.format = vpiIntVal;
      vpi_get_value(h, &v);
      std::string_view fullname = vpi_get_str(vpiFullName, h);
      std::string_view type = vpi_get_str(vpiType, h);
			auto sig_width = vpi_get(vpiSize, h);
      if (type.starts_with("vpi")) {
        type = type.substr(3);
      }
      // Remove the "TOP."
      auto notop_name = fullname.substr(4);
      auto first_modname = notop_name.substr(0, notop_name.find('.'));
      notop_name = notop_name.substr(first_modname.size() + 1);

			auto val_out_width=(sig_width+3)/4+2; // 0x + (8bits per 2hex) upceil

      std::cout << std::format(
          ANSIFMT_SIGNAL_TYPE "{} " ANSIFMT_SIGNAL_WIDTH "{:2}.W " ANSIFMT_GRAY
                              "{}." ANSIFMT_SIGNAL_NAME "{}" ANSIFMT_NONE
                              " = {:#0{}x}\n",
          type, sig_width, first_modname,notop_name, (uint32_t)v.value.integer,
					val_out_width
					);
    }
  }
};
