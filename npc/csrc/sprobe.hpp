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
  std::vector<vpiHandle> _watched_handles;

  ~SProbe() {
    for (auto &h : _watched_handles) {
      vpi_release_handle(h);
    }
  }

  void watch_inside(vpiHandle top, int max_depth = 1, int cur_depth = 0) {
    if (cur_depth >= max_depth)
      return;

    vpiHandle iter;
    vpiHandle it;
    int type_begin = 0, type_end = 136;
    for (int type = type_begin; type < type_end; type++) {
      iter = vpi_iterate(type, top);
      if (iter != NULL) {
        // printf("SProbe scanning type %d\n",type);
        while ((it = vpi_scan(iter)) != NULL) {
          // printf("SProbe found %d  %s\n", type, _fullnames.back().c_str());
          if (type == vpiModule) {
            watch_inside(it, max_depth, cur_depth + 1);
            vpi_release_handle(it);
          } else {
            std::cout << "add watch " << vpi_get_str(vpiType, it) << " "
                      << vpi_get_str(vpiFullName, it) << std::endl;
            _watched_handles.push_back(it);
          }
          // printf("SProbe back to %s\n",vpi_get_str(vpiFullName,top));
        }
      }
    }
  }
  void watch_inside(std::string_view top) {
    watch_inside(vpi_handle_by_name((PLI_BYTE8 *)top.data(), NULL));
  }

  bool add_watch(std::string_view sig_name) {
    vpiHandle vh1 = vpi_handle_by_name((PLI_BYTE8 *)(sig_name.data()), NULL);
    if (!vh1) {
      std::cout << "SProbe add_watch no handle found for " << sig_name
                << std::endl;
      return false;
    }
    auto type = vpi_get(vpiType, vh1);
    if (type == vpiModule) {
      watch_inside(vh1);
      vpi_release_handle(vh1);
      return true;
    }
    _watched_handles.push_back(vh1);
    return true;
  }

  void dump_watched() {
    if (_watched_handles.empty())
      return;

#define ANSIFMT_NONE "\e[0m"
#define ANSIFMT_GRAY "\e[38;2;90;90;90m"
#define ANSIFMT_SIGNAL_REPEATED_PARENT "\e[38;2;84;118;138m"
#define ANSIFMT_SIGNAL_NAME "\e[38;2;156;220;254m"
#define ANSIFMT_NUM "\e[38;2;181;206;168m"
#define ANSIFMT_NUM_PREFIX "\e[38;2;113;129;105m"
#define ANSIFMT_COMMENT "\e[38;2;106;153;85m"
#define ANSIFMT_SIGNAL_TYPE "\e[38;2;78;201;176m"

    bool is_first = true;

    std::cout << ANSIFMT_COMMENT << "-- poke beg\n" << ANSIFMT_NONE;

    std::string last_parent = "";
    std::string selfname;

    std::string_view parent_colfmt;

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
      if (notop_name.starts_with("Top.")) {
        notop_name = notop_name.substr(4);
      }
      auto parent_end = notop_name.rfind('.');
      std::string_view parent = notop_name.substr(0, parent_end);
      selfname = notop_name.substr(parent_end + 1);

			if(selfname=="reset"||selfname=="clock"||selfname=="_RANDOM"){
				continue;
			}

      if (parent != last_parent) {
        last_parent = parent;
        parent_colfmt = ANSIFMT_SIGNAL_NAME;
      } else {
        parent_colfmt = ANSIFMT_SIGNAL_REPEATED_PARENT;
      }

      auto val_out_width = (sig_width + 3) / 4; // (8bits per 2hex) upceil

      if (!is_first) {
        std::cout << std::endl;
      } else {
        is_first = false;
      }

      std::cout << std::format(
          ANSIFMT_GRAY "Signal " ANSIFMT_NUM "{:2}W " ANSIFMT_SIGNAL_TYPE
                       "{} {}{}" ANSIFMT_SIGNAL_NAME ".{}" ANSIFMT_NONE
                       " = " ANSIFMT_NUM_PREFIX "h'" ANSIFMT_NUM
                       "{:0{}x}" ANSIFMT_NONE,
          sig_width, type, parent_colfmt, parent, selfname,
          (uint32_t)v.value.integer, val_out_width);
    }
    std::cout << ANSIFMT_COMMENT " -- end" ANSIFMT_NONE << std::endl;
  }
};
