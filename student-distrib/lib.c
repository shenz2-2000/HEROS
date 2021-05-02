/* lib.c - Some basic library functions (printf, strlen, etc.)
 * vim:ts=4 noexpandtab */

#include "lib.h"
#include "terminal.h"
#define VIDEO       0xB8000
#define NUM_COLS    80
#define NUM_ROWS    25
#define ATTRIB      0x7

// "blue screen"
#define BULE_SCREEN 0x30

int screen_x;
int screen_y;
static char* video_mem = (char *)VIDEO;



/* update_cursor
 * Description: update the cursor position
 * Inputs: x -- x position
 *         y -- y position
 * Return Value: none
 * Function: write to video memory */
void update_cursor(int x, int y){
    uint16_t  pos = y* NUM_COLS + x;

    // update the new position
    outb(0x0F,0x3D4);
    outb((uint8_t) (pos & 0xFF), 0x3D5);
    outb(0x0E,0x3D4);
    outb((uint8_t) ( ( pos >> 8) & 0xFF), 0x3D5);
}



/* void set_blue_screen(void);
 * Description: Set the whole screen into blue
 * Inputs: void
 * Return Value: none
 * Function: write to  video memory */
void set_blue_screen(){
    int32_t i;
    for (i = 0; i < NUM_ROWS * NUM_COLS; i++)
        *(uint8_t *)(video_mem + (i << 1) + 1) = BULE_SCREEN;
}

/* void restore_blue_screen(void);
 * Description: Set the whole screen into black
 * Inputs: void
 * Return Value: none
 * Function: write to  video memory */
void restore_blue_screen(){
    int32_t i;
    for (i = 0; i < NUM_ROWS * NUM_COLS; i++)
        *(uint8_t *)(video_mem + (i << 1) + 1) = ATTRIB;

}

/* void clear(void);
 * Inputs: void
 * Return Value: none
 * Function: Clears video memory */
void clear(void) {
    int32_t i;
    for (i = 0; i < NUM_ROWS * NUM_COLS; i++) {
        *(uint8_t *)(video_mem + (i << 1)) = 0;
        *(uint8_t *)(video_mem + (i << 1) + 1) = ATTRIB;
    }
}

/* void delete_last(void);
 * Inputs: void
 * Return Value: none
 * Function: delete the last character */
void delete_last(void) {
    if (screen_x==0&&screen_y>0) {
        screen_y--;
        screen_x = NUM_COLS;
    }
    if (screen_x>0) screen_x--,
                    *(uint8_t *)(video_mem+((screen_x+screen_y*NUM_COLS)<<1))=0,
                    *(uint8_t *)(video_mem+((screen_x+screen_y*NUM_COLS)<<1)+1)=ATTRIB;

    // update the cursor position
    update_cursor(get_showing_terminal()->screen_x,get_showing_terminal()->screen_y);
}

/* void reset_screen(void);
 * Inputs: void
 * Return Value: none
 * Function: Clears screen position */
void reset_screen() {
    screen_x = 0;
    screen_y = 0;
}

/* Standard printf().
 * Only supports the following format strings:
 * %%  - print a literal '%' character
 * %x  - print a number in hexadecimal
 * %u  - print a number as an unsigned integer
 * %d  - print a number as a signed integer
 * %c  - print a character
 * %s  - print a string
 * %#x - print a number in 32-bit aligned hexadecimal, i.e.
 *       print 8 hexadecimal digits, zero-padded on the left.
 *       For example, the hex number "E" would be printed as
 *       "0000000E".
 *       Note: This is slightly different than the libc specification
 *       for the "#" modifier (this implementation doesn't add a "0x" at
 *       the beginning), but I think it's more flexible this way.
 *       Also note: %x is the only conversion specifier that can use
 *       the "#" modifier to alter output. */
int32_t printf(int8_t *format, ...) {

    /* Pointer to the format string */
    int8_t* buf = format;

    /* Stack pointer for the other parameters */
    int32_t* esp = (void *)&format;
    esp++;

    while (*buf != '\0') {
        switch (*buf) {
            case '%':
                {
                    int32_t alternate = 0;
                    buf++;

format_char_switch:
                    /* Conversion specifiers */
                    switch (*buf) {
                        /* Print a literal '%' character */
                        case '%':
                            putc('%');
                            break;

                        /* Use alternate formatting */
                        case '#':
                            alternate = 1;
                            buf++;
                            /* Yes, I know gotos are bad.  This is the
                             * most elegant and general way to do this,
                             * IMHO. */
                            goto format_char_switch;

                        /* Print a number in hexadecimal form */
                        case 'x':
                            {
                                int8_t conv_buf[64];
                                if (alternate == 0) {
                                    itoa(*((uint32_t *)esp), conv_buf, 16);
                                    puts(conv_buf);
                                } else {
                                    int32_t starting_index;
                                    int32_t i;
                                    itoa(*((uint32_t *)esp), &conv_buf[8], 16);
                                    i = starting_index = strlen(&conv_buf[8]);
                                    while(i < 8) {
                                        conv_buf[i] = '0';
                                        i++;
                                    }
                                    puts(&conv_buf[starting_index]);
                                }
                                esp++;
                            }
                            break;

                        /* Print a number in unsigned int form */
                        case 'u':
                            {
                                int8_t conv_buf[36];
                                itoa(*((uint32_t *)esp), conv_buf, 10);
                                puts(conv_buf);
                                esp++;
                            }
                            break;

                        /* Print a number in signed int form */
                        case 'd':
                            {
                                int8_t conv_buf[36];
                                int32_t value = *((int32_t *)esp);
                                if(value < 0) {
                                    conv_buf[0] = '-';
                                    itoa(-value, &conv_buf[1], 10);
                                } else {
                                    itoa(value, conv_buf, 10);
                                }
                                puts(conv_buf);
                                esp++;
                            }
                            break;

                        /* Print a single character */
                        case 'c':
                            putc((uint8_t) *((int32_t *)esp));
                            esp++;
                            break;

                        /* Print a NULL-terminated string */
                        case 's':
                            puts(*((int8_t **)esp));
                            esp++;
                            break;

                        default:
                            break;
                    }

                }
                break;

            default:
                putc(*buf);
                break;
        }
        buf++;
    }
    return (buf - format);
}

/* int32_t puts(int8_t* s);
 *   Inputs: int_8* s = pointer to a string of characters
 *   Return Value: Number of bytes written
 *    Function: Output a string to the console */
int32_t puts(int8_t* s) {
    register int32_t index = 0;
    while (s[index] != '\0') {
        putc(s[index]);
        index++;
    }
    return index;
}

/* void putc(uint8_t c);
 * Inputs: uint_8* c = character to print
 * Return Value: void
 *  Function: Output a character to the console */
void putc(uint8_t c) {
    int i,j;
    if(c == '\n' || c == '\r') {
        screen_y++;
        screen_x = 0;
        // FIX the bug when the screen is full, it starts from the beginning of the screen
        if (screen_y == NUM_ROWS) {
            for (i=0; i<NUM_ROWS;++i)
                for (j=0;j<NUM_COLS;++j) {
                    *(uint8_t *)(video_mem+((NUM_COLS*i+j)<<1)) = (i!=NUM_ROWS-1)?(*(uint8_t *)(video_mem+((NUM_COLS*(i+1)+j)<<1))):0;
                    *(uint8_t *)(video_mem+((NUM_COLS*i+j)<<1)+1) = ATTRIB;
                }
            screen_y--;
            update_cursor(get_showing_terminal()->screen_x,get_showing_terminal()->screen_y);
        }
    } else {
        *(uint8_t *)(video_mem + ((NUM_COLS * screen_y + screen_x) << 1)) = c;
        *(uint8_t *)(video_mem + ((NUM_COLS * screen_y + screen_x) << 1) + 1) = ATTRIB;
        screen_x++;
        // FIX the bug when the screen is full, it starts from the beginning of the screen
        if (screen_x==NUM_COLS&&screen_y+1==NUM_ROWS) {
            for (i=0; i<NUM_ROWS;++i)
                for (j=0;j<NUM_COLS;++j) {
                    *(uint8_t *)(video_mem+((NUM_COLS*i+j)<<1)) = (i!=NUM_ROWS-1)?(*(uint8_t *)(video_mem+((NUM_COLS*(i+1)+j)<<1))):0;
                    *(uint8_t *)(video_mem+((NUM_COLS*i+j)<<1)+1) = ATTRIB;
                }
            screen_y--;
//            update_cursor(screen_x,screen_y);
        }

////       screen_x %= NUM_COLS;
//       screen_y = (screen_y + (screen_x / NUM_COLS)) % NUM_ROWS;
//       screen_x %= NUM_COLS;
       if (screen_x == NUM_COLS){
           screen_x = 0;
           screen_y += 1;
           update_cursor(screen_x,screen_y);
       }
       else{
           screen_x %= NUM_COLS;
           screen_y = (screen_y + (screen_x / NUM_COLS)) % NUM_ROWS;
           update_cursor(screen_x,screen_y);
       }

    }
}

/* int8_t* itoa(uint32_t value, int8_t* buf, int32_t radix);
 * Inputs: uint32_t value = number to convert
 *            int8_t* buf = allocated buffer to place string in
 *          int32_t radix = base system. hex, oct, dec, etc.
 * Return Value: number of bytes written
 * Function: Convert a number to its ASCII representation, with base "radix" */
int8_t* itoa(uint32_t value, int8_t* buf, int32_t radix) {
    static int8_t lookup[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int8_t *newbuf = buf;
    int32_t i;
    uint32_t newval = value;

    /* Special case for zero */
    if (value == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return buf;
    }

    /* Go through the number one place value at a time, and add the
     * correct digit to "newbuf".  We actually add characters to the
     * ASCII string from lowest place value to highest, which is the
     * opposite of how the number should be printed.  We'll reverse the
     * characters later. */
    while (newval > 0) {
        i = newval % radix;
        *newbuf = lookup[i];
        newbuf++;
        newval /= radix;
    }

    /* Add a terminating NULL */
    *newbuf = '\0';

    /* Reverse the string and return */
    return strrev(buf);
}

/* int8_t* strrev(int8_t* s);
 * Inputs: int8_t* s = string to reverse
 * Return Value: reversed string
 * Function: reverses a string s */
int8_t* strrev(int8_t* s) {
    register int8_t tmp;
    register int32_t beg = 0;
    register int32_t end = strlen(s) - 1;

    while (beg < end) {
        tmp = s[end];
        s[end] = s[beg];
        s[beg] = tmp;
        beg++;
        end--;
    }
    return s;
}

/* uint32_t strlen(const int8_t* s);
 * Inputs: const int8_t* s = string to take length of
 * Return Value: length of string s
 * Function: return length of string s */
uint32_t strlen(const int8_t* s) {
    register uint32_t len = 0;
    while (s[len] != '\0')
        len++;
    return len;
}

/* void* memset(void* s, int32_t c, uint32_t n);
 * Inputs:    void* s = pointer to memory
 *          int32_t c = value to set memory to
 *         uint32_t n = number of bytes to set
 * Return Value: new string
 * Function: set n consecutive bytes of pointer s to value c */
void* memset(void* s, int32_t c, uint32_t n) {
    c &= 0xFF;
    asm volatile ("                 \n\
            .memset_top:            \n\
            testl   %%ecx, %%ecx    \n\
            jz      .memset_done    \n\
            testl   $0x3, %%edi     \n\
            jz      .memset_aligned \n\
            movb    %%al, (%%edi)   \n\
            addl    $1, %%edi       \n\
            subl    $1, %%ecx       \n\
            jmp     .memset_top     \n\
            .memset_aligned:        \n\
            movw    %%ds, %%dx      \n\
            movw    %%dx, %%es      \n\
            movl    %%ecx, %%edx    \n\
            shrl    $2, %%ecx       \n\
            andl    $0x3, %%edx     \n\
            cld                     \n\
            rep     stosl           \n\
            .memset_bottom:         \n\
            testl   %%edx, %%edx    \n\
            jz      .memset_done    \n\
            movb    %%al, (%%edi)   \n\
            addl    $1, %%edi       \n\
            subl    $1, %%edx       \n\
            jmp     .memset_bottom  \n\
            .memset_done:           \n\
            "
            :
            : "a"(c << 24 | c << 16 | c << 8 | c), "D"(s), "c"(n)
            : "edx", "memory", "cc"
    );
    return s;
}

/* void* memset_word(void* s, int32_t c, uint32_t n);
 * Description: Optimized memset_word
 * Inputs:    void* s = pointer to memory
 *          int32_t c = value to set memory to
 *         uint32_t n = number of bytes to set
 * Return Value: new string
 * Function: set lower 16 bits of n consecutive memory locations of pointer s to value c */
void* memset_word(void* s, int32_t c, uint32_t n) {
    asm volatile ("                 \n\
            movw    %%ds, %%dx      \n\
            movw    %%dx, %%es      \n\
            cld                     \n\
            rep     stosw           \n\
            "
            :
            : "a"(c), "D"(s), "c"(n)
            : "edx", "memory", "cc"
    );
    return s;
}

/* void* memset_dword(void* s, int32_t c, uint32_t n);
 * Inputs:    void* s = pointer to memory
 *          int32_t c = value to set memory to
 *         uint32_t n = number of bytes to set
 * Return Value: new string
 * Function: set n consecutive memory locations of pointer s to value c */
void* memset_dword(void* s, int32_t c, uint32_t n) {
    asm volatile ("                 \n\
            movw    %%ds, %%dx      \n\
            movw    %%dx, %%es      \n\
            cld                     \n\
            rep     stosl           \n\
            "
            :
            : "a"(c), "D"(s), "c"(n)
            : "edx", "memory", "cc"
    );
    return s;
}

/* void* memcpy(void* dest, const void* src, uint32_t n);
 * Inputs:      void* dest = destination of copy
 *         const void* src = source of copy
 *              uint32_t n = number of byets to copy
 * Return Value: pointer to dest
 * Function: copy n bytes of src to dest */
void* memcpy(void* dest, const void* src, uint32_t n) {
    asm volatile ("                 \n\
            .memcpy_top:            \n\
            testl   %%ecx, %%ecx    \n\
            jz      .memcpy_done    \n\
            testl   $0x3, %%edi     \n\
            jz      .memcpy_aligned \n\
            movb    (%%esi), %%al   \n\
            movb    %%al, (%%edi)   \n\
            addl    $1, %%edi       \n\
            addl    $1, %%esi       \n\
            subl    $1, %%ecx       \n\
            jmp     .memcpy_top     \n\
            .memcpy_aligned:        \n\
            movw    %%ds, %%dx      \n\
            movw    %%dx, %%es      \n\
            movl    %%ecx, %%edx    \n\
            shrl    $2, %%ecx       \n\
            andl    $0x3, %%edx     \n\
            cld                     \n\
            rep     movsl           \n\
            .memcpy_bottom:         \n\
            testl   %%edx, %%edx    \n\
            jz      .memcpy_done    \n\
            movb    (%%esi), %%al   \n\
            movb    %%al, (%%edi)   \n\
            addl    $1, %%edi       \n\
            addl    $1, %%esi       \n\
            subl    $1, %%edx       \n\
            jmp     .memcpy_bottom  \n\
            .memcpy_done:           \n\
            "
            :
            : "S"(src), "D"(dest), "c"(n)
            : "eax", "edx", "memory", "cc"
    );
    return dest;
}

/* void* memmove(void* dest, const void* src, uint32_t n);
 * Description: Optimized memmove (used for overlapping memory areas)
 * Inputs:      void* dest = destination of move
 *         const void* src = source of move
 *              uint32_t n = number of byets to move
 * Return Value: pointer to dest
 * Function: move n bytes of src to dest */
void* memmove(void* dest, const void* src, uint32_t n) {
    asm volatile ("                             \n\
            movw    %%ds, %%dx                  \n\
            movw    %%dx, %%es                  \n\
            cld                                 \n\
            cmp     %%edi, %%esi                \n\
            jae     .memmove_go                 \n\
            leal    -1(%%esi, %%ecx), %%esi     \n\
            leal    -1(%%edi, %%ecx), %%edi     \n\
            std                                 \n\
            .memmove_go:                        \n\
            rep     movsb                       \n\
            "
            :
            : "D"(dest), "S"(src), "c"(n)
            : "edx", "memory", "cc"
    );
    return dest;
}

/* int32_t strncmp(const int8_t* s1, const int8_t* s2, uint32_t n)
 * Inputs: const int8_t* s1 = first string to compare
 *         const int8_t* s2 = second string to compare
 *               uint32_t n = number of bytes to compare
 * Return Value: A zero value indicates that the characters compared
 *               in both strings form the same string.
 *               A value greater than zero indicates that the first
 *               character that does not match has a greater value
 *               in str1 than in str2; And a value less than zero
 *               indicates the opposite.
 * Function: compares string 1 and string 2 for equality */
int32_t strncmp(const int8_t* s1, const int8_t* s2, uint32_t n) {
    int32_t i;
    for (i = 0; i < n; i++) {
        if ((s1[i] != s2[i]) || (s1[i] == '\0') /* || s2[i] == '\0' */) {

            /* The s2[i] == '\0' is unnecessary because of the short-circuit
             * semantics of 'if' expressions in C.  If the first expression
             * (s1[i] != s2[i]) evaluates to false, that is, if s1[i] ==
             * s2[i], then we only need to test either s1[i] or s2[i] for
             * '\0', since we know they are equal. */
            return s1[i] - s2[i];
        }
    }
    return 0;
}

/* int8_t* strcpy(int8_t* dest, const int8_t* src)
 * Inputs:      int8_t* dest = destination string of copy
 *         const int8_t* src = source string of copy
 * Return Value: pointer to dest
 * Function: copy the source string into the destination string */
int8_t* strcpy(int8_t* dest, const int8_t* src) {
    int32_t i = 0;
    while (src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
    return dest;
}

/* int8_t* strcpy(int8_t* dest, const int8_t* src, uint32_t n)
 * Inputs:      int8_t* dest = destination string of copy
 *         const int8_t* src = source string of copy
 *                uint32_t n = number of bytes to copy
 * Return Value: pointer to dest
 * Function: copy n bytes of the source string into the destination string */
int8_t* strncpy(int8_t* dest, const int8_t* src, uint32_t n) {
    int32_t i = 0;
    while (src[i] != '\0' && i < n) {
        dest[i] = src[i];
        i++;
    }
    while (i < n) {
        dest[i] = '\0';
        i++;
    }
    return dest;
}

int32_t strcmp(int8_t* str1, int8_t* str2){
    int32_t i = 0;
    while(str1[i] != '\0'){
        if(str2[i] == '\0' || (str1[i] != str2[i])){
            return -1;
        }
        i++;
    }

    if(str2[i] != '\0'){return -1;}

    return 1;

}

/* void test_interrupts(void)
 * Inputs: void
 * Return Value: void
 * Function: increments video memory. To be used to test rtc */
void test_interrupts(void) {
    int32_t i;
    for (i = 0; i < NUM_ROWS * NUM_COLS; i++) {
        video_mem[i << 1]++;
    }
}

/* int max(a, b)
 * Inputs: a, b be the two number
 * Return Value: the larger one of the two
 * Function: Calculate the larger one of the two number */
int max(int a, int b){
    return a>b?a:b;
}


/* int32_t read(int32_t fd, void* buf, int32_t nbytes)
 * Inputs: fd : file descriptor
 *         buf: container provided by caller
 *         nbytes: number of bytes we want to read
 * Return Value: ret: whether the read is successful
 */

int32_t read(int32_t fd, void* buf, int32_t nbytes) {
    long ret;
    asm volatile ("INT $0x80"
    : "=a" (ret)
    : "a" (0x03), "b" (fd), "c" (buf), "d" (nbytes)
    : "memory", "cc");
    return ret;
}

/* int32_t write(int32_t fd, void* buf, int32_t nbytes)
 * Inputs: fd : file descriptor
 *         buf: container provided by caller
 *         nbytes: number of bytes we want to write
 * Return Value: ret: whether the write is successful
 */

int32_t write(int32_t fd, const void* buf, int32_t nbytes) {
    long ret;
    asm volatile ("INT $0x80"
    : "=a" (ret)
    : "a" (0x04), "b" (fd), "c" (buf), "d" (nbytes)
    : "memory", "cc");
    return ret;
}

/* int32_t open(const uint8_t* filename)
 * Inputs: filename : the file we want to open
 * Return Value: ret: whether the open is successful
 */

int32_t open(const uint8_t* filename) {
    long ret;
    asm volatile ("INT $0x80"
    : "=a" (ret)
    : "a" (0x05), "b" (filename)
    : "memory", "cc");
    return ret;
}

/* int32_t close(int32_t fd)
 * Inputs: fd : the file we want to close
 * Return Value: ret: whether the close is successful
 */
int32_t close(int32_t fd) {
    long ret;
    asm volatile ("INT $0x80"
    : "=a" (ret)
    : "a" (0x06), "b" (fd)
    : "memory", "cc");
    return ret;
}

/* int32_t ioctl(int32_t fd)
 * Inputs: fd : the target file
 *         cmd : the command to send
 * Return Value: ret: whether the close is successful
 */
int32_t ioctl(int32_t fd, int32_t cmd) {
    long ret;
    asm volatile ("INT $0x80"
    : "=a" (ret)
    : "a" (0x0D), "b" (fd), "c" (cmd)
    : "memory", "cc");
    return ret;
}

/* sleep
 * Description: it sleep for roughly a certain time
 * Inputs: time_in_ms - duration in ms unit
 * Return Value: ret: 0 if success, -1 if fail
 */
// int32_t sleep(uint32_t time_in_ms) {
//     int32_t fd;
//     int32_t *buf;
//     int32_t freq;
//     int32_t i;

//     if (time_in_ms == 0) return 0;

//     // open a rtc file
//     fd = open((uint8_t*) "rtc");
//     if (fd == -1) return -1;
//     // set the frequence to 1024 (close to 1000)
//     freq = 1024;
//     if (write(fd, &freq, 4) == -1) return -1;
//     // loop
//     for (i = 0; i < time_in_ms; ++i) {
//         read(fd, buf, 1);
//     }

//     close(fd);

//     return 0;
// }
