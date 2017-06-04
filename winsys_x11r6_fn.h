/* Jordan Justen : gears3d is public domain */

FNSYM(XOpenDisplay, Display*, (const char*))
FNSYM(XCreateSimpleWindow, Window, (Display*, Window, int, int, unsigned int,
                                    unsigned int, unsigned int, unsigned long,
                                    unsigned long))
FNSYM(XCreateWindow, Window, (Display*, Window, int, int, unsigned int,
                              unsigned int, unsigned int, int, unsigned int,
                              Visual*,
                              unsigned long, XSetWindowAttributes*))
FNSYM(XMapWindow, int, (Display*, Window))
FNSYM(XFlush, int, (Display*))
FNSYM(XChangeProperty, int, (Display*, Window, Atom, Atom, int, int,
                             unsigned char*, int))
FNSYM(XStoreName, int, (Display*, Window, char*))
FNSYM(XEventsQueued, int, (Display*, int))
FNSYM(XNextEvent, int, (Display*, XEvent*))
FNSYM(XSelectInput, int, (Display*, Window, long))
FNSYM(XKeysymToKeycode, KeyCode, (Display*, KeySym))
FNSYM(XStringToKeysym, KeySym, (char*))
