/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <common.h>
#include "monitor/sdb/sdb.h"
#include <errno.h>

void init_monitor(int, char *[]);
void am_init_monitor();
void engine_start();
int is_exit_status_bad();

int main(int argc, char *argv[]) {
  /* Initialize the monitor. */
#ifdef CONFIG_TARGET_AM
  am_init_monitor();
#else
  init_monitor(argc, argv);
#endif

  FILE* fp=fopen("./tools/gen-expr/test_51","r");
  if(fp==NULL)printf("err: %d\n", errno);
  assert(fp!=NULL);

  char str[114514];
  char sans[114];

  while(true){
    if(!fgets(sans,sizeof(sans),fp)) break;
    if(feof(fp)) break;
    if(!fgets(str,sizeof(str),fp))break;;
    uint32_t ans=atoi(sans);
    bool success=0;
    uint32_t result=expr(str,&success);
    if(!success) {
      printf("FAILED expr %s\n", str);
      continue;
    }
    if(result!=ans){
      printf("WA: %s\n", str);
      printf("expected: %u, get: %u\n", ans, result);
    }
  }

  fclose(fp);
  

  /* Start engine. */
  engine_start();


  return is_exit_status_bad();
}
