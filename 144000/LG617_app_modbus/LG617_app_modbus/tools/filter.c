#include "filter.h"
#include "arm_math.h"

void InitKalmanInfo(KalmanInfo* kalmanInfo, double Q, double R, double filterValue)
{
	kalmanInfo->A = 1;  
	kalmanInfo->H = 1;  
	kalmanInfo->P = 10;  
	kalmanInfo->Q = Q;    
	kalmanInfo->R = R;    
	kalmanInfo->filterValue = filterValue;
}

double KalmanFilter(KalmanInfo* kalmanInfo, double lastMeasurement)
{
  //double preValue;
	double predictValue = kalmanInfo->A* kalmanInfo->filterValue;  
	
	kalmanInfo->P = kalmanInfo->A*kalmanInfo->A*kalmanInfo->P + kalmanInfo->Q;  //p(n|n-1)=A^2*p(n-1|n-1)+q
	//preValue = kalmanInfo->filterValue; 
 
	kalmanInfo->kalmanGain = kalmanInfo->P*kalmanInfo->H / (kalmanInfo->P*kalmanInfo->H*kalmanInfo->H + kalmanInfo->R);  //Kg(k)= P(k|k-1) H’ / (H P(k|k-1) H’ + R)

	kalmanInfo->filterValue = predictValue + (lastMeasurement - predictValue)*kalmanInfo->kalmanGain;  // X(k|k)= X(k|k-1)+Kg(k) (Z(k)-H X(k|k-1))

	kalmanInfo->P = (1 - kalmanInfo->kalmanGain*kalmanInfo->H)*kalmanInfo->P;//P[n|n]=(1-K[n]*H)*P[n|n-1]
 
	return  kalmanInfo->filterValue;
}
