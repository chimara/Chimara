#ifndef ZTERP_STACK_H
#define ZTERP_STACK_H

#include <stdint.h>

#include "io.h"

#define DEFAULT_STACK_SIZE	0x4000
#define DEFAULT_CALL_DEPTH	0x400

void init_stack(void);

uint16_t variable(uint16_t);
void store_variable(uint16_t, uint16_t);
uint16_t *stack_top_element(void);

void call(int);
#ifdef ZTERP_GLK
uint16_t direct_call(uint16_t);
#endif
void do_return(uint16_t);

int save_quetzal(zterp_io *, int);
int restore_quetzal(zterp_io *, int);

int do_save(int);
int do_restore(int);

int push_save(void);
int pop_save(void);

void zpush(void);
void zpull(void);
void zload(void);
void zstore(void);
void zret_popped(void);
void zpop(void);
void zcatch(void);
void zthrow(void);
void zret(void);
void zrtrue(void);
void zrfalse(void);
void zcheck_arg_count(void);
void zpop_stack(void);
void zpush_stack(void);
void zsave_undo(void);
void zrestore_undo(void);
void zsave(void);
void zrestore(void);

void zcall_store(void);
void zcall_nostore(void);

#define zcall		zcall_store
#define zcall_1n	zcall_nostore
#define zcall_1s	zcall_store
#define zcall_2n	zcall_nostore
#define zcall_2s	zcall_store
#define zcall_vn	zcall_nostore
#define zcall_vn2	zcall_nostore
#define zcall_vs2	zcall_store

#endif
