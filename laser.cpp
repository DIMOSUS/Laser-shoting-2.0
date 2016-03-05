#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cstdarg>
#include <cmath> 
#include <fstream>
#include <iostream>
#include <iomanip> 
#include <sstream>
#include <queue>
#include <unistd.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/file.h>
#include <pthread.h> 
#include <termios.h>

#include "camera.h"
#include "gui.h"


using namespace std;
	
int32_t main (void)
{
	camerainit();
	
	if (calib())
	{
		initgui();
		
		double x, y, d;
		int32_t i = 0;
		int32_t sum = 0;
		while(1)
		{
			if (detect(&x, &y, &d))
			{
				if (d < 150)
				{
					int32_t points = d;
					if (points > 100) points = 100;
					int32_t P = 10-points/10;

					sum += P;
					cout << "shot " << i << " scr " << P << endl;
					
					if (i == 0)
						clear();
					
					addhole(x, y);
					
					addShotScoore(P, i, false);
					i++;
					if (i == 10)
					{
						addShotScoore(sum, i, true);
						cout << "all scr " << sum << endl << endl;
						i = 0;
						sum = 0;
						usleep(1500*1000);
					}
				}

				detect(&x, &y, &d);
				detect(&x, &y, &d);
				detect(&x, &y, &d);
				usleep(500*1000);
			}
		}
	}
}
