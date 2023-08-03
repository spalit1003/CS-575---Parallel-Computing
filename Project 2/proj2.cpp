#include <stdio.h>
#include <iostream>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

using namespace std;

int	NowYear; // 2023 - 2028
int	NowMonth; // 0 - 11
float NowPrecip; // inches of rain per month
float NowTemp; // temperature this month
float NowHeight; // rye grass height in inches
int	NowNumRabbits; // number of rabbits in the current population
int NowNumCheckAgent; // added this variable

const float RYEGRASS_GROWS_PER_MONTH = 20.0;
const float ONE_RABBITS_EATS_PER_MONTH = 1.0;
const float AVG_PRECIP_PER_MONTH = 12.0; // average
const float AMP_PRECIP_PER_MONTH = 4.0;	// plus or minus
const float RANDOM_PRECIP =	2.0; // plus or minus noise
const float AVG_TEMP = 60.0; // average
const float AMP_TEMP = 20.0; // plus or minus
const float RANDOM_TEMP = 10.0;	// plus or minus noise
const float MIDTEMP = 60.0;
const float MIDPRECIP =	14.0;

unsigned int seed = 0;

omp_lock_t	Lock;
volatile int NumInThreadTeam;
volatile int NumAtBarrier;
volatile int NumGone;

float SQR(float x)
{
    return x * x;
}

float Ranf(unsigned int *seedp, float low, float high)
{
    float r = (float) rand_r(seedp); // 0 - RAND_MAX
    return(low + r * (high - low) / (float)RAND_MAX);
}

// specify how many threads will be in the barrier:
void InitBarrier( int n )
{
    NumInThreadTeam = n;
    NumAtBarrier = 0;
	omp_init_lock( &Lock );
}

void WaitBarrier( )
{
    omp_set_lock( &Lock );
    {
        NumAtBarrier++;
        if( NumAtBarrier == NumInThreadTeam )
        {
            NumGone = 0;
            NumAtBarrier = 0;
            // let all other threads get back to what they were doing
			// before this one unlocks, knowing that they might immediately
			// call WaitBarrier( ) again:
            while( NumGone != NumInThreadTeam-1 );
            omp_unset_lock( &Lock );
            return;
        }
     }
    omp_unset_lock( &Lock );
    while( NumAtBarrier != 0 );	// this waits for the nth thread to arrive
    #pragma omp atomic
    NumGone++;	// this flags how many threads have returned
}

void Rabbits() {
    while(NowYear < 2029)
    {
        int nextNumRabbits = NowNumRabbits;
        int carryingCapacity = (int)(NowHeight);
        if(nextNumRabbits < carryingCapacity)
            nextNumRabbits++;
        else if(nextNumRabbits > carryingCapacity)
            nextNumRabbits--;
        if(nextNumRabbits < 0) 
            nextNumRabbits = 0;

        WaitBarrier();  // Done Computing barrier
        NowNumRabbits = nextNumRabbits;

        WaitBarrier();   

        WaitBarrier();  // Done Printing barrier
    }
}

void RyeGrass() {
    while(NowYear < 2029)
    {
        float tempFactor = exp(-SQR((NowTemp - MIDTEMP) / 10.));
        float precipFactor = exp(-SQR((NowPrecip - MIDPRECIP) / 10.));
        float nextHeight = NowHeight;
        nextHeight += tempFactor * precipFactor * RYEGRASS_GROWS_PER_MONTH;
        nextHeight -= (float)NowNumRabbits * ONE_RABBITS_EATS_PER_MONTH;
        if(nextHeight < 0.) 
            nextHeight = 0.; 

        WaitBarrier(); // Done Computing barrier
        NowHeight = nextHeight;

        WaitBarrier(); // Done Assigning barrier

        WaitBarrier();  // Done Printing barrier
    }
}

void Watcher() {
    while(NowYear < 2029)
    {
        WaitBarrier();  // Done Computing barrier

        WaitBarrier();  // Done Assigning barrier:

        float ang = (30.*(float)NowMonth + 15.) * (M_PI / 180.);

        float temp = AVG_TEMP - AMP_TEMP * cos(ang);
        NowTemp = temp + Ranf(&seed, -RANDOM_TEMP, RANDOM_TEMP);

        float precip = AVG_PRECIP_PER_MONTH + AMP_PRECIP_PER_MONTH * sin(ang);
        NowPrecip = precip + Ranf(&seed, -RANDOM_PRECIP, RANDOM_PRECIP);

        if(NowPrecip < 0.)
            NowPrecip = 0.;

        if (NowMonth >= 11) 
        {
            NowYear++;
            NowMonth = 0;
        }
        else
            NowMonth++;

        float C_temperature = (5. / 9.) * (NowTemp - 32);
		float P_Height = NowPrecip * 2.54;
		float R_Height = NowHeight * 2.54;
		cout << NowMonth << "," << NowYear << "," << NowNumCheckAgent << "," 
            << NowNumRabbits << "," << R_Height << "," << C_temperature 
            << "," << P_Height << endl;

        WaitBarrier(); // Done Printing barrier:
    }
}

void MyAgent() {
    while(NowYear < 2029)
    {
        int nextNumZombie = NowNumCheckAgent;

        if (nextNumZombie >= (NowNumRabbits / 2))
            nextNumZombie--;
        else if (nextNumZombie < (NowNumRabbits / 2))
            nextNumZombie++;
        if (nextNumZombie < 0) nextNumZombie = 0;

        WaitBarrier(); // Done Computing barrier
        NowNumCheckAgent = nextNumZombie;

        WaitBarrier(); // Done Assigning barrier

        WaitBarrier(); // Done Printing barrier:
    }
}


int main () {
    // starting date and time:
    NowMonth =    0;
    NowYear  = 2023;

    // starting state (feel free to change this if you want):
    NowNumRabbits = 1;
    NowHeight =  5.;
    NowNumCheckAgent = 2;

    float ang = (30.*(float)NowMonth + 15.) * (M_PI / 180.);

    float temp = AVG_TEMP - AMP_TEMP * cos(ang);
    NowTemp = temp + Ranf(&seed, -RANDOM_TEMP, RANDOM_TEMP);

    float precip = AVG_PRECIP_PER_MONTH + AMP_PRECIP_PER_MONTH * sin(ang);
    NowPrecip = precip + Ranf(&seed, -RANDOM_PRECIP, RANDOM_PRECIP);
    if(NowPrecip < 0.) 
        NowPrecip = 0.;

    cout<< "Month,Year,NumZombie,NumRabbits,R_Height,Temperature,P_Height"<< endl;
    cout << NowMonth << "," << NowYear << "," << NowNumCheckAgent << "," 
            << NowNumRabbits << "," << NowHeight*2.54 << "," << (5. / 9.) * (NowTemp - 32) 
            << "," << NowPrecip*2.54 << endl; // Printing the initial values

    omp_set_num_threads(4);	// same as # of sections
    InitBarrier(4);

    #pragma omp parallel sections
    {
        #pragma omp section
        {
            Rabbits( );
        }

        #pragma omp section
        {
            RyeGrass( );
        }

        #pragma omp section
        {
            Watcher( );
        }

        #pragma omp section
        {
            MyAgent();
        }
    }       // implied barrier -- all functions must return in order
            // to allow any of them to get past here
    return 0;
}
