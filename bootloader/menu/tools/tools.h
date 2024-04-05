#include "utils/types.h"


__attribute__((noreturn)) void wait_for_button_and_reboot(void);

int launch_payload(char*);

//Replace a pattern of string for another string
char *str_replace(char *orig, char *rep, char *with);
//tools
void strrep(char *orig, char *rep, char *with);
void SDStrap();
void BootStrapNX();
void printerCU(char *text,char *title,int clean);
u32 fusesB();
char *fusesM();
void *m_realloc(void* ptr, u32 current_size, u32 new_size);

bool cyper(char *inputFileName, char *outputFileName);

