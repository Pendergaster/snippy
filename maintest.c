/************************************************************
 * Check license.txt in project root for license information *
 *********************************************************** */
#if 1
// gcc -o xclipget xclipget.c -lX11
#include <stdio.h>
#include <limits.h>
#include <X11/Xlib.h>
#include "defs.h"
#include "dynamicArray.h"


void atom_print(Display* display, Atom atom) {

    char* an = XGetAtomName(display, atom);
    printf("atom name '%s'\n", an);
    if (an)
        XFree(an);
}

Bool PrintSelection(Display *display, Window window, const char *bufname, const char *fmtname)
{
    char *result;
    unsigned long ressize, restail;
    int resbits;
    Atom bufid = XInternAtom(display, bufname, False),
         fmtid = XInternAtom(display, fmtname, False),
         propid = XInternAtom(display, "XSEL_DATA", False),
         incrid = XInternAtom(display, "INCR", False);
    XEvent event;

    u8* imageData = dynamicarray_create(sizeof(u8));

    XSelectInput (display, window, PropertyChangeMask);
    XConvertSelection(display, bufid, fmtid, propid, window, CurrentTime);
    do {
        XNextEvent(display, &event);
    } while (event.type != SelectionNotify || event.xselection.selection != bufid);

    if (event.xselection.property)
    {
        XGetWindowProperty(display, window, propid, 0, LONG_MAX/4, True, AnyPropertyType,
                &fmtid, &resbits, &ressize, &restail, (unsigned char**)&result);

        atom_print(display, fmtid);
        if (fmtid != incrid) {
            printf("(No INCR) %d \n ", (int)ressize );

            FILE* file = fopen("tempTEST.jpeg", "wb");
            fwrite(result, 1, ressize, file); // write 10 bytes from our buffer
            fclose(file);
            return 1;
        }

        XFree(result);

        if (fmtid == incrid) {
            printf("(INCR %d)\n", *((u8*)result));
            do {
                do {
                    XNextEvent(display, &event);
                    printf("got new event!\n");
                } while (event.type != PropertyNotify ||
                        event.xproperty.atom != propid ||
                        event.xproperty.state != PropertyNewValue);

                printf("getting new property.....!\n");
                XGetWindowProperty(display, window, propid, 0, LONG_MAX/4, True, AnyPropertyType,
                        &fmtid, &resbits, &ressize, &restail, (unsigned char**)&result);

                atom_print(display, fmtid);
                printf("%d SIZE AAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n", (int)ressize);
                for(int i = 0; i < (int)ressize; i++) {
                    dynamicarray_push_back(imageData, &result[i]);
                }
                printf("image size is %d \n", dynamicarray_size(imageData));
                XFree(result);
            } while (ressize > 0);
        }

        //XDeleteProperty(display, window, propid);
        printf("all good\n");

        printf("final size is %d \n", dynamicarray_size(imageData));
        FILE* file = fopen("tempTEST.jpeg", "wb");
        fwrite(imageData, 1, dynamicarray_size(imageData), file); // write 10 bytes from our buffer
        fclose(file);
        return True;
    }
    else // request failed, e.g. owner can't convert to the target format
        return False;
}


int main()
{
    Display *display = XOpenDisplay(NULL);
    unsigned long color = BlackPixel(display, DefaultScreen(display));
    Window window = XCreateSimpleWindow(display, DefaultRootWindow(display), 0,0, 1,1, 0, color, color);
    Bool result =
        /*PrintSelection(display, window, "CLIPBOARD", "UTF8_STRING") ||*/
        PrintSelection(display, window, "CLIPBOARD", "image/jpeg");

    XDestroyWindow(display, window);
    XCloseDisplay(display);
    return !result;
}
#endif
#if 0

#include <stdio.h>
#include <X11/Xlib.h>


Atom clipboard, targets;

void
print_targets(Display *dpy, Window target_window)
{

    Atom target_property = XInternAtom(dpy, "PENGUIN", False);
    XConvertSelection(dpy, clipboard, targets, target_property, target_window,
            CurrentTime);

    for (;;)
    {
        XEvent ev;
        XSelectionEvent* sev;
        XNextEvent(dpy, &ev);
        switch (ev.type)
        {
            case SelectionNotify:
                sev = (XSelectionEvent*)&ev.xselection;
                if (sev->property == None)
                {
                    printf("Conversion could not be performed.\n");
                    return;
                }
                else
                {
                    Atom da;
                    int formatReturn;
                    unsigned long size = 128, afterRet, nitems;
                    unsigned char *prop_ret = NULL;

                    XGetWindowProperty(dpy, target_window, target_property, 0, size,
                            False,
                            AnyPropertyType,
                            &da, // ??
                            &formatReturn, // ??
                            &nitems,
                            &afterRet, //after return
                            &prop_ret);


                    printf("num items: %lu\nAfter return %lu\n", nitems, afterRet);


                    for (int i = 0; i < (int)nitems; i++) {
                        char *name = XGetAtomName(dpy, ((Atom*)prop_ret)[i]);
                        printf("    '%s'\n", name);
                        if (name)
                            XFree(name);
                    }

                    fflush(stdout);
                    XFree(prop_ret);

                    /* Signal the selection owner that we have successfully read the
                     * data. */
                    XDeleteProperty(dpy, target_window, target_property);

                    return;
                }
                break;
        }
    }
}

void
get_image(Display *dpy, Window target_window) {

    Atom target_property = XInternAtom(dpy, "PENGUIN2", False);
    Atom image = XInternAtom(dpy, "image/jpeg", False);
    XConvertSelection(dpy, clipboard, image, target_property, target_window,
            CurrentTime);



    for (;;)
    {
        XEvent ev;
        XSelectionEvent* sev;
        printf("next event....\n\n");
        XNextEvent(dpy, &ev);
        switch (ev.type)
        {
            case SelectionNotify:
                sev = (XSelectionEvent*)&ev.xselection;
                if (sev->property == None)
                {
                    printf("Conversion could not be performed.\n");
                    return;
                }
                else
                {


                    printf("get image...\n");

                    Atom type;
                    int formatReturn;

                    unsigned long afterRet, nitems;
                    unsigned char *prop_ret = NULL;

                    /* Dummy call to get type and size. */
                    XGetWindowProperty(
                            dpy,
                            target_window,
                            target_property,
                            0,
                            0,
                            False,
                            AnyPropertyType,
                            &type,
                            &formatReturn,
                            &nitems,
                            &afterRet,
                            &prop_ret);

                    printf("bytes after return %d,"
                            " nitems %d, format return %d\n",
                            (int)afterRet, (int)nitems, (int)formatReturn);

                    //  The selection requestor starts the transfer process by deleting the (type==INCR)
                    //  property forming the reply to the selection.

                    XFree(prop_ret);

                    Atom incr = XInternAtom(dpy, "INCR", False);
                    if (type == incr) {
                        printf("reading packet!\n");

                        XGetWindowProperty(dpy,
                                target_window,
                                target_property,
                                0,
                                afterRet,
                                True,
                                AnyPropertyType,
                                &type, // ??
                                &formatReturn, // ??
                                &nitems,
                                &afterRet, //after return
                                &prop_ret);


                        printf("(!!!!!!!) bytes after return %d,"
                                " nitems %d, format return %d\n",
                                (int)afterRet, (int)nitems, (int)formatReturn);

#if 0
                        XGetWindowProperty(dpy, w, p, 0, size,
                                False,
                                AnyPropertyType,
                                &da,
                                &di,
                                &dul,
                                &dul,
                                &prop_ret);
#endif

                        XDeleteProperty(dpy, target_window, target_property);
                        //return;
                    } else {
                        printf("All red!\n");
                    }

                    //return;
                }
                break;
        }
    }
}

int
main()
{
    Display *dpy;
    Window target_window, root;
    int screen;

    dpy = XOpenDisplay(NULL);
    if (!dpy)
    {
        fprintf(stderr, "Could not open X display\n");
        return 1;
    }

    screen = DefaultScreen(dpy);
    root = RootWindow(dpy, screen);



    /* The selection owner will store the data in a property on this
     * window: */
    target_window = XCreateSimpleWindow(dpy, root, -10, -10, 1, 1, 0, 0, 0);

    clipboard = XInternAtom(dpy, "CLIPBOARD", False);
    targets = XInternAtom(dpy, "TARGETS", False);

    print_targets(dpy, target_window);
    get_image(dpy, target_window);

    /* That's the property used by the owner. Note that it's completely
     * arbitrary. */

    /* Request conversion to UTF-8. Not all owners will be able to
     * fulfill that request. */
}
#endif

#if 0
#include <stdio.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>

void
show_targets(Display *display, Window target_window, Atom p)
{
    Atom type, *targets;
    int formatReturn;
    unsigned long i, nitems, afterRet;
    unsigned char *prop_ret = NULL;
    char *an = NULL;

    /* Read the first 1024 atoms from this list of atoms. We don't
     * expect the selection owner to be able to convert to more than
     * 1024 different targets. :-) */
    XGetWindowProperty(display,
            target_window,
            p,
            0,
            1024 * sizeof (Atom),
            False,
            XA_ATOM,
            &type,
            &formatReturn,
            &nitems,
            &afterRet,
            &prop_ret);



    printf("Targets (num %d, bytes after return %d):\n", (int)nitems, (int)afterRet);
    targets = (Atom *)prop_ret;

    //printf("aaa %d %d\n", (int)targets[0], (int)targets[1]);
#if 0
    for (i = 0; i < nitems; i++) {
        an = XGetAtomName(display, targets[i]);
        printf("    '%s'\n", an);
        if (an)
            XFree(an);
    }
#endif
    XFree(prop_ret);

    XDeleteProperty(display, target_window, p);
}

int
main()
{
    Display *display;
    Window target_window, root;
    int screen;
    Atom clipboard, targets, target_property;
    XEvent ev;
    XSelectionEvent *sev;

    display = XOpenDisplay(NULL);
    if (!display) {
        fprintf(stderr, "Could not open X display\n");
        return 1;
    }

    screen = DefaultScreen(display);
    root = RootWindow(display, screen);

    clipboard = XInternAtom(display, "CLIPBOARD", False);
    targets = XInternAtom(display, "UTF8_STRING", False);
    target_property = XInternAtom(display, "PENGUIN", False);

    target_window = XCreateSimpleWindow(display, root, -10, -10, 1, 1, 0, 0, 0);

    XConvertSelection(display, clipboard, targets, target_property, target_window,
            CurrentTime);

    for (;;)
    {
        XNextEvent(display, &ev);
        switch (ev.type)
        {
            case SelectionNotify:
                sev = (XSelectionEvent*)&ev.xselection;
                if (sev->property == None)
                {
                    printf("Conversion could not be performed.\n");
                    return 1;
                }
                else
                {
                    show_targets(display, target_window, target_property);
                    return 0;
                }
                break;
        }
    }
}
#endif
