/*
 * tuxctl-ioctl.c
 *
 * Driver (skeleton) for the mp2 tuxcontrollers for ECE391 at UIUC.
 *
 * Mark Murphy 2006
 * Andrew Ofisher 2007
 * Steve Lumetta 12-13 Sep 2009
 * Puskar Naha 2013
 */

#include <asm/current.h>
#include <asm/uaccess.h>

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/file.h>
#include <linux/miscdevice.h>
#include <linux/kdev_t.h>
#include <linux/tty.h>
#include <linux/spinlock.h>

#include "tuxctl-ld.h"
#include "tuxctl-ioctl.h"
#include "mtcp.h"

#define debug(str, ...) printk(KERN_DEBUG "%s: " str, __FUNCTION__, ## __VA_ARGS__)
#define LED_BUFF_SIZE   6    // always sending 6 bytes to LED - opcode, # LEDs, LED data 0-3
#define DEC_PT_POS      0x10  // bitmask to set decimal point

static char ledmap[16] = {0xE7, 0x06, 0xCB, 0x8F, 0x2E, 0xAD, 0xED, 0x86,
                          0xEF, 0xAF, 0xEE, 0x6D, 0xE1, 0x4F, 0xE9, 0xE8}; // mapping for each hex char to LED

static spinlock_t packlock = SPIN_LOCK_UNLOCKED;

static unsigned b_static, c_static;
static int spam = 0; // flag to prevent LEDs from being spammed
static char buffer[LED_BUFF_SIZE];

/************************ Protocol Implementation *************************/

/* tuxctl_handle_packet()
 * DESCRIPTION: Receives packets sent by tux
 * INPUTS: tty struct, char array of packets
 * OUTPUTS: None
 * RETURN VALUE: None
 * SIDE EFFECTS: Stores packet data into static variables, processes bioc, ack, reset
 * IMPORTANT : Read the header for tuxctl_ldisc_data_callback() in
 * tuxctl-ld.c. It calls this function, so all warnings there apply
 * here as well.
 */
void tuxctl_handle_packet (struct tty_struct* tty, unsigned char* packet) {
    unsigned a, b, c;
    unsigned long sflags;

    a = packet[0]; /* Avoid printk() sign extending the 8-bit */
    b = packet[1]; /* values when printing them. */
    c = packet[2];

    switch (a) {
      case MTCP_BIOC_EVENT: // processes button packets from tux
        spin_lock_irqsave(&packlock, sflags);
        b_static = ~b;
        c_static = ~c;
        spin_unlock_irqrestore(&packlock, sflags);
        break;
      case MTCP_ACK: // resets spam flag based on if ACK was received
        spin_lock_irqsave(&packlock, sflags);
        spam = 0; // resets flag b/c tux acknowledged 
        spin_unlock_irqrestore(&packlock, sflags);
        break;
      case MTCP_RESET: // processes reset signal, restores previous LED state
      {
        char bioc[1] = {MTCP_BIOC_ON}; // buffer size = 1 b/c just opcode
        char set[1] = {MTCP_LED_USR}; // always want LEDs in user mode
        tuxctl_ldisc_put(tty, bioc, 1);
        tuxctl_ldisc_put(tty, set, 1);
        tuxctl_ldisc_put(tty, buffer, LED_BUFF_SIZE); // restore LEDs
        break;
      }
      default:
        break;
    }

    /*printk("packet : %x %x %x\n", a, b, c); */
}

/*
 * tuxctl_ioctl
 * DESCRIPTION: Dispatcher function for opcodes for commands tux needs to execute
 * Inputs: tty_struct struct, file struct, command, argument
 * Outputs: int 0 for success
 * Side Effects: calls routines for init, reading buttons, writing to LEDs
*/
int tuxctl_ioctl(struct tty_struct* tty, struct file* file,
                 unsigned cmd, unsigned long arg) {
/******** IMPORTANT NOTE: READ THIS BEFORE IMPLEMENTING THE IOCTLS ************
 *                                                                            *
 * The ioctls should not spend any time waiting for responses to the commands *
 * they send to the controller. The data is sent over the serial line at      *
 * 9600 BAUD. At this rate, a byte takes approximately 1 millisecond to       *
 * transmit; this means that there will be about 9 milliseconds between       *
 * the time you request that the low-level serial driver send the             *
 * 6-byte SET_LEDS packet and the time the 3-byte ACK packet finishes         *
 * arriving. This is far too long a time for a system call to take. The       *
 * ioctls should return immediately with success if their parameters are      *
 * valid.                                                                     *
 *                                                                            *
 ******************************************************************************/
    unsigned long lflags;
    switch (cmd) {
        /* initializes tux, sets LEDs to user mode */
        case TUX_INIT:
        {
          //return init(tty);
          // set driver data to 0, MTCP_BIOC_ON, MTCP_CLK_RESET
          char bioc[1] = {MTCP_BIOC_ON};
      //    char clk[1] = {MTCP_CLK_RESET};
          char set[1] = {MTCP_LED_USR}; // always want LEDs in user mode
          tuxctl_ldisc_put(tty, bioc, 1);
      //    tuxctl_ldisc_put(tty, clk, 1);
          tuxctl_ldisc_put(tty, set, 1);
          return 0;
        }
        /* processes button inputs from packets, sets which are pressed for user */
        case TUX_BUTTONS:
        {
          unsigned long * arg_ptr;
          unsigned long buf[1];
          arg_ptr = (unsigned long *) arg;
          if (copy_from_user(buf, arg_ptr, 1) != 0) // to, from, size
            return -EINVAL;
          else
          {
            spin_lock_irqsave(&packlock, lflags);
            buf[0] = 0;
            if ( (b_static & 0x08) == 0x08 ) // C bitmask
            {
              buf[0] |= 0x08;
          //    printk("C\n");
            }
            if ( (b_static & 0x04) == 0x04 ) // B bitmask
            {
              buf[0] |= 0x04;
          //    printk("B\n");
            }
            if ( (b_static & 0x02) == 0x02 ) // A bitmask
            {
              buf[0] |= 0x02;
          //    printk("A\n");
            }
            if ( (b_static & 0x01) == 0x01 ) // START bitmask
            {
              buf[0] |= 0x01;
            //  printk("START\n");
            }
            if ( (c_static & 0x08) == 0x08 ) // right bitmask
            {
              buf[0] |= 0x80;
          //    printk("right\n");
            }
            if ( (c_static & 0x04) == 0x04 ) // down bitmask
            {
              buf[0] |= 0x20;
            //  printk("down\n");
            }
            if ( (c_static & 0x02) == 0x02 ) // left bitmask
            {
              buf[0] |= 0x40;
            //  printk("left\n");
            }
            if ( (c_static & 0x01) == 0x01 ) // up bitmask
            {
              buf[0] |= 0x10;
          //    printk("up\n");
            }
            spin_unlock_irqrestore(&packlock, lflags);

      //      printk("%lu", buf[0]);
            copy_to_user(arg_ptr, buf, 1); // to, from, size

            return 0;
          }
        }
        /* Sets LEDs by mapping input characters to 7 segment display and decimal point */
        case TUX_SET_LED:
        {
          char c3 = 0x00, c2 = 0x00, c1 = 0x00, c0 = 0x00;
          unsigned long index = 0;

          if (spam == 1) // don't set LEDs if previous write not complete
            return 0;

          buffer[0] = MTCP_LED_SET;
          buffer[1] = 0x0F; // light up all LEDs (even if spaces)

          if ( (arg & 0x00080000) == 0x00080000) // LED 3 bitmask
          {
            index = (arg & 0x0000F000) >> 12; // to shift into lowest 4 bits
            c3 = ledmap[index];
          }
          if ( (arg & 0x00040000) == 0x00040000) // LED 2 bitmask
          {
            index = (arg & 0x00000F00) >> 8; // to shift into lowest 4 bits
            c2 = ledmap[index];
          }
          if ( (arg & 0x00020000) == 0x00020000) // LED 1 bitmask
          {
            index = (arg & 0x000000F0) >> 4; // to shift into lowest 4 bits
            c1 = ledmap[index];
          }
          if ( (arg & 0x00010000) == 0x00010000) // LED 0 bitmask
          {
            index = (arg & 0x0000000F); // to shift into lowest 4 bits
            c0 = ledmap[index];
          }

          // check which decimal points are on
          if ( (arg & 0x08000000) == 0x08000000) // decimal point 3 bitmask
          {
            c3 = c3 | DEC_PT_POS;
          }
          if ( (arg & 0x04000000) == 0x04000000) // decimal point 2 bitmask
          {
            c2 = c2 | DEC_PT_POS;
          }
          if ( (arg & 0x02000000) == 0x02000000) // decimal point 1 bitmask
          {
            c1 = c1 | DEC_PT_POS;
          }
          if ( (arg & 0x01000000) == 0x01000000) // decimal point 0 bitmask
          {
            c0 = c0 | DEC_PT_POS;
          }

          buffer[2] = c0;
          buffer[3] = c1;
          buffer[4] = c2;
          buffer[5] = c3;

          tuxctl_ldisc_put(tty, buffer, LED_BUFF_SIZE);
          spin_lock_irqsave(&packlock, lflags);
          spam = 1; // sets spam flag
          spin_unlock_irqrestore(&packlock, lflags);
          return 0;
        }
        default:
          return -EINVAL;
    }
}
