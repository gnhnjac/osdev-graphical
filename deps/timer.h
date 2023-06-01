#define PHASE 100
//refs
void timer_phase(int hz);
void timer_handler();
void timer_install();
void timer_wait(int seconds);
void wait_milliseconds(int milliseconds);
void set_time(unsigned int hour, unsigned int minute);
void update_time();