#include "singlepulse.h"

#include <QCoreApplication>
#include <iostream>
#include <iomanip>
#include <stdint.h>
#include <stdio.h>

/*#include "fescommon.h"
#include "fesinterface.h"
#include "fesmonitor.h"
#include "fesserial.h"
#include "festrajectorygenerator.h"*/

#include "feshandler.h"
#include <pthread.h>

#include "signal.h"

using namespace std;

#define DEFAULT_LOOP_TIME_NS 1000000L

#define MAX_CURRENT 0
#define TH_CURRENT 30

int fesF=40;
int fesT=1000/fesF;

bool runningFES =true;
bool stimulationON2 = true;

//float rampIndex[400] = {  0.00,   0.01,   0.02,   0.03,   0.04,   0.05,   0.06,   0.07,   0.08,   0.09,   0.10,   0.11,   0.12,   0.13,   0.14,   0.15,   0.16,   0.17,   0.18,   0.19,   0.20,   0.21,   0.22,   0.23,   0.24,   0.25,   0.26,   0.27,   0.28,   0.29,   0.30,   0.31,   0.32,   0.33,   0.34,   0.35,   0.36,   0.37,   0.38,   0.39,   0.40,   0.41,   0.42,   0.43,   0.44,   0.45,   0.46,   0.47,   0.48,   0.49,   0.51,   0.52,   0.53,   0.54,   0.55,   0.56,   0.57,   0.58,   0.59,   0.60,   0.61,   0.62,   0.63,   0.64,   0.65,   0.66,   0.67,   0.68,   0.69,   0.70,   0.71,   0.72,   0.73,   0.74,   0.75,   0.76,   0.77,   0.78,   0.79,   0.80,   0.81,   0.82,   0.83,   0.84,   0.85,   0.86,   0.87,   0.88,   0.89,   0.90,   0.91,   0.92,   0.93,   0.94,   0.95,   0.96,   0.97,   0.98,   0.99,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   1.00,   0.99,   0.99,   0.98,   0.98,   0.97,   0.97,   0.96,   0.96,   0.95,   0.95,   0.94,   0.94,   0.93,   0.93,   0.92,   0.92,   0.91,   0.91,   0.90,   0.90,   0.89,   0.89,   0.88,   0.88,   0.87,   0.87,   0.86,   0.86,   0.85,   0.85,   0.84,   0.84,   0.83,   0.83,   0.82,   0.82,   0.81,   0.81,   0.80,   0.80,   0.79,   0.79,   0.78,   0.78,   0.77,   0.77,   0.76,   0.76,   0.75,   0.75,   0.74,   0.74,   0.73,   0.73,   0.72,   0.72,   0.71,   0.71,   0.70,   0.70,   0.69,   0.69,   0.68,   0.68,   0.67,   0.67,   0.66,   0.66,   0.65,   0.65,   0.64,   0.64,   0.63,   0.63,   0.62,   0.62,   0.61,   0.61,   0.60,   0.60,   0.59,   0.59,   0.58,   0.58,   0.57,   0.57,   0.56,   0.56,   0.55,   0.55,   0.54,   0.54,   0.53,   0.53,   0.52,   0.52,   0.51,   0.51,   0.50,   0.50,   0.49,   0.49,   0.48,   0.48,   0.47,   0.47,   0.46,   0.46,   0.45,   0.45,   0.44,   0.44,   0.43,   0.43,   0.42,   0.42,   0.41,   0.41,   0.40,   0.40,   0.39,   0.39,   0.38,   0.38,   0.37,   0.37,   0.36,   0.36,   0.35,   0.35,   0.34,   0.34,   0.33,   0.33,   0.32,   0.32,   0.31,   0.31,   0.30,   0.30,   0.29,   0.29,   0.28,   0.28,   0.27,   0.27,   0.26,   0.26,   0.25,   0.25,   0.24,   0.24,   0.23,   0.23,   0.22,   0.22,   0.21,   0.21,   0.20,   0.20,   0.19,   0.19,   0.18,   0.18,   0.17,   0.17,   0.16,   0.16,   0.15,   0.15,   0.14,   0.14,   0.13,   0.13,   0.12,   0.12,   0.11,   0.11,   0.10,   0.10,   0.09,   0.09,   0.08,   0.08,   0.07,   0.07,   0.06,   0.06,   0.05,   0.05,   0.04,   0.04,   0.03,   0.03,   0.02,   0.02,   0.01,   0.01,   0.00};
float rampIndexCrescente[400]= {  0.00,   0.00,   0.01,   0.01,   0.01,   0.01,   0.02,   0.02,   0.02,   0.02,   0.03,   0.03,   0.03,   0.03,   0.04,   0.04,   0.04,   0.04,   0.05,   0.05,   0.05,   0.05,   0.06,   0.06,   0.06,   0.06,   0.07,   0.07,   0.07,   0.07,   0.08,   0.08,   0.08,   0.08,   0.09,   0.09,   0.09,   0.09,   0.10,   0.10,   0.10,   0.10,   0.11,   0.11,   0.11,   0.11,   0.12,   0.12,   0.12,   0.12,   0.13,   0.13,   0.13,   0.13,   0.14,   0.14,   0.14,   0.14,   0.15,   0.15,   0.15,   0.15,   0.16,   0.16,   0.16,   0.16,   0.17,   0.17,   0.17,   0.17,   0.18,   0.18,   0.18,   0.18,   0.19,   0.19,   0.19,   0.19,   0.20,   0.20,   0.20,   0.20,   0.21,   0.21,   0.21,   0.21,   0.22,   0.22,   0.22,   0.22,   0.23,   0.23,   0.23,   0.23,   0.24,   0.24,   0.24,   0.24,   0.25,   0.25,   0.25,   0.25,   0.26,   0.26,   0.26,   0.26,   0.27,   0.27,   0.27,   0.27,   0.28,   0.28,   0.28,   0.28,   0.29,   0.29,   0.29,   0.29,   0.30,   0.30,   0.30,   0.30,   0.31,   0.31,   0.31,   0.31,   0.32,   0.32,   0.32,   0.32,   0.33,   0.33,   0.33,   0.33,   0.34,   0.34,   0.34,   0.34,   0.35,   0.35,   0.35,   0.35,   0.36,   0.36,   0.36,   0.36,   0.37,   0.37,   0.37,   0.37,   0.38,   0.38,   0.38,   0.38,   0.39,   0.39,   0.39,   0.39,   0.40,   0.40,   0.40,   0.40,   0.41,   0.41,   0.41,   0.41,   0.42,   0.42,   0.42,   0.42,   0.43,   0.43,   0.43,   0.43,   0.44,   0.44,   0.44,   0.44,   0.45,   0.45,   0.45,   0.45,   0.46,   0.46,   0.46,   0.46,   0.47,   0.47,   0.47,   0.47,   0.48,   0.48,   0.48,   0.48,   0.49,   0.49,   0.49,   0.49,   0.50,   0.50,   0.50,   0.50,   0.51,   0.51,   0.51,   0.51,   0.52,   0.52,   0.52,   0.52,   0.53,   0.53,   0.53,   0.53,   0.54,   0.54,   0.54,   0.54,   0.55,   0.55,   0.55,   0.55,   0.56,   0.56,   0.56,   0.56,   0.57,   0.57,   0.57,   0.57,   0.58,   0.58,   0.58,   0.58,   0.59,   0.59,   0.59,   0.59,   0.60,   0.60,   0.60,   0.60,   0.61,   0.61,   0.61,   0.61,   0.62,   0.62,   0.62,   0.62,   0.63,   0.63,   0.63,   0.63,   0.64,   0.64,   0.64,   0.64,   0.65,   0.65,   0.65,   0.65,   0.66,   0.66,   0.66,   0.66,   0.67,   0.67,   0.67,   0.67,   0.68,   0.68,   0.68,   0.68,   0.69,   0.69,   0.69,   0.69,   0.70,   0.70,   0.70,   0.70,   0.71,   0.71,   0.71,   0.71,   0.72,   0.72,   0.72,   0.72,   0.73,   0.73,   0.73,   0.73,   0.74,   0.74,   0.74,   0.74,   0.75,   0.75,   0.75,   0.75,   0.76,   0.76,   0.76,   0.76,   0.77,   0.77,   0.77,   0.77,   0.78,   0.78,   0.78,   0.78,   0.79,   0.79,   0.79,   0.79,   0.80,   0.80,   0.80,   0.80,   0.81,   0.81,   0.81,   0.81,   0.82,   0.82,   0.82,   0.82,   0.83,   0.83,   0.83,   0.83,   0.84,   0.84,   0.84,   0.84,   0.85,   0.85,   0.85,   0.85,   0.86,   0.86,   0.86,   0.86,   0.87,   0.87,   0.87,   0.87,   0.88,   0.88,   0.88,   0.88,   0.89,   0.89,   0.89,   0.89,   0.90,   0.90,   0.90,   0.90,   0.91,   0.91,   0.91,   0.91,   0.92,   0.92,   0.92,   0.92,   0.93,   0.93,   0.93,   0.93,   0.94,   0.94,   0.94,   0.94,   0.95,   0.95,   0.95,   0.95,   0.96,   0.96,   0.96,   0.96,   0.97,   0.97,   0.97,   0.97,   0.98,   0.98,   0.98,   0.98,   0.99,   0.99,   0.99,   0.99,   1.00,   1.00};
float cyclingIndexes[360]= { 0,      1,      2,      3,      4,      5,      6,      7,      8,      9,     10,     11,     12,     13,     14,     15,     16,     17,     18,     19,     20,     21,     22,     23,     24,     25,     26,     27,     28,     29,     30,     31,     32,     33,     34,     35,     36,     37,     38,     39,     40,     41,     42,     43,     44,     45,     46,     47,     48,     49,     50,     51,     52,     53,     54,     55,     56,     57,     58,     59,     60,     61,     62,     63,     64,     65,     66,     67,     68,     69,     70,     71,     72,     73,     74,     75,     76,     77,     78,     79,     80,     81,     82,     83,     84,     85,     86,     87,     88,     89,     90,     91,     92,     93,     94,     95,     96,     97,     98,     99,    100,    101,    102,    103,    104,    105,    106,    107,    108,    109,    110,    111,    112,    113,    114,    115,    116,    117,    118,    119,    120,    121,    122,    123,    124,    125,    126,    127,    128,    129,    130,    131,    132,    133,    134,    135,    136,    137,    138,    139,    140,    141,    142,    143,    144,    145,    146,    147,    148,    149,    150,    151,    152,    153,    154,    155,    156,    157,    158,    159,    160,    161,    162,    163,    164,    165,    166,    167,    168,    169,    170,    171,    172,    173,    174,    175,    176,    177,    178,    179,    181,    182,    183,    184,    185,    186,    187,    188,    189,    190,    191,    192,    193,    194,    195,    196,    197,    198,    199,    200,    201,    202,    203,    204,    205,    206,    207,    208,    209,    210,    211,    212,    213,    214,    215,    216,    217,    218,    219,    220,    221,    222,    223,    224,    225,    226,    227,    228,    229,    230,    231,    232,    233,    234,    235,    236,    237,    238,    239,    240,    241,    242,    243,    244,    245,    246,    247,    248,    249,    250,    251,    252,    253,    254,    255,    256,    257,    258,    259,    260,    261,    262,    263,    264,    265,    266,    267,    268,    269,    270,    271,    272,    273,    274,    275,    276,    277,    278,    279,    280,    281,    282,    283,    284,    285,    286,    287,    288,    289,    290,    291,    292,    293,    294,    295,    296,    297,    298,    299,    300,    301,    302,    303,    304,    305,    306,    307,    308,    309,    310,    311,    312,    313,    314,    315,    316,    317,    318,    319,    320,    321,    322,    323,    324,    325,    326,    327,    328,    329,    330,    331,    332,    333,    334,    335,    336,    337,    338,    339,    340,    341,    342,    343,    344,    345,    346,    347,    348,    349,    350,    351,    352,    353,    354,    355,    356,    357,    358,    359,    360};
float cyclingIndexes_channels[360]={     1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      2,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      3,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4};
double size = 400;
int indexNow;
int indexNowMax = 399;
int indexNowCycling;
int indexNowCyclingMax=359;
double duration = 20000;
int pauseTime = 4000;

void myInterruptHandler (int signum){
    printf("ctrl-c has been pressed. Programs will be terminated in sequence.\n");
    runningFES=false;
}

timespec addition(timespec a, timespec b) {
    timespec r;

    if(a.tv_nsec + b.tv_nsec <= 999999999) {
        r.tv_nsec = a.tv_nsec + b.tv_nsec; //sommo i nsecondi
        r.tv_sec = a.tv_sec + b.tv_sec; //sommo i secondi
    }
    else {
        int c = (a.tv_nsec + b.tv_nsec)/1000000000;
        r.tv_nsec = a.tv_nsec + b.tv_nsec - 1000000000*c;
        r.tv_sec = a.tv_sec + b.tv_sec + c;
    }

    return r;
}

void *myThreadFunction(void* a)
{
    signal(SIGINT, myInterruptHandler);

    FesSerial serial_instance;

    struct timespec t_next; // defines time to wait before repeating thread loop
    struct timespec t_period; // define loop duration insiede thread

    unsigned long int loop_count = 0;
    byte current=0;

    t_period.tv_sec = 0;
    t_period.tv_nsec = DEFAULT_LOOP_TIME_NS; //1ms
    // Running si ripete ogni milli secondo

    clock_gettime(CLOCK_MONOTONIC, &t_next); // t_next = tempoOra --> get time retrieves the time of the clock

    while (runningFES){
        t_next = addition(t_next, t_period); // update t_next (needed for usleep at the end)--> sommo al tempo attuale il tperiod (+1ms)

        loop_count++; //ogni 1ms aggiorno loop count

        if( (loop_count%fesT == 0) && stimulationON2) // repeats every fesT (25ms for 40Hz)--> se entro in questo if vuol dire che sono passati 25ms
        {



            /**********RAMPA METODO 1********/
            if (loop_count%1000==0){
                current ++;
            }

            serial_instance.fes_State.active[0]= true;
            serial_instance.fes_State.stim_value[0]= current;

            cout << "time = "<<loop_count/1000.000<<endl;
            cout << "current = "<<current<<endl;
            cout << "stim value ="<<serial_instance.fes_State.stim_value[0]<<endl;
            cout << endl;
            cout << endl;


            if (current>=TH_CURRENT){
                runningFES=false;
                stimulationON2=false;
                loop_count=0;
                cout << "threshold reached"<<endl;
            }

            else {

                serial_instance.createChannelTrajectory();

                serial_instance.setChannels();

                serial_instance.createMsg();
            }

        }

        /**********STIMULATION fixed value*****************/
//            for(int ch=0;ch<NUMBER_OF_CHANNELS;ch++){
//                serial_instance.fes_State.active[ch]= true;
//                serial_instance.fes_State.stim_value[ch]= 3;
//            }

//            serial_instance.createChannelTrajectory();

//            serial_instance.setChannels();

//            serial_instance.createMsg();

//        }



//       /**********RAMPA METODO 2**************************/
//            indexNow = (int) ((loop_count/duration) * size);
//            if(indexNow >= indexNowMax) {
//                stimulationON2 = false;
//                runningFES=false;
//                loop_count = 0; //ricomincio da stim 0 così
//                indexNow = indexNowMax;
//            }






            clock_nanosleep (CLOCK_MONOTONIC, TIMER_ABSTIME, &t_next, nullptr ); //dormi per 1ms --> e ricomincio
    }
}


void Pulse::startFES() {
    pthread_t thread;
    pthread_create(&thread, NULL, myThreadFunction, nullptr);
    pthread_join(thread, NULL);

}
