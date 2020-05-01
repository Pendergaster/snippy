/************************************************************
 * Check license.txt in project root for license information *
 *********************************************************** */

#include <stdio.h>
#include <X11/Xutil.h>
#include <X11/Xlib.h>
#include <X11/X.h>
#include <X11/extensions/Xinerama.h>
#include <time.h>
#include "defs.h"

void
clipboard_deny(Display *dpy, XSelectionRequestEvent *sev)
{
    XSelectionEvent ssev;
    char *an;

    an = XGetAtomName(dpy, sev->target);
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

    XSendEvent(dpy, sev->requestor, True, NoEventMask, (XEvent *)&ssev);
}

void
clipboard_send_targets(Display *dpy, XSelectionRequestEvent *sev, Atom target, Atom* targets,
        int numTargets) {

    XSelectionEvent ssev;
    char* nametemp = XGetAtomName(dpy, sev->property);
    printf("Sending data to window 0x%lx, property '%s', numTargets %d\n",
            sev->requestor,
            nametemp,
            numTargets);

    if (nametemp)
        XFree(nametemp);

    XChangeProperty(dpy,
            sev->requestor,
            sev->property,
            target,
            32,
            PropModeReplace,
            (unsigned char*)targets,
            numTargets);

    ssev.type = SelectionNotify;
    ssev.requestor = sev->requestor;
    ssev.selection = sev->selection;
    ssev.target = sev->target;
    ssev.property = sev->property;
    ssev.time = sev->time;

    XSendEvent(dpy, sev->requestor, True, NoEventMask, (XEvent *)&ssev);
}

void atom_print(Display* display, Atom atom) {

    char* an = XGetAtomName(display, atom);
    printf("atom name '%s'\n", an);
    if (an)
        XFree(an);
}

void
send_utf8(Display *dpy, XSelectionRequestEvent *sev, Atom utf8)
{
#if 0
    XSelectionEvent ssev;
    time_t now_tm;
    char *now, *an;

    now_tm = time(NULL);
    now = ctime(&now_tm);

    an = XGetAtomName(dpy, sev->property);
    printf("Sending data to window 0x%lx, property '%s' (%s)\n", sev->requestor, an, now);
    if (an)
        XFree(an);


    Atom incrAtom = XInternAtom(display, "INCR", False),

         XChangeProperty(dpy, sev->requestor, sev->property, utf8, 8, PropModeReplace,
                 (unsigned char *)now, strlen(now));

    ssev.type = SelectionNotify;
    ssev.requestor = sev->requestor;
    ssev.selection = sev->selection;
    ssev.target = sev->target;
    ssev.property = sev->property;
    ssev.time = sev->time;

    XSendEvent(dpy, sev->requestor, True, NoEventMask, (XEvent *)&ssev);
#endif

    XSelectionEvent ssev;
    time_t now_tm;
    char *now, *an;

    now_tm = time(NULL);
    now = ctime(&now_tm);

    an = XGetAtomName(dpy, sev->property);
    printf("Sending data to window 0x%lx, property '%s' (%s)\n", sev->requestor, an, now);
    if (an)
        XFree(an);


    Atom incrAtom = XInternAtom(dpy, "INCR", False);
    u32 kys = 0;
    XChangeProperty(dpy, sev->requestor, sev->property, incrAtom, 32, PropModeReplace,
            (unsigned char *)&kys, 1);

    ssev.type = SelectionNotify;
    ssev.requestor = sev->requestor;
    ssev.selection = sev->selection;
    ssev.target = sev->target;
    ssev.property = sev->property;
    ssev.time = sev->time;

    XSendEvent(dpy, sev->requestor, True, NoEventMask, (XEvent *)&ssev);

    Atom targetAtom = sev->property;
    printf("waiting for event...\n");

    XSelectInput(dpy, sev->requestor, PropertyChangeMask);
    int dataSend = 0;
    XEvent ev;
    for (;;)
    {
        XNextEvent(dpy, &ev);
        printf("new event!\n");
        if (ev.type == PropertyNotify &&
                ev.xproperty.atom == targetAtom &&
                ev.xproperty.state == PropertyDelete) {

            printf("rRERE\n");
            if(dataSend++ < 5) {

                printf("sending more data!!\n");

                XSelectionEvent ssev;
                time_t now_tm;
                char *now, *an;

                now_tm = time(NULL);
                now = ctime(&now_tm);

                an = XGetAtomName(dpy, sev->property);
                printf("Sending data to window 0x%lx, property '%s' (%s)\n", sev->requestor, an, now);
                if (an)
                    XFree(an);

                XChangeProperty(dpy, sev->requestor, sev->property, utf8, 8, PropModeReplace,
                        (unsigned char *)now, strlen(now));

                ssev.type = SelectionNotify;
                ssev.requestor = sev->requestor;
                ssev.selection = sev->selection;
                ssev.target = sev->target;
                ssev.property = sev->property;
                ssev.time = sev->time;

                XSendEvent(dpy, sev->requestor, True, NoEventMask, (XEvent *)&ssev);
            } else {

                printf("sending end signal!!\n");

                XChangeProperty(dpy, sev->requestor, sev->property, utf8, 8, PropModeReplace,
                        (unsigned char *)now, 0);

                ssev.type = SelectionNotify;
                ssev.requestor = sev->requestor;
                ssev.selection = sev->selection;
                ssev.target = sev->target;
                ssev.property = sev->property;
                ssev.time = sev->time;

                XSendEvent(dpy, sev->requestor, True, NoEventMask, (XEvent *)&ssev);
            }

        }
    }
}

int main() {
    Display* display = XOpenDisplay(NULL);

    Window owner, utf8, root;
    int screen;
    Atom sel, targets;
    XEvent ev;
    XSelectionRequestEvent *sev;

    screen = DefaultScreen(display);
    root = RootWindow(display, screen);

    /* We need a window to receive messages from other clients. */
    owner = XCreateSimpleWindow(display, root, -10, -10, 1, 1, 0, 0, 0);

    sel = XInternAtom(display, "CLIPBOARD", False);
    targets = XInternAtom(display, "TARGETS", False);
    utf8 = XInternAtom(display, "UTF8_STRING", False);

    Atom targetAtoms[] = {targets, 11};
    //XSelectInput(display, owner, PropertyChangeMask | StructureNotifyMask | SubstructureNotifyMask);
    //utf8 = XInternAtom(display, "UTF8_STRING", False);

    /* Claim ownership of the clipboard. */
    XSetSelectionOwner(display, sel, owner, CurrentTime);

    for (;;)
    {
        XNextEvent(display, &ev);
        switch (ev.type)
        {
            case SelectionClear:
                printf("Lost selection ownership\n");
                return 1;
                break;

            case SelectionRequest:
                sev = (XSelectionRequestEvent*)&ev.xselectionrequest;
                printf("Requestor: 0x%lx\n", sev->requestor);
                /* Property is set to None by "obsolete" clients. */

                if (sev->property == None) {
                    printf("PROP NONE\n");
                    clipboard_deny(display, sev);
                    break;
                }

                if (sev->target == targets) {
                    clipboard_send_targets(
                            display,
                            sev,
                            targets,
                            targetAtoms,
                            SIZEOF_ARRAY(targetAtoms));
                }
                else if (sev->target == utf8) {
                    send_utf8(display, sev, utf8);
                }
                else {
                    clipboard_deny(display, sev);
                }
                break;
        }
    }
}
