// Include the most common headers from the C standard library
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

// Include the main libnx system header, for Switch development
#include <switch.h>
#include <curl/curl.h>

#define MAX_SWKBD_TEXT_LENGTH 500
#define MAX_PATH_LENGTH 256

size_t write_callback(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

SwkbdTextCheckResult validate_text(char *tmp_string, size_t tmp_string_size)
{
    if (strcmp(tmp_string, "bad") == 0)
    {
        strncpy(tmp_string, "Bad string.", tmp_string_size);
        return SwkbdTextCheckResult_Bad;
    }

    return SwkbdTextCheckResult_OK;
}

void inputText(char *str, uint len)
{
    SwkbdConfig kbd;
    Result rc = 0;
    rc = swkbdCreate(&kbd, 0);
    printf("  swkbdCreate(): 0x%x\n", rc);
    if (R_SUCCEEDED(rc))
    {
        swkbdConfigMakePresetDefault(&kbd);
        swkbdConfigSetTextCheckCallback(&kbd, validate_text);
        printf("  Running swkbdShow...\n");
        rc = swkbdShow(&kbd, str, len);
        printf("  swkbdShow(): 0x%x\n", rc);
        if (R_SUCCEEDED(rc))
        {
            printf("  out str: %s\n", str);
        }
        swkbdClose(&kbd);
    }
    return;
}

void downloadFromURL(char *durl, char *fn)
{
    printf("Start Download\n");
    consoleUpdate(NULL);

    printf("  curl ready\n");
    consoleUpdate(NULL);
    CURL *curl;
    FILE *fp;
    CURLcode res;
    curl = curl_easy_init();

    printf("  set download directory\n");
    char out[FILENAME_MAX] = "sdmc:/downloads/";
    strcat(out, fn);
    printf("\nURL = %s\nPath = %s\n\nDownload...\n", durl, out);
    consoleUpdate(NULL);

    printf("  open file handle\n");
    consoleUpdate(NULL);
    fp = fopen(out, "wb");

    printf("  set curl option\n");
    consoleUpdate(NULL);
    curl_easy_setopt(curl, CURLOPT_URL, durl);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);            // skipping cert. verification, if needed (ACTUALLY IS NEEDED FROM LIBNX)
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback); // writes data into pointer
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);                 // writes pointer into FILE *destination
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);            // support redirect

    printf("  run curl\n");
    consoleUpdate(NULL);
    res = curl_easy_perform(curl); // perform tasks curl_easy_setopt asked before
    if (res != CURLE_OK)
    {
        printf("\nfailed: %s\n", curl_easy_strerror(res));
    }

    printf("  curl cleanup\n");
    consoleUpdate(NULL);
    curl_easy_cleanup(curl); // always cleanup

    printf("  close file handle\n");
    consoleUpdate(NULL);
    fclose(fp); // closing FILE *fp

    printf("Done!\n");
    consoleUpdate(NULL);
}

int main(int argc, char *argv[])
{
    consoleInit(NULL);
    socketInitializeDefault();
    printf("  socket Init...\n");

    padConfigureInput(1, HidNpadStyleSet_NpadStandard);
    PadState pad;
    padInitializeDefault(&pad);

    printf("  curl Init...\n");
    curl_global_init(CURL_GLOBAL_DEFAULT); // initializing/calling functions

    printf("Press A to Download\n");
    printf("Press B to Exit\n");

    while (appletMainLoop())
    {
        consoleUpdate(NULL);
        padUpdate(&pad);
        u64 kDown = padGetButtonsDown(&pad);
        if (kDown & HidNpadButton_B)
            break;

        // Your code goes here
        if (kDown & HidNpadButton_A)
        {
            consoleClear();
            char url[MAX_SWKBD_TEXT_LENGTH] = {0};
            printf("Input URL\n");
            consoleUpdate(NULL);
            sleep(1);
            inputText(url, MAX_SWKBD_TEXT_LENGTH);

            char fn[MAX_PATH_LENGTH] = {0};
            printf("\nInput FileName\n");
            consoleUpdate(NULL);
            sleep(1);
            inputText(fn, MAX_PATH_LENGTH);

            printf("\nURL = %s\n", url);
            printf("FILENAME = %s\n\n", fn);
            consoleUpdate(NULL);

            downloadFromURL(url, fn);
            sleep(5);
            consoleClear();
            printf("Press A to Download\n");
            printf("Press B to Exit\n");
        }
    }

    // Deinitialize and clean up resources used by the console (important!)
    printf("curl Deinit...\n");
    curl_global_cleanup();
    printf("socket Deinit...\n");
    socketExit();
    consoleExit(NULL);
    return 0;
}
