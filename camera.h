#ifndef CAMERA
#define CAMERA
extern void camerainit(void);
extern bool calib(void);
extern bool detect(double *x, double *y, double *d);
#endif