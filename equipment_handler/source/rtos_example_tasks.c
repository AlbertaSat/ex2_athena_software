
#include "rtos_example_tasks.h"

#include "HL_gio.h"
#include "HL_het.h"

#include <redconf.h>
#include <redfs.h>
#include <redfse.h>
#include <redposix.h>
#include <redtests.h>
#include <redvolume.h>

#include "FreeRTOS.h"
#include "integer.h"
#include "os_task.h"
#include "sd_io.h"
#include "printf.h"

/* Task1 */
 void vBlinkyTask(void *pvParameters)
{
    for(;;)
    {
        /* Taggle HET[1] with timer tick */
        gioSetBit(hetPORT1, 18, 1);
        vTaskDelay(100);
        gioSetBit(hetPORT1, 18, 0);
        vTaskDelay(100);
    }
}

 void testSDCard(int card_number)
{

    int32_t iErr;

    if(card_number == 1){
        const char *currentVolume = gaRedVolConf[0].pszPathPrefix;
    } else if (card_number == 2){
        const char *currentVolume = gaRedVolConf[1].pszPathPrefix;

    } else{
        printf("Invalid SD Card number");
    }

    iErr = red_init();
    if (iErr == -1)
    {
        printf("Unexpected error %d from red_init() on SD Card %d\r\n", (int)red_errno, card_number);
        exit(red_errno);
    }

    iErr = red_format(currentVolume);
    if (iErr == -1)
    {
        printf("Unexpected error %d from red_format() on SD Card %d\r\n", (int)red_errno, card_number);
        exit(red_errno);
    }

    iErr = red_mount(currentVolume);
    if (iErr == -1)
    {
        printf("Unexpected error %d from red_mount() on SD Card %d\r\n", (int)red_errno, card_number);
        exit(red_errno);
    }

    printf("Mounted SD Card %d\r\n", card_number);

    char buf[1024] = "";

    red_getcwd(buf, 1024);

    printf("CWD = %s\r\n", buf);

    iErr = red_mkdir("home");
    if (iErr == -1)
    {
        printf("Unexpected error %d from red_mkdir() on SD Card %d\r\n", (int)red_errno, card_number);
        exit(red_errno);
    }
    iErr = red_chdir("home");
    if (iErr == -1)
    {
        printf("Unexpected error %d from red_chdir() on SD Card %d\r\n", (int)red_errno, card_number);
        exit(red_errno);
    }

    red_getcwd(buf, 1024);

    printf("CWD = %s\r\n", buf);


    int32_t file1;

    file1 = red_open("test.txt", RED_O_RDWR | RED_O_CREAT);
    if (file1 == -1)
    {
        printf("Unexpected error %d from red_open() on SD Card %d\r\n", (int)red_errno, card_number);
        exit(red_errno);
    }

    iErr = red_write(file1, "8 7 6 5 4 3 2 1\r\n", strlen("8 7 6 5 4 3 2 1\r\n"));
    if (iErr == -1)
    {
        printf("Unexpected error %d from red_write() on SD Card %d\r\n", (int)red_errno, card_number);
        exit(red_errno);
    }
    else{
        printf("%d bytes written on SD Card %d\r\n", iErr, card_number);
    }

    iErr = red_close(file1);
    if (iErr == -1)
    {
        printf("Unexpected error %d from red_close() on SD Card %d\r\n", (int)red_errno, card_number);
        exit(red_errno);
    }

    file1 = red_open("damn.txt", RED_O_RDWR);
    if (file1 == -1)
    {
        printf("Unexpected error %d from red_open() on SD Card %d\r\n", (int)red_errno, card_number);
        exit(red_errno);
    }

    char readdata[30] = {0};

    iErr = red_read(file1, readdata, strlen("1 2 3 4 5 6 7 8\r\n"));
    if (iErr == -1)
    {
        printf("Unexpected error %d from red_read() on SD Card %d\r\n", (int)red_errno, card_number);
        exit(red_errno);
    }
    else{
        printf("%d B read\r\n", iErr);
    }

    iErr = red_close(file1);
    if (iErr == -1)
    {
        printf("Unexpected error %d from red_close() on SD Card %d\r\n", (int)red_errno, card_number);
        exit(red_errno);
    }
}

 void athena2FileSystemTest(void *pvParameters){

     testSDCard(1);//Do for SD Card 1 first
     testSDCard(2);//Then SD Card 2

 }
