#ifndef MESUREMENTS_H
#define MESUREMENTS_H




void     Set_default_settings(void);
void     Angle_calculation(void);
float    ANSIN01_Convert_int_temp(uint16_t val);
float    ANSIN01_Convert_ext_temp(uint16_t val);
float    Get_angle_smpl_time(void);
void     Task_Measurement(void *handle);
void     Measurement_post_flag(void);
void     Init_measur_sync_obj(void);


#endif // MESUREMENTS_H



