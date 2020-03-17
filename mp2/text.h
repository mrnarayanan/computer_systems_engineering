/*
 * tab:4
 *
 * text.h - font data and text to mode X conversion utility header file
 *
 * "Copyright (c) 2004-2009 by Steven S. Lumetta."
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without written agreement is
 * hereby granted, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 *
 * IN NO EVENT SHALL THE AUTHOR OR THE UNIVERSITY OF ILLINOIS BE LIABLE TO
 * ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
 * DAMAGES ARISING OUT  OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
 * EVEN IF THE AUTHOR AND/OR THE UNIVERSITY OF ILLINOIS HAS BEEN ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * THE AUTHOR AND THE UNIVERSITY OF ILLINOIS SPECIFICALLY DISCLAIM ANY
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE
 * PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND NEITHER THE AUTHOR NOR
 * THE UNIVERSITY OF ILLINOIS HAS ANY OBLIGATION TO PROVIDE MAINTENANCE,
 * SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS."
 *
 * Author:        Steve Lumetta
 * Version:       2
 * Creation Date: Thu Sep  9 22:08:16 2004
 * Filename:      text.h
 * History:
 *    SL    1    Thu Sep  9 22:08:16 2004
 *        First written.
 *    SL    2    Sat Sep 12 13:40:11 2009
 *        Integrated original release back into main code base.
 */

#ifndef TEXT_H
#define TEXT_H

#include <string.h>

/* The default VGA text mode font is 8x16 pixels. */
#define FONT_WIDTH   8
#define FONT_HEIGHT  16

// constants for # pixel in width and height of status bar
#define WIDTH_PIX    320
#define HEIGHT_PIX   18

/* Standard VGA text font. */
extern unsigned char font_data[256][16];

/*
 * strbuff
 *   DESCRIPTION: Populates buffer for status bar with color values of each pixel
 *   INPUTS: character array (string to display), buffer to write status bar to,
 *            level #
 *   OUTPUTS: changes status bar buffer
 *   RETURN VALUE: none
 *   SIDE EFFECTS: calls two helper functions
 */
extern void strbuff(char* input, unsigned char buff[HEIGHT_PIX][WIDTH_PIX], int lvl);

/*
 * _convasc
 *   DESCRIPTION: Displays the status bar on the video display. (helper function)
 *                Also displays level #, # fruits, time elapsed
 *   INPUTS: buffer to write status bar to, integer array with ASCII indices of chars to print,
 *            starting index of where to write chars, ending index of where to finish writing chars
 *   OUTPUTS: changes status bar buffer
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Uses font_data to determine bit patters for each character
 */
void _convasc(unsigned char buff[HEIGHT_PIX][WIDTH_PIX], int * letter, int start, int end);

/*
 * _changecolor
 *   DESCRIPTION: changes background color of status bar based on level (helper function)
 *   INPUTS: level #
 *   OUTPUTS: changes static variable for off color (background of status bar)
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void _changecolor(int lvl);

#endif /* TEXT_H */
