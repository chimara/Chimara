#ifndef ABORT_H
#define ABORT_H

void interrupt_init();
void interrupt_free();
void cleanup();
void check_for_abort();
void signal_abort();

#endif

