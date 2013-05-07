// HJI #include <stdarg.h>
// HJI #include <stdio.h>
// HJI #include <string.h>

// HJI #include "log.h"

//#include "usb_serial.h"
//#include "leds.h"
// HJI #include "ff.h"
// HJI #include "microsd_spi.h"

///////////////////////////////////////////////////////////////////////////////

#include "board.h"

///////////////////////////////////////////////////////////////////////////////

static uint16_t sd_card_available = 0;
static FATFS FATFS_Obj;
static FIL file;

void log_init(void)
{
    disk_initialize(0);
    delay(1); //FIX doubt this is actually the correct delay, but it's a one time call so it's probably ok.
    int result = disk_initialize(0);

    if (result == 0)
    {
        f_mount(0, &FATFS_Obj);

        char filename[200];
        {
            int filecounter = 0;
            FILINFO filetest;
            sprintf(filename, "0:log%05u.txt", filecounter);

            while (f_stat(filename, &filetest) == FR_OK)
            {
                filecounter++;
                sprintf(filename, "0:log%05u.txt", filecounter);
            }
        }

#ifdef DEBUG
        cliPrintF(" got to the open\n");
#endif

        int result = f_open(&file, filename, FA_CREATE_NEW | FA_WRITE);

        if (result != 0)
        {
            //led_fastBlink(LED_SDCARD);
#ifdef DEBUG
        	cliPrintF("SD failed at f_open\n");
#endif
            sd_card_available = 0;
            return;
        }
#ifdef DEBUG
        else
        {
        	cliPrintF("SD succeed - open file\n");
        }
#endif

        result = f_sync(&file);

        if (result != 0)
        {
            //led_fastBlink(LED_SDCARD);
#ifdef DEBUG
        	cliPrintF("SD failed at f_sync\n");
#endif
            sd_card_available = 0;
            return;
        }
#ifdef DEBUG
        else
        {
        	cliPrintF("SD succeed - sync file\n");
        }
#endif

        result = f_lseek(&file, file.fsize);

        if (result != 0)
        {
            //led_fastBlink(LED_SDCARD);
#ifdef DEBUG
        	cliPrintF("SD failed at f_lseek");
#endif
            sd_card_available = 0;
            return;
        }
#ifdef DEBUG
        else
        {
        	cliPrintF("SD succeed - lseek file\n");
        }
#endif

        //usb_printfi_buffered("SD filename: %s\n",filename);
        //led_on(LED_SDCARD);
#ifdef DEBUG
        cliPrintF("SD success filename: %s\n", filename);
#endif
        sd_card_available = 1;

    }

#ifdef DEBUG
    {
        //usb_printf_buffered("NO SD");
    }
#endif
}

void write_to_file(const char *fname, uint8_t *buffer, uint32_t length)
{
    if (sd_card_available == 0)
    {
        return;
    }

    static FIL file2;

    char filename[200];
    sprintf(filename, "0:%s", fname);
    int result = f_open(&file2, filename, FA_CREATE_ALWAYS | FA_WRITE);

    if (result != 0)
    {
        //led_fastBlink(LED_SDCARD);
#ifdef DEBUG
    	cliPrintF("SD failed at f_open:write\n");
#endif
        sd_card_available = 0;
        return;
    }

    unsigned int bw = 0;

    result = f_write(&file2, buffer, length, &bw);

    if (result != 0)
    {
        //led_fastBlink(LED_SDCARD);
#ifdef DEBUG
    	cliPrintF("SD failed at f_write:write\n");
#endif
        sd_card_available = 0;
        return;
    }

    if (bw != length)
    {
        //led_fastBlink(LED_SDCARD);
#ifdef DEBUG
    	cliPrintF("SD failed due to length mismatch:write\n");
#endif
        sd_card_available = 0;
        return;
    }

    result = f_close(&file2);

    if (result != 0)
    {
        //led_fastBlink(LED_SDCARD);
#ifdef DEBUG
    	cliPrintF("SD failed at f_close:write\n");
#endif
        sd_card_available = 0;
        return;
    }
}

void log_printf(const char *text, ...)
{
    char tmp[500];
    va_list args;
    va_start(args, text);
    vsnprintf(tmp, sizeof(tmp), text, args);
    va_end(args);

    if (sd_card_available == 0)
    {
        return;
    }

    char line[500];

    uint32_t mmillis = millis();
    //uint32_t millis = 1;
    uint32_t seconds = mmillis / 1000;
    uint32_t fract = mmillis - (seconds * 1000);

    snprintf(line, sizeof(line), "[%5lu.%05lu] %s", seconds, fract, tmp);
    unsigned int len = strlen(line);

    unsigned int bw = 0;

    unsigned int result = f_write(&file, line, len, &bw);

    if (result != 0)
    {
        //led_fastBlink(LED_SDCARD);
#ifdef DEBUG
    	cliPrintF("SD failed at f_write:logprintf\n");
#endif
        sd_card_available = 0;
        return;
    }

    if (bw != len)
    {
        //led_fastBlink(LED_SDCARD);
#ifdef DEBUG
    	cliPrintF("SD failed due to length mismatch:logprintf\n");
#endif
        sd_card_available = 0;
        return;
    }

    /*	result = f_sync(&file);

    	if(result != 0)
    	{
    		led_fastBlink(LED_SDCARD);
    		sd_card_available = 0;
    		return;
    	}
    */
}

void log_sync(void)
{
    if (sd_card_available == 0)
    {
        return;
    }

    unsigned int result = f_sync(&file);


    if (result != 0)
    {
        //led_fastBlink(LED_SDCARD);
#ifdef DEBUG
    	cliPrintF("SD failed at f_sync:log_sync\n");
#endif
        sd_card_available = 0;
        return;
    }
}
