/************************************************************
 * Check license.txt in project root for license information *
 *********************************************************** */

#if 1

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <stdio.h>
#include <X11/Xutil.h>
#include <X11/Xlib.h>
#include <X11/X.h>
#include <X11/extensions/Xinerama.h>
#include <X11/extensions/Xdbe.h>
#include <time.h>
#include <unistd.h>
#include "defs.h"
#include "fileutils.h"
#include "cmath.h"

static int _XlibErrorHandler(Display *display, XErrorEvent *event) {

    char code[1024];
    XGetErrorText(display, event->error_code, code, 1024);

    fprintf(stderr, "error: %s\n", code);
    return True;
}


static clock_t g_timers[10];
int g_currentTimer = 0;

void
timer_start() {

    g_timers[g_currentTimer] = clock();
    g_currentTimer += 1;
}

void
timer_end(char* msg) {

    clock_t t = clock() - g_timers[g_currentTimer - 1];
    double time_taken = ((double)t)/CLOCKS_PER_SEC; // in seconds

    printf("%s took %f seconds to execute \n", msg, time_taken);
    g_currentTimer -= 1;
}

void
clipboard_deny(Display *display, XSelectionRequestEvent *sev)
{
    XSelectionEvent ssev;
    char *an;

    an = XGetAtomName(display, sev->target);
    printf("Denying request of type '%s'\n", an);
    if (an)
        XFree(an);

    /* All of these should match the values of the request. */
    ssev.type = SelectionNotify;
    ssev.requestor = sev->requestor;
    ssev.selection = sev->selection;
    ssev.target = sev->target;
    ssev.property = None;  /* signifies "nope" */
    ssev.time = sev->time;

    XSendEvent(display, sev->requestor, True, NoEventMask, (XEvent *)&ssev);
}

void atom_print(Display* display, Atom atom) {

    char* an = XGetAtomName(display, atom);
    printf("atom name '%s'\n", an);
    if (an)
        XFree(an);
}

void
clipboard_accept(Display *display, XSelectionRequestEvent *sev, Atom image, u8* data, u32 size) {
    XSelectionEvent ssev;
    /*
       time_t now_tm;
       char *now, *an;

       now_tm = time(NULL);
       now = ctime(&now_tm);
       */
    char* an = XGetAtomName(display, sev->property);
    printf("Sending data to window 0x%lx, property '%s', size %d\n", sev->requestor, an, size);
    if (an)
        XFree(an);


    if(size > KILOS(256)) {
        // send chunks in INCR loop
        printf("sending chunked data...\n");
        Atom incrAtom = XInternAtom(display, "INCR", False);
        atom_print(display, sev->selection);
        atom_print(display, sev->target);
        atom_print(display, sev->property);

        /*
           atom name 'CLIPBOARD'
           atom name 'image/jpeg'
           atom name 'XSEL_DATA'
           */

        //Send incr atom
        int temp = 10;
        XChangeProperty(display, sev->requestor, sev->property, incrAtom, 32, PropModeReplace,
                (u8*)&temp, 1);

        ssev.type = SelectionNotify;
        ssev.requestor = sev->requestor;
        ssev.selection = sev->selection;
        ssev.target = sev->target;
        ssev.property = sev->property;
        ssev.time = sev->time;

        XSendEvent( display, sev->requestor, True, NoEventMask, (XEvent *)&ssev);

        Atom targetAtom = sev->property;
        Window requestorWindow = sev->property;
        (void)requestorWindow;
        // Listen for the clients property changes
        XSelectInput(display, sev->requestor, PropertyChangeMask);

        printf("waiting for event...\n");

        XEvent event;
        XSelectionEvent ssev;
        while(size > 0) {

            // Wait until client has red the data
            do {
                XNextEvent(display, &event);
            } while (event.type != PropertyNotify ||
                    event.xproperty.atom != targetAtom ||
                    event.xproperty.state != PropertyDelete /*||
                                                              event.xproperty.window != requestorWindow */);

                //sleep(3);
                // send new data
                printf("got new selection request!!!!\n");

            atom_print(display, sev->property);

            u32 newPackageLen = size > KILOS(256) ? KILOS(256) : size;

            printf("sending size  AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA %d\n", newPackageLen);

            XChangeProperty(display, sev->requestor, sev->property, image, 8, PropModeReplace,
                    (unsigned char *) (data), newPackageLen);

            data += newPackageLen;
            size = size > KILOS(256) ? size - KILOS(256) : 0;

            ssev.type = SelectionNotify;
            ssev.requestor = sev->requestor;
            ssev.selection = sev->selection;
            ssev.target = sev->target;
            ssev.property = sev->property;
            ssev.time = sev->time;

            XSendEvent(display, sev->requestor, True, NoEventMask, (XEvent *)&ssev);
        }
        // Send zero lenght packet to message the end

        // Wait until client has red the data


        do {
            XNextEvent(display, &event);
        } while (event.type != PropertyNotify ||
                event.xproperty.atom != targetAtom ||
                event.xproperty.state != PropertyDelete /*||
                                                          event.xproperty.window != requestorWindow*/);
            //sleep(10);
            /*
               do {
               XNextEvent(display, &event);
               } while (event.type == PropertyNotify &&
               event.xproperty.atom == targetAtom &&
               event.xproperty.state == PropertyDelete);
               */

            //sleep(4);
            printf("SEND END!\n");
        atom_print(display, sev->property);

        printf("sending size %d\n", 0);
        XChangeProperty(display, sev->requestor, sev->property, image, 8, PropModeReplace,
                (unsigned char *) (data), 0);

        ssev.type = SelectionNotify;
        ssev.requestor = sev->requestor;
        ssev.selection = sev->selection;
        ssev.target = sev->target;
        ssev.property = sev->property;
        ssev.time = sev->time;

        XSendEvent(display, sev->requestor, True, NoEventMask, (XEvent *)&ssev);

    } else {
        //just send the hole image as one

        XChangeProperty(display, sev->requestor, sev->property, image, 8, PropModeReplace,
                data, size);

        ssev.type = SelectionNotify;
        ssev.requestor = sev->requestor;
        ssev.selection = sev->selection;
        ssev.target = sev->target;
        ssev.property = sev->property;
        ssev.time = sev->time;

        XSendEvent(display, sev->requestor, True, NoEventMask, (XEvent *)&ssev);
    }

}

void
clipboard_send_targets(Display *display, XSelectionRequestEvent *sev, Atom target, Atom* targets,
        u32 numTargets) {

    XSelectionEvent ssev;
    char* tempname = XGetAtomName(display, sev->property);
    printf("Sending data to window 0x%lx, property '%s', size %d\n",
            sev->requestor,
            tempname,
            (u32)(sizeof(Atom) * numTargets));

    if (tempname)
        XFree(tempname);

#if 0
    XChangeProperty( my_display,
            event.xselectionrequest.requestor,
            propertyOfRequestorToSet,
            XA_ATOM,
            32, // 32 bits actually means long, according to what I've read // [Note 1]
            PropModeReplace,
            (unsigned char *) possibleTargets,
            sizeof(possibleTargets)/sizeof(possibleTargets[0])  // [Note 3]
            );
#endif


    XChangeProperty(display,
            sev->requestor,
            sev->property,
            target,
            32,
            PropModeReplace,
            (u8*)targets,
            numTargets);

    ssev.type = SelectionNotify;
    ssev.requestor = sev->requestor;
    ssev.selection = sev->selection;
    ssev.target = sev->target;
    ssev.property = sev->property;
    ssev.time = sev->time;

    XSendEvent(display, sev->requestor, True, NoEventMask, (XEvent *)&ssev);
}

void
clipboard_write(Display *display, u8* data, u32 size) {
    Window owner, root;
    int screen;
    Atom sel, image, targets;
    XSelectionRequestEvent *sev;

    screen = DefaultScreen(display);
    root = RootWindow(display, screen);

    /* We need a window to receive messages from other clients. */
    owner = XCreateSimpleWindow(display, root, -10, -10, 1, 1, 0, 0, 0);

    sel = XInternAtom(display, "CLIPBOARD", False);
    targets = XInternAtom(display, "TARGETS", False);
    image = XInternAtom(display, "image/png", False);

    Atom targetAtoms[] = {targets, image};

    //utf8 = XInternAtom(display, "UTF8_STRING", False);

    /* Claim ownership of the clipboard. */
    XSetSelectionOwner(display, sel, owner, CurrentTime);

    for (;;)
    {

        XEvent event;
        XNextEvent(display, &event);
        switch (event.type)
        {
            case SelectionClear:
                printf("Lost selection ownership\n");
                return;
                break;

            case SelectionRequest:
                sev = (XSelectionRequestEvent*)&event.xselectionrequest;
                printf("Requestor: 0x%lx\n", sev->requestor);
                /* Property is set to None by "obsolete" clients. */

                if (sev->property == None) {
                    clipboard_deny(display, sev);
                    continue;
                }

                if (sev->target == image)
                    clipboard_accept(display, sev, image, data, size);
                else if (sev->target == targets)
                    clipboard_send_targets(display, sev, targets, targetAtoms, SIZEOF_ARRAY(targetAtoms));
                else
                    clipboard_deny(display, sev);
                break;
        }
    }
}

XineramaScreenInfo
window_get_location(Display *display) {

    Window inwin, inchildwin;
    i32 root_x, root_y, win_x, win_y;
    u32 mask;

    i32 number_of_screens = XScreenCount(display);

    XSetErrorHandler(_XlibErrorHandler);

    printf("number of screens is %d\n", number_of_screens);
    Window* root_windows = malloc(sizeof(Window) * number_of_screens);

    for (i32 i = 0; i < number_of_screens; i++) {
        root_windows[i] = XRootWindow(display, i);
    }

    i32 result, windowIndex  = 0;
    for (; windowIndex < number_of_screens; windowIndex++) {
        result = XQueryPointer(display, root_windows[windowIndex], &inwin,
                &inchildwin, &root_x, &root_y, &win_x, &win_y, &mask);
        if (result == True) {
            break;
        }
    }

    if(!result) exit(EXIT_FAILURE);

    XWindowAttributes winattr = {};
    Status status = XGetWindowAttributes(display, root_windows[windowIndex], &winattr);
    (void)status;

    printf("Mouse is at (%d,%d)\n", root_x, root_y);

    // Determine which display has cursor
    int event_base_return, error_base_return;
    if (XineramaQueryExtension(display, &event_base_return, &error_base_return) &&
            XineramaIsActive(display)) {

        printf("ximera active\n");

        int numberOfMonitors;
        XineramaScreenInfo *info = XineramaQueryScreens(display, &numberOfMonitors);
        printf("numberOfMonitors is %d\n", numberOfMonitors);
        int correctMonitor = 0;

        for(; correctMonitor < numberOfMonitors; correctMonitor++) {
            printf("width is %d\n", info[correctMonitor].width);
            printf("height is %d\n", info[correctMonitor].height);
            printf("screen number is %d\n", info[correctMonitor].screen_number);
            printf("screen xorg is %d\n", info[correctMonitor].x_org);
            printf("screen yorg is %d\n", info[correctMonitor].y_org);

            int rectMinX = info[correctMonitor].x_org;
            int rectMinY = info[correctMonitor].y_org;
            int rectMaxX = info[correctMonitor].x_org + info[correctMonitor].width;
            int rectMaxY = info[correctMonitor].y_org + info[correctMonitor].height;

            if(rectMinX < root_x && rectMinY < root_y && rectMaxX > root_x && rectMaxY > root_y) {
                return info[correctMonitor];
            }

        }
        // Found right monitor
        exit(EXIT_FAILURE);
    }

    exit(EXIT_FAILURE);
}

XImage*
image_create_true_color(Display *display, Visual *visual, u8 *image, int width, int height) {

#if 0
    unsigned char *p=image;
    int i,j;
    for(i=0; i < width; i++)
    {
        for(j=0; j < height; j++)
        {
            if((i<256)&&(j<256))
            {
                *p++=rand()%256; // blue
                *p++=rand()%256; // green
                *p++=rand()%256; // red
            }
            else
            {
                *p++=i%256; // blue
                *p++=j%256; // green
                if(i<256)
                    *p++=i%256; // red
                else if(j<256)
                    *p++=j%256; // red
                else
                    *p++=(256-j)%256; // red
            }
            p++;
        }
    }
#endif
    return XCreateImage(display, visual, 24, ZPixmap, 0, (char*)image, width, height, 32, 0);
}

XImage*
monitor_get_image(Display* display, XineramaScreenInfo monitor) {

    timer_start();
    XImage *image = XGetImage(display, XDefaultRootWindow(display), monitor.x_org, monitor.y_org,
            monitor.width - 1, monitor.height - 1, AllPlanes, ZPixmap);

    timer_end("getImage");


    u8* imagedata = calloc((monitor.width - 1) * (monitor.height - 1), 4);

    if(!imagedata) exit(EXIT_FAILURE);

    timer_start();

    for (i32 y = 0; y < monitor.height - 1; y++) {
        for (i32 x = 0; x < monitor.width - 1; x++) {
            unsigned long pixel = image->f.get_pixel(image, x, y);

            u8 blue = pixel & image->blue_mask;
            u8 green = (pixel & image->green_mask) >> 8;
            u8 red = (pixel & image->red_mask) >> 16;

            imagedata[(x + (monitor.width - 1) * y) * 4] = blue;//  * 0.7;
            imagedata[(x + (monitor.width - 1) * y) * 4 + 1] = green;// * 0.7;
            imagedata[(x + (monitor.width - 1) * y) * 4 + 2] = red;// * 0.7;
        }
    }
    timer_end("copying image");

    return image_create_true_color(display,
            DefaultVisual(display, 0),
            imagedata,
            monitor.width - 1,
            monitor.height - 1);
}

i32
main(i32 argc, char** argv) {

    (void)argc; (void)argv;
    Display* display = XOpenDisplay(NULL);

    Window root = DefaultRootWindow(display);
    Visual *visual = NULL;
    int major,minor;

    if (XdbeQueryExtension(display, &major, &minor)) {
        printf("Xdbe (%d.%d) supported, using double buffering\n", major, minor);

        int numScreens = 1;
        Drawable screens[] = { DefaultRootWindow(display) };
        XdbeScreenVisualInfo *info = XdbeGetVisualInfo(display, screens, &numScreens);
        if (!info || numScreens < 1 || info->count < 1) {
            fprintf(stderr, "No visuals support Xdbe\n");
            return 110;
        }

        // Choosing the first one, seems that they have all perflevel of 0,
        // and the depth varies.
        XVisualInfo xvisinfo_templ;
        xvisinfo_templ.visualid = info->visinfo[0].visual; // We know there's at least one
        // As far as I know, screens are densely packed, so we can assume that if at least 1 exists, it's screen 0.
        xvisinfo_templ.screen = 0;
        xvisinfo_templ.depth = info->visinfo[0].depth;

        int matches;
        XVisualInfo *xvisinfo_match =
            XGetVisualInfo(display, VisualIDMask|VisualScreenMask|VisualDepthMask, &xvisinfo_templ, &matches);

        if (!xvisinfo_match || matches < 1) {
            fprintf(stderr, "Couldn't match a Visual with double buffering\n");
            return 111;
        }


        printf("%d supported visuals\n", info->count);
        for (int i = 0; i < info->count; ++i) {
            printf("visual %d/%d: id %d, depth %d, perf %d\n",
                    i, info->count,
                    (int)info->visinfo[i].visual,
                    info->visinfo[i].depth,
                    info->visinfo[i].perflevel);
        }
        printf("We got xvisinfo: id: %d, screen %d, depth %d\n",
                (int)xvisinfo_match->visualid, xvisinfo_match->screen, xvisinfo_match->depth);


        // We can use Visual from the match
        visual = xvisinfo_match->visual;
    } else {
        printf("Xdbe not supported\n");
        return EXIT_FAILURE;
    }
    //exit(1);

    XineramaScreenInfo monitor = window_get_location(display);

    // Determine which display has cursor
    // Found correct monitor

    printf("dimensions %d %d\n", monitor.width, monitor.height);

    XImage* image = monitor_get_image(display, monitor);

    u32 allocSize = (monitor.width - 1) * (monitor.height - 1) * 4;
    u8* brightImageData = malloc(allocSize);
    if(!brightImageData) return EXIT_FAILURE;

    memcpy(brightImageData, image->data, allocSize);


    XImage* brightImage = image_create_true_color(display,
            DefaultVisual(display, 0),
            brightImageData, monitor.width - 1,
            monitor.height - 1);

    printf("image %d %d\n", image->width, image->height);
#if 1
    int c = 0;







    for(int i = 0; i < image->height; i++) {
        for(int i2 = 0; i2 < image->width; i2++) {
            c++;

#if 0

            y * image->width * 4 + x


                imagedata[(x + (monitor.width - 1) * y) * 4] = blue * 0.7;
            imagedata[(x + (monitor.width - 1) * y) * 4 + 1] = green * 0.7;
            imagedata[(x + (monitor.width - 1) * y) * 4 + 2] = red * 0.7;
#endif
#if 0
            //int (*put_pixel)            (struct _XImage *, int, int, unsigned long);
            unsigned long byte = image->f.get_pixel(image, i2, i);

            ((unsigned char*)&byte)[0] *= 0.7;
            ((unsigned char*)&byte)[1] *= 0.7;
            ((unsigned char*)&byte)[2] *= 0.7;
            ((unsigned char*)&byte)[3] *= 0.7;

            image->f.put_pixel(image, i2, i, byte);
#else
            image->data[i * image->width * 4 + i2 * 4] =
                (u8)image->data[i * image->width * 4 + i2 * 4] * 0.5;

            image->data[i * image->width * 4 + i2 * 4 + 1] =
                (u8)image->data[i * image->width * 4 + i2 * 4 + 1] * 0.5;

            image->data[i * image->width * 4 + i2 * 4 + 2] =
                (u8)image->data[i * image->width * 4 + i2 * 4 + 2] * 0.5;

            image->data[i * image->width * 4 + i2 * 4 + 3] =
                (u8)image->data[i * image->width * 4 + i2 * 4 + 3] * 0.5;

            //image->data[i * image->width * 4 + i2 + 3] *= 0.9;
#endif
        }
    }

    printf("image %d %d\n", image->width, image->height);
    printf("darken!! %d\n", c);
#endif
    /*
       timer_start();
       stbi_write_png_compression_level = 2;
       stbi_write_png("./test.png", monitor.width - 1, monitor.height - 1, 3, imagedata, 0);
       timer_end("png write");

       timer_start();
       stbi_write_jpg("./test.jpg", monitor.width - 1, monitor.height - 1, 3, imagedata, 100);
       timer_end("jpg write");

       size_t size;
       u8* _imagedata = load_binary_file("./test.jpg", &size);
       printf("IMAGESIZE IS %d\n", (int)size);

       clipboard_write(display, _imagedata, size);
       */

    printf("creating window\n");

    // create window and image

    XVisualInfo vinfo;




    if (!XMatchVisualInfo(display, DefaultScreen(display), 24, TrueColor, &vinfo)) {
        printf("No visual found supporting 32 bit color, terminating\n");
        exit(EXIT_FAILURE);
    }

    XSetWindowAttributes attrs;
    attrs.override_redirect = 1;

    attrs.colormap = XCreateColormap(display, root, vinfo.visual, AllocNone);
    attrs.background_pixel = 0;
    attrs.border_pixel = 0;

    Window overlay = XCreateWindow(
            display, root,
            monitor.x_org, monitor.y_org, monitor.width, monitor.height, 0,
            vinfo.depth, InputOutput,
            vinfo.visual,
            CWOverrideRedirect | CWColormap | CWBackPixel | CWBorderPixel, &attrs
            );

    XdbeBackBuffer backbuffer = XdbeAllocateBackBufferName(display, overlay, XdbeBackground);

    GC backGC = XCreateGC(display, backbuffer, 0, NULL);

    i32 d_drawColor = BlackPixel(display, DefaultScreen(display));

    //XSelectInput(display, overlay, StructureNotifyMask); // get window visible msg
    XSelectInput(display, overlay, ButtonPressMask | ExposureMask | StructureNotifyMask);

    XMapWindow(display, overlay);
#if 1

    printf("waiting map notify!\n");
    XEvent ev;
    for (;;) {
        XNextEvent(display, &ev);
        if (ev.type == MapNotify) break;
    }

    printf("got map notify!\n");

    printf("image width %d  height %d\n", image->width, image->height);

    Window inwin, inchildwin;
    i32 root_x, root_y, win_x, win_y;
    u32 mask;

    int m1Pressed = 0;
    i32 mouseXStart = 0, mouseYStart = 0, mouseXend = 0, mouseYend = 0;


    for(;;) {
        XQueryPointer(display, root, &inwin,
                &inchildwin, &root_x, &root_y, &win_x, &win_y, &mask);

#if 1
        if(BIT_CHECK(Button1Mask, mask)) {
            printf("PRESSED 1111 !\n");
            if(!m1Pressed) {
                m1Pressed = 1;
                // monitor.x_org, monitor.y_org, monitor.width, monitor.height
                mouseXStart = root_x - monitor.x_org;
                mouseYStart = root_y - monitor.y_org;

                if(mouseXStart < 0 || mouseXStart > monitor.width ||
                        mouseXStart < 0 || mouseXStart > monitor.width) {
                    printf("mx %d my %d\n", mouseXStart, mouseYStart);
                    break;
                }
            }

            mouseXend = root_x - monitor.x_org;
            mouseYend = root_y - monitor.y_org;

            mouseXend = clamp_i32(mouseXend, 0 , monitor.width - 1);
            mouseYend = clamp_i32(mouseYend, 0 , monitor.height - 1);

            printf("startx %d starty %d endx %d endy %d\n",
                    mouseXStart, mouseYStart, mouseXend, mouseYend);

            printf("RERERERRE\n");

        } else if (m1Pressed == 1) {

            break;
        }
#endif

        //XPutImage(display, overlay, DefaultGC(display, 0), image, 0, 0, 0, 0, image->width, image->height);
        //XPutImage(display, overlay, DefaultGC(display, 0), image, 0, 0, 0, 0, image->width, image->height);
        XPutImage(display, backbuffer, backGC, image, 0, 0, 0, 0, image->width, image->height);


        if( mouseXStart != mouseXend || mouseYStart != mouseYend ) {

            int realXstart = mouseXStart,
                realXend = mouseXend,
                realYstart = mouseYStart,
                realYend = mouseYend;

            if(realXstart > realXend) {
                SWAP_VALUES(realXstart, realXend);
            }

            if(realYstart > realYend) {
                SWAP_VALUES(realYstart, realYend);
            }

            XPutImage(display, backbuffer, backGC, brightImage,
                    realXstart, realYstart, realXstart, realYstart,
                    realXend - realXstart, realYend - realYstart);
        }

        XFlush(display);

        XdbeSwapInfo swapInfo;
        swapInfo.swap_window = overlay;
        swapInfo.swap_action = XdbeBackground;

        // XdbeSwapBuffers returns true on success, we return 0 on success.
        if (!XdbeSwapBuffers(display, &swapInfo, 1))
            printf("ERROR\n");

    }


#endif

    XUnmapWindow(display, overlay);

    // Copy image and send to clients

    if(mouseXStart > mouseXend) {
        SWAP_VALUES(mouseXStart, mouseXend);
    }

    if(mouseYStart > mouseYend) {
        SWAP_VALUES(mouseYStart, mouseYend);
    }
    int imageWidth = mouseXend - mouseXStart;
    int imageHeight = mouseYend - mouseYStart;

    u8* sendData = calloc(imageWidth * imageHeight, 4);


    for(int i = 0, c = mouseYStart; i < imageHeight; i++ , c++) {
        for(int i2 = 0, c2 = mouseXStart; i2 < imageWidth; i2++, c2++) {

            sendData[(imageWidth * i + i2) * 4] =
                (u8)brightImage->data[(c * brightImage->width + c2) * 4 + 2];

            sendData[(imageWidth * i + i2) * 4 + 1] =
                (u8)brightImage->data[(c * brightImage->width + c2) * 4 + 1];

            sendData[(imageWidth * i + i2) * 4 + 2] =
                (u8)brightImage->data[(c * brightImage->width + c2) * 4];

            sendData[(imageWidth * i + i2) * 4 + 3] = numeric_max_u8;
            //(u8)image->data[(c * brightImage->width + c2) * 4 + 3];
        }
    }


    size_t size;
    timer_start();

    //u8* _imagedata = stbi_write_png("./test.png", imageWidth , imageHeight , 4, sendData, 0, &size);
    //
    u8* _imagedata = stbi_write_png_to_mem(sendData, 0, imageWidth, imageHeight, 4, &size);
    timer_end("png write");

#if 0
    timer_start();
    stbi_write_jpg("./test.jpg", imageWidth , imageHeight , 4, sendData, 100);
    timer_end("jpg write");
#endif
    /*

       u8* _imagedata = load_binary_file("./test.png", &size);
       printf("IMAGESIZE IS %d\n", (int)size);
       */
    clipboard_write(display, _imagedata, size);

    XCloseDisplay(display);

    return EXIT_SUCCESS;
}
#endif
