#pragma once
#include "vpi_user.h"
#include <cstdint>
#include <format>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <verilated.h>
#include <verilated_vpi.h>

#define ANSIFMT_NONE "\e[0m"
#define ANSIFMT_GRAY "\e[38;2;90;90;90m"
#define ANSIFMT_SIGNAL_REPEATED_PARENT "\e[38;2;84;118;138m"
#define ANSIFMT_SIGNAL_NAME "\e[38;2;156;220;254m"
#define ANSIFMT_NUM "\e[38;2;181;206;168m"
#define ANSIFMT_NUM_PREFIX "\e[38;2;113;129;105m"
#define ANSIFMT_COMMENT "\e[38;2;106;153;85m"
#define ANSIFMT_HINT "\e[38;2;220;220;170m"
#define ANSIFMT_SIGNAL_TYPE "\e[38;2;78;201;176m"

class SProbe {
public:
  struct WatchItem {
    vpiHandle handle;
    uint64_t last_value;
    auto getValue() {
      s_vpi_value v;
      v.format = vpiIntVal;
      vpi_get_value(handle, &v);
      return v.value.integer;
    }
    void updateLastValue() { last_value = getValue(); }
    std::string getFullname() { return vpi_get_str(vpiFullName, handle); }
    std::string getType() { return vpi_get_str(vpiType, handle); }
    std::string getName() { return vpi_get_str(vpiName, handle); }
    auto getSize() { return vpi_get(vpiSize, handle); }
    WatchItem(vpiHandle h) {
      handle = h;
      last_value = getValue();
    }
  };
  std::vector<WatchItem> _watched;

  ~SProbe() {
    for (auto &w : _watched) {
      vpi_release_handle(w.handle);
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
          // printf("SProbe found %d  %s\n", type, vpi_get_str(vpiFullName,
          // it));
          if (type == vpiModule) {
            watch_inside(it, max_depth, cur_depth + 1);
            vpi_release_handle(it);
          } else {
            std::cout << "add watch " << vpi_get_str(vpiType, it) << " "
                      << vpi_get_str(vpiFullName, it) << std::endl;
            _watched.emplace_back(it);
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
    _watched.emplace_back(vh1);
    return true;
  }

  void dump_watched() {
    if (_watched.empty())
      return;

    bool is_first = true;

    std::cout << ANSIFMT_COMMENT << "-- poke beg\n" << ANSIFMT_NONE;

    std::string last_parent = "";

    std::string_view parent_colfmt;

    auto val_upd_hint = ANSIFMT_HINT "*" ANSIFMT_NONE;

    for (auto &h : _watched) {
      auto fullname = h.getFullname();
      auto selfname = h.getName();
      auto type = h.getType();
      auto sig_width = h.getSize();

      if (type.starts_with("vpi")) {
        type = type.substr(3);
      }

      // Remove the "TOP."
      auto notop_name = fullname.substr(4);
      if (notop_name.starts_with("Top.")) {
        notop_name = notop_name.substr(4);
      }
      auto parent_end = notop_name.rfind('.');
      auto parent = notop_name.substr(0, parent_end);

      // if (selfname == "reset" || selfname == "clock" || selfname ==
      // "_RANDOM") {
      //   continue;
      // }

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

      auto sig_value = h.getValue();
      bool value_changed = (sig_value != h.last_value);

      std::cout << std::format(
          ANSIFMT_GRAY "si{} " ANSIFMT_SIGNAL_TYPE "{} " ANSIFMT_NUM "{:2} "
                       "{}{}" ANSIFMT_SIGNAL_NAME ".{}" ANSIFMT_NONE
                       " = " ANSIFMT_NUM_PREFIX "h'" ANSIFMT_NUM
                       "{:0{}x}" ANSIFMT_NONE,
          value_changed ? val_upd_hint : "g", type[0], sig_width, parent_colfmt,
          parent, selfname, (uint32_t)sig_value, val_out_width);
      h.updateLastValue();
    }
    std::cout << ANSIFMT_COMMENT " -- end" ANSIFMT_NONE << std::endl;
  }
};
