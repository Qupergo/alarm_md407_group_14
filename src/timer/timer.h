volatile long timer_ms;

//timer functions
void timer_default_irq( void );
void timer_start( void );
long timer_stop( void );
void timer_init( int ms );