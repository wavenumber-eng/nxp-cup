int motor_init(void);
int motor_drive(int regi, int power, int duration, int active_group[4]);
int motor_interp(int regi, int start_pwr, int end_pwr, int step, int active_group[4]);
int motor_interp_servo(int angle_srt, int angle_end);
int motor_servo(int angle);

int motor_set_state_steer_tgt(int tgt);
int motor_set_state_drive_tgt(int tgt);