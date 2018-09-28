/*

This is a modified code based on "Example_TLCamera_C_UsingEvents_Color.c"
This code is currently set to arm the camera in Standard hardware trigger mode. See comments below to adjust
the code to set either software trigger (just like the "Example_TLCamera_C_UsingEvents_Color.c") or Bulb hardware mode.

The program waits for the camera to trigger, and when it does, it saves the image in a .TIF 16-bit file.
After the trigger, the program returns to the "wait state" and waits for the camera to trigger again (and saves another
file with a different file name).

The program will run and wait for triggers indefinitely unless terminated by typing "e" followed by enter.

Feel free to modify the code for your particular application. 

*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <MagickWand/MagickWand.h>
#include <direct.h>
#include "windows.h"
#include "tl_camera_sdk.h"
#include "tl_camera_sdk_load.h"
#include <conio.h>

HANDLE frame_acquired_event;
volatile int is_first_frame_finished = 0; // not using this, originally part of the example
int imageCount = 0;
time_t t;
struct tm * p;
int ch;
char s1[100];

// The callback that is registered with the camera
void frame_available_callback(void* sender, unsigned short* image_buffer, int image_width, int image_height, int bit_depth, int number_of_color_channels, int frame_count, void* context)
{
    if (frame_count == 1)
    {
        printf("image buffer = 0x%Ix\n", (int)image_buffer);
        printf("image width = %d\n", image_width);
        printf("image height = %d\n", image_height);
        printf("frame number = %d\n", frame_count);
    }

    // If you need to save the image data for application specific purposes, this would be the place to copy it into separate buffer.

    // Boot ImageMagick environment.
    MagickWandGenesis();
    MagickWand * wand;
    PixelWand * bgcolor;
    MagickBooleanType status;
    // Allocate & Initialize image.
    wand = NewMagickWand();
    bgcolor = NewPixelWand();
    if (wand == (MagickWand *)NULL) {
        fprintf(stderr, "Unable to allocate wand instance.\n");
    }
    if (bgcolor == (PixelWand *)NULL) {
        fprintf(stderr, "Unable to allocate pixel instance.\n");
    }
    PixelSetColor(bgcolor, "WHITE");
    MagickNewImage(wand, image_width, image_height, bgcolor);
    MagickTransformImageColorspace(wand, GRAYColorspace); // Kinda important

    // Import data as 16-bit gray (or single "red" channel).

    status = MagickImportImagePixels(wand, 0, 0, image_width, image_height, "R", ShortPixel, image_buffer);
    if (status == MagickFalse) {
        fprintf(stderr, "Unable to import device.\n");
    }

    // Create image file name
    // name of image has the date and time image was taken and a numerical identifier
    char s[100];
	char s2[100];
	char s3[100];

    t = time(NULL);
    p = localtime(&t);

    strftime(s, 1000, "%Y-%m-%d_%H-%M-%S", p);
    sprintf(s2, "_image%d.tiff", imageCount);
    // sprintf(s2, "_image%d.jpg", i);        // save jpg with compression for smaller file size
    strcat(s, s2);
    strcat(s3,s1);

    strcat(s3,"//");
    strcat(s3,s);




    // Write TIFF file.
    // MagickSetImageCompressionQuality(wand,50);     // compression set to 50 %
    MagickWriteImage(wand, s3);
    imageCount = imageCount + 1;
    // Clean-up.
    bgcolor = DestroyPixelWand(bgcolor);
    wand = DestroyMagickWand(wand);
    MagickWandTerminus();
    printf("Trigger received!\n");
    printf("Image %s saved in directory %s \n",s,s1);
    SetEvent(frame_acquired_event);

}



void camera_connect_callback(char* cameraSerialNumber, enum TL_CAMERA_USB_PORT_TYPE usb_bus_speed, void* context)
{
    printf("camera %s connected with bus speed = %d!\n", cameraSerialNumber, usb_bus_speed);
}

void camera_disconnect_callback(char* cameraSerialNumber, void* context)
{
    printf("camera %s disconnected!\n", cameraSerialNumber);
}

// Close the SDK, then the dll
int close_sdk_dll(void) {
    int ret = 0;

    if (tl_camera_close_sdk())
    {
        printf("Failed to close SDK!\n");
        ret = 1;
    }

    if (tl_camera_sdk_dll_terminate())
    {
        printf("Failed to close dll!\n");
        ret = 1;
    }

    if (!ret) printf("SDK & dll closed\n");

    return ret;
}

int main(int argc, const char * argv[])
{

    // Initializes dll
    if (tl_camera_sdk_dll_initialize())
    {
        printf("Failed to initialize dll!\n");
        return 1;
    }
    else printf("Successfully initialized dll\n");


    // Open the SDK
    if (tl_camera_open_sdk())
    {
        printf("Failed to open SDK!\n");
        tl_camera_sdk_dll_terminate();
        return 1;
    }
    else printf("Successfully opened SDK\n");


    // Set the camera connect event callback. This is used to register for run time camera connect events.
    if (tl_camera_set_camera_connect_callback(camera_connect_callback, 0))
    {
        printf("Failed to set camera connect callback!\n");
        close_sdk_dll();
        return 1;
    }

    // Set the camera disconnect event callback. This is used to register for run time camera disconnect events.
    if (tl_camera_set_camera_disconnect_callback(camera_disconnect_callback, 0))
    {
        printf("Failed to set camera disconnect callback!\n");
        close_sdk_dll();
        return 1;
    }

    char camera_ids[1024];

    void* camera_handle = 0;

    // Discover cameras.
    if (tl_camera_discover_available_cameras(camera_ids, 1024))
    {
        printf("Failed to get available cameras!\n");
        close_sdk_dll();
        return 1;
    }
    else printf("camera IDs: %s\n", camera_ids);

    // Check for no cameras.
    if (!strlen(camera_ids))
    {
        printf("Error: did not find any cameras!\n");
        close_sdk_dll();
        return 1;
    }

    // Camera IDs are separated by spaces.
    char* p_space = strchr(camera_ids, ' ');
    if (p_space)
    {
        *p_space = '\0'; // isolate the first detected camera
    }

    char first_camera[256];

    // Copy the ID of the first camera to separate buffer (for clarity)
    strcpy_s(first_camera, 256, camera_ids);

    printf("First camera_id = %s\n", first_camera);

    // Connect to the camera (get a handle to it).
    if (tl_camera_open_camera(first_camera, &camera_handle))
    {
        printf("Failed to open camera!\n");
        close_sdk_dll();
        return 1;
    }

    printf("Camera handle = 0x%Ix\n", (int)camera_handle);

    // Set the exposure
    long long exposure = atoll(argv[1]); // exposure time is an argument when you execute the program
    // for 10ms, for example, you run" "ArmCamera.exe 10000" units are in usec
    if (tl_camera_set_exposure_time(camera_handle, exposure)) printf("Failed to set exposure\n");
    else printf("Camera exposure set to %d\n", exposure);

    // Configure camera for continuous acquisition by setting the number of frames to 0.
    // I'm only taking one frame per trigger. You can change this if you want multiple frames per trigger
    tl_camera_set_frames_per_trigger_zero_for_unlimited(camera_handle, 1);

    // Set the image available callback
    tl_camera_set_frame_available_callback(camera_handle, frame_available_callback, 0);


    /**HARDWARE TRIGGER**/
    /*
        The alternative to software triggering. This is specified by tl_camera_set_operation_mode().
        By default the operation mode is TL_CAMERA_TRIGGER_TYPE_NONE, which is no hardware triggering.
        TL_CAMERA_TRIGGER_TYPE_STANDARD means for each hardware trigger the camera will take an image
        with exposure equal to the current value of tl_camera_get_exposure_time_us().
        TL_CAMERA_TRIGGER_TYPE_BULB means that exposure will be equal to the high pulse (or low, depending on polarity).
    */
    //// Set the trigger polarity for hardware triggers (ACTIVE_HIGH or ACTIVE_LOW)
    //if (tl_camera_set_trigger_polarity(camera_handle, TL_CAMERA_TRIGGER_POLARITY_ACTIVE_HIGH)) printf("Failed to set trigger polarity!\n");
    //// Set trigger mode

    // comment these two lines if you don't want the hardware trigger
    if (tl_camera_set_operation_mode(camera_handle, TL_CAMERA_TRIGGER_TYPE_STANDARD)) printf("Failed to set operation mode!\n");
    else printf("Hardware trigger mode activated\n");


    // Arm the camera.
    // if Hardware Triggering, make sure to set the operation mode before arming the camera.
    if (tl_camera_arm(camera_handle, 2)) printf("Failed to arm the camera!\n");
    else printf("Camera armed\n");



    // Create directory for storing images



    t = time(NULL);
    p = localtime(&t);

    strftime(s1, 1000, "%Y-%m-%d_%H-%M-%S", p);

    // name of directory is the current time and date when program executes
    mkdir(s1);  // create directory

    /**SOFTWARE TRIGGER**/
    /*
        Once the camera is initialized and armed, this function sends trigger command to camera over USB, GE, or CL.
        Camera will return images using a dummy thread to call frame_available_callback.
        Continuous acquisition is specified by setting the number of frames to 0 and issuing a single software trigger request.
    */

    // uncomment these two lines to use the software trigger insted of hardware trigger
 //   if (tl_camera_issue_software_trigger(camera_handle)) printf("Failed to start the camera!\n");
 //   else printf("Software trigger sent\n");


    // Wait to get an image from the frame available callback
    printf("Waiting for an image...\n");
    for (;;)
    {
        WaitForSingleObject(frame_acquired_event, INFINITE);
        if(kbhit()){
            ch = getch();
            if(ch == 'e')  // type 'e' followed by enter to quit program
                break;
        }
    }


    // Stop the camera.
    if (tl_camera_disarm(camera_handle))
    {
        printf("Failed to stop the camera!\n");
    }

    // Release the concurrent data structure resources.
    CloseHandle(frame_acquired_event);

    // Close the camera.
    if (tl_camera_close_camera(camera_handle))
    {
        printf("Failed to close camera!\n");
    }
    camera_handle = 0;

    // Free the C Camera SDK followed by the Unified SDK
    if (close_sdk_dll()) return 1;
    else return 0;

}

