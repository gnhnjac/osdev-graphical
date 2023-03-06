#define PHASE 100
//refs
void timer_phase(int hz);
void timer_handler(struct regs *r);
void timer_install();
void timer_wait(int seconds);
void wait_milliseconds(int milliseconds);