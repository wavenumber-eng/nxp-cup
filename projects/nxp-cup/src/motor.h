int motor_init(void);
int motor_drive(int regi, int power, int duration, int active_group[4]);
int motor_interp(int regi, int start_pwr, int end_pwr, int step, int active_group[4]);
int motor_servo(int angle_srt, int angle_end);