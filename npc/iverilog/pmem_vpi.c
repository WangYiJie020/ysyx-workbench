#define _POSIX_C_SOURCE 200809L

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <vpi_user.h>

#define PMEM_BASE UINT32_C(0x80000000)
#define PMEM_SIZE (128u * 1024u * 1024u)
#define UART_ADDR UINT32_C(0x10000000)
#define RTC_ADDR UINT32_C(0x02000048)

static unsigned char *pmem;
static unsigned char *pmem_xmask;
static uint64_t boot_time_us;

static uint64_t now_us(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (uint64_t)ts.tv_sec * UINT64_C(1000000) + (uint64_t)ts.tv_nsec / 1000;
}

static void get_vec32(vpiHandle arg, uint32_t *value, uint32_t *xmask) {
  s_vpi_value raw = {.format = vpiVectorVal};
  vpi_get_value(arg, &raw);
  *value = (uint32_t)raw.value.vector[0].aval;
  *xmask = (uint32_t)raw.value.vector[0].bval;
}

static int get_known_u32(vpiHandle arg, uint32_t *value, const char *name) {
  uint32_t xmask;
  get_vec32(arg, value, &xmask);
  if (xmask) {
    vpi_printf("ERROR: $pmem access has unknown %s (X mask 0x%08" PRIx32 ")\n",
               name, xmask);
    exit(EXIT_FAILURE);
    return 0;
  }
  return 1;
}

static void put_vec32(vpiHandle arg, uint32_t value, uint32_t xmask) {
  s_vpi_vecval vector = {.aval = (PLI_UINT32)value, .bval = (PLI_UINT32)xmask};
  s_vpi_value out = {.format = vpiVectorVal};
  out.value.vector = &vector;
  vpi_put_value(arg, &out, NULL, vpiNoDelay);
}

static void read_word(uint32_t addr, uint32_t *value, uint32_t *xmask) {
  *xmask = 0;
  if (addr == RTC_ADDR || addr == RTC_ADDR + 4) {
    uint64_t elapsed = now_us() - boot_time_us;
    *value = addr == RTC_ADDR ? (uint32_t)elapsed : (uint32_t)(elapsed >> 32);
    return;
  }
  if (addr >= PMEM_BASE && addr - PMEM_BASE <= PMEM_SIZE - 4) {
    uint32_t offset = (addr - PMEM_BASE) & ~UINT32_C(3);
    *value = (uint32_t)pmem[offset] | ((uint32_t)pmem[offset + 1] << 8) |
             ((uint32_t)pmem[offset + 2] << 16) |
             ((uint32_t)pmem[offset + 3] << 24);
    *xmask = (uint32_t)pmem_xmask[offset] |
             ((uint32_t)pmem_xmask[offset + 1] << 8) |
             ((uint32_t)pmem_xmask[offset + 2] << 16) |
             ((uint32_t)pmem_xmask[offset + 3] << 24);
    return;
  }
  vpi_printf("WARNING: unmapped read at 0x%08" PRIx32 " returns zero\n", addr);
  *value = 0;
}

static PLI_INT32 pmem_read_calltf(PLI_BYTE8 *user_data) {
  vpiHandle call = vpi_handle(vpiSysTfCall, NULL);
  vpiHandle args = vpi_iterate(vpiArgument, call);
  vpiHandle addr_arg = vpi_scan(args);
  vpiHandle data_arg = vpi_scan(args);
  uint32_t addr, data, xmask;
  (void)user_data;
  if (get_known_u32(addr_arg, &addr, "read address")) {
    read_word(addr, &data, &xmask);
    put_vec32(data_arg, data, xmask);
  }
  return 0;
}

static PLI_INT32 pmem_write_calltf(PLI_BYTE8 *user_data) {
  vpiHandle call = vpi_handle(vpiSysTfCall, NULL);
  vpiHandle args = vpi_iterate(vpiArgument, call);
  vpiHandle addr_arg = vpi_scan(args);
  vpiHandle data_arg = vpi_scan(args);
  vpiHandle mask_arg = vpi_scan(args);
  uint32_t addr, data, data_xmask, mask;
  unsigned lane;
  (void)user_data;
  if (!get_known_u32(addr_arg, &addr, "write address") ||
      !get_known_u32(mask_arg, &mask, "write mask")) {
    return 0;
  }
  get_vec32(data_arg, &data, &data_xmask);
  if (addr == UART_ADDR) {
    if (data_xmask & UINT32_C(0xff)) {
      vpi_printf("ERROR: UART write data is unknown\n");
      exit(EXIT_FAILURE);
    }
    putchar((int)(data & 0xff));
    fflush(stdout);
    return 0;
  }
  if (addr < PMEM_BASE || addr - PMEM_BASE > PMEM_SIZE - 4) {
    vpi_printf("ERROR: unmapped write at 0x%08" PRIx32 "\n", addr);
    exit(EXIT_FAILURE);
    return 0;
  }
  addr = (addr - PMEM_BASE) & ~UINT32_C(3);
  for (lane = 0; lane < 4; ++lane) {
    if (mask & (1u << lane)) {
      pmem[addr + lane] = (unsigned char)(data >> (lane * 8));
      pmem_xmask[addr + lane] = (unsigned char)(data_xmask >> (lane * 8));
    }
  }
  return 0;
}

static PLI_INT32 load_image_cb(p_cb_data cb_data) {
  s_vpi_vlog_info info;
  const char *image = NULL;
  FILE *fp;
  long size;
  int i;
  (void)cb_data;
  vpi_get_vlog_info(&info);
  for (i = 0; i < info.argc; ++i) {
    if (strncmp(info.argv[i], "+image=", 7) == 0) image = info.argv[i] + 7;
  }
  if (!image || !*image) {
    vpi_printf("ERROR: missing +image=<binary>\n");
    exit(EXIT_FAILURE);
    return 0;
  }
  pmem = calloc(PMEM_SIZE, 1);
  pmem_xmask = calloc(PMEM_SIZE, 1);
  fp = fopen(image, "rb");
  if (!pmem || !pmem_xmask || !fp) {
    vpi_printf("ERROR: cannot allocate memory or open image '%s'\n", image);
    exit(EXIT_FAILURE);
    return 0;
  }
  fseek(fp, 0, SEEK_END);
  size = ftell(fp);
  rewind(fp);
  if (size < 0 || (unsigned long)size > PMEM_SIZE ||
      fread(pmem, 1, (size_t)size, fp) != (size_t)size) {
    vpi_printf("ERROR: cannot load image '%s'\n", image);
    fclose(fp);
    exit(EXIT_FAILURE);
    return 0;
  }
  fclose(fp);
  boot_time_us = now_us();
  vpi_printf("Loaded %ld bytes from %s at 0x%08" PRIx32 "\n", size, image, PMEM_BASE);
  return 0;
}

static void register_pmem(void) {
  s_vpi_systf_data read_tf = {.type = vpiSysTask, .tfname = "$pmem_read",
                              .calltf = pmem_read_calltf};
  s_vpi_systf_data write_tf = {.type = vpiSysTask, .tfname = "$pmem_write",
                               .calltf = pmem_write_calltf};
  s_cb_data cb = {.reason = cbStartOfSimulation, .cb_rtn = load_image_cb};
  vpi_register_systf(&read_tf);
  vpi_register_systf(&write_tf);
  vpi_register_cb(&cb);
}

void (*vlog_startup_routines[])(void) = {register_pmem, 0};
