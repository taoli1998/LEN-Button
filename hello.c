#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define trigger0 "/sys/class/leds/beaglebone:green:usr0/trigger"
#define trigger1 "/sys/class/leds/beaglebone:green:usr1/trigger"
#define trigger2 "/sys/class/leds/beaglebone:green:usr2/trigger"
#define trigger3 "/sys/class/leds/beaglebone:green:usr3/trigger"

#define Brightness0 "/sys/class/leds/beaglebone:green:usr0/brightness"
#define Brightness1 "/sys/class/leds/beaglebone:green:usr1/brightness"
#define Brightness2 "/sys/class/leds/beaglebone:green:usr2/brightness"
#define Brightness3 "/sys/class/leds/beaglebone:green:usr3/brightness"

#define button "/sys/class/gpio/gpio72/value"


void TriggerControl(char* trigger){
    FILE *pLedTriggerFile = fopen(trigger, "w");
    if (pLedTriggerFile == NULL) {
        printf("ERROR OPENING %s.", trigger);
        exit(1);
    }
    int charWritten = fprintf(pLedTriggerFile, "none");
    if (charWritten <= 0) {
        printf("ERROR WRITING DATA");
        exit(1);
    }
    fclose(pLedTriggerFile);
}

void TriggerBrightness(char* DA_TRIGGER_FILE_NAME_HERE, int value ){
    FILE *pLedBrightnessFile = fopen(DA_TRIGGER_FILE_NAME_HERE, "w");
    if (pLedBrightnessFile == NULL) {
        printf("ERROR OPENING %s.", DA_TRIGGER_FILE_NAME_HERE);
        exit(1);
    }
    int charWritten = fprintf(pLedBrightnessFile, "%d", value);
    if (charWritten <= 0) {
        printf("ERROR WRITING DATA");
        exit(1);
    }
    fclose(pLedBrightnessFile);
}

void GPIOWriting(float value){
    FILE *pFile = fopen("/sys/class/gpio/export", "w");
    if (pFile == NULL) {
        printf("ERROR: Unable to open export file.\n");
        exit(1);
    }
    fprintf(pFile, "%f", value);
    fclose(pFile);
}

int readFromFileToScreen(char *fileName)
{
    FILE *pFile = fopen(fileName, "r");
    if (pFile == NULL) {
        printf("ERROR: Unable to open file (%s) for read\n", fileName);
        exit(-1);
    }
    const int MAX_LENGTH = 1024;
    char buff[MAX_LENGTH];
    fgets(buff, MAX_LENGTH, pFile);
    fclose(pFile);
    //printf("Read: '%s'\n", buff);
    int value = atoi(buff);
    return value;
}

static long long getTimeInMs(void)
{
 struct timespec spec;
 clock_gettime(CLOCK_REALTIME, &spec);
 long long seconds = spec.tv_sec;
 long long nanoSeconds = spec.tv_nsec;
 long long milliSeconds = seconds * 1000
 + nanoSeconds / 1000000;
 return milliSeconds;
}

static void sleepForMs(long long delayInMs)
{
 const long long NS_PER_MS = 1000 * 1000;
 const long long NS_PER_SECOND = 1000000000;
 long long delayNs = delayInMs * NS_PER_MS;
 int seconds = delayNs / NS_PER_SECOND;
 int nanoseconds = delayNs % NS_PER_SECOND;
 struct timespec reqDelay = {seconds, nanoseconds};
 nanosleep(&reqDelay, (struct timespec *) NULL);
}

static void runCommand(char* command)
{
 FILE *pipe = popen(command, "r");
 char buffer[1024];
 while (!feof(pipe) && !ferror(pipe)) {
 if (fgets(buffer, sizeof(buffer), pipe) == NULL)
 break;
 }
 int exitCode = WEXITSTATUS(pclose(pipe));
 if (exitCode != 0) {
 perror("Unable to execute command:");
 printf(" command: %s\n", command);
 printf(" exit code: %d\n", exitCode);
 }
}

long long RandomNumber(int lower, int upper) 
{
    int num = (rand() % (upper - lower + 1)) + lower;
    return num;
}


int main()
{
    printf("Hello embedded world, from Tao!\n\n");
    runCommand("config-pin p8.43 gpio");
    runCommand("config-pin -q p8.43");
    runCommand("echo 72 > export");
    TriggerControl(trigger0);
    TriggerControl(trigger1);
    TriggerControl(trigger2);
    TriggerControl(trigger3);
    TriggerBrightness(Brightness0, 0);
    TriggerBrightness(Brightness1, 0);
    TriggerBrightness(Brightness2, 0);
    TriggerBrightness(Brightness3, 0);
    
    //GPIOWriting(72);
    long long resptime = 0;
    long long besttime = 0;
    printf("When LED3 lights up, press the USER buttom!\n");

    while(1){   
        if(readFromFileToScreen(button) == 0){
            while(readFromFileToScreen(button) == 0){};
            TriggerBrightness(Brightness0,1);
            TriggerBrightness(Brightness1,0);
            TriggerBrightness(Brightness2,0);
            TriggerBrightness(Brightness3,0);
        
            int lower = 500, upper = 3000;
            srand(time(0));
            long long randtim = RandomNumber(lower, upper);
            int b = 1;
            for (int i = 1; i<20;i++){
                long long a = randtim/20;
                if(readFromFileToScreen(button) == 0 && readFromFileToScreen(Brightness3)==0){
                    resptime = 5000;
                    if(besttime==0){
                        besttime = resptime;
                        printf("New best time!\n");
                    }
                    TriggerBrightness(Brightness0,1);
                    TriggerBrightness(Brightness1,1);
                    TriggerBrightness(Brightness2,1);
                    TriggerBrightness(Brightness3,1);
                    printf("Your rection time was %lld ms, best so far in game is %lld ms.\n", resptime, besttime);
                    b = 0;
                    while(readFromFileToScreen(button) == 0){};
                    break;
                }
                sleepForMs(a);               
            }
            if(b==1){
                TriggerBrightness(Brightness3,1);
                long long time1 = getTimeInMs();
                while(1){     
                    if(readFromFileToScreen(button) == 0&& getTimeInMs() - time1 <= 5000){
                        TriggerBrightness(Brightness0,1);
                        TriggerBrightness(Brightness1,1);
                        TriggerBrightness(Brightness2,1);
                        TriggerBrightness(Brightness3,1);
                        resptime = getTimeInMs()-time1;
                        if (besttime == 0 || besttime > resptime){
                            besttime = resptime;
                            printf("New best time!\n");
                        }
                        printf("Your rection time was %lld ms, best so far in game is %lld ms.\n", resptime, besttime);
                        while(readFromFileToScreen(button) == 0){};
                        break;
                    }
                else if(getTimeInMs() - time1 > 5000){
                    printf("No input within 5000 ms; quitting!\n");
                    return 1;
                }
            }

            }
            
            

        }
    }

    
 
    return 0;
}
