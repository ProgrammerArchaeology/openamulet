/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

#include <am_inc.h>

#include GEM__H
#include MISC__H

#include "test_utils.h"

#include <string.h>
#include <stdlib.h>

// image filenames
#if defined(_WINDOWS) || defined(_MACINTOSH)

#define IMFN_GREY	"lib/images/grey.gif"
#define IMFN_HI		"lib/images/hi.gif"
#define IMFN_SMILE	"lib/images/smile.gif" 
#define IMFN_FUN	"lib/images/fun.gif"
#define IMFN_DIAMOND	"lib/images/diamond.gif"
#define IMFN_ENT	"lib/images/ent.gif"	  
#define IMFN_ENT_D	"lib/images/ent_d.gif"
#define IMFN_KLING	"lib/images/kling.gif"
#define IMFN_HAPPY	"lib/images/happy.gif"
#define IMFN_HAPPY_M	"lib/images/happy_m.gif"
#define IMFN_DS9	"lib/images/ds9.gif"
#define IMFN_DS9_M	"lib/images/ds9_m.gif"
#define IMFN_BORG	"lib/images/borg.gif"
#define IMFN_BORG_M	"lib/images/borg_m.gif"
#define IMFN_CROSSHR	"lib/images/crsshr.gif"
#define IMFN_CROSSHR_M	"lib/images/crsshr_m.gif"
#define IMFN_ROM	"lib/images/rom.gif"
#define IMFN_ROM_M	"lib/images/rom_m.gif"
#define AMULET_GIF	"lib/images/Am_med.gif"
#else
// Unix names
#define IMFN_GREY	"lib/images/grey.xbm"
#define IMFN_HI		"lib/images/hi.xbm"
#define IMFN_SMILE	"lib/images/smile.xbm" 
#define IMFN_FUN	"lib/images/fun.xbm"
#define IMFN_DIAMOND	"lib/images/diamond.xbm"
#define IMFN_ENT	"lib/images/ent.xbm"
#define IMFN_ENT_D	"lib/images/ent_d.xbm"
#define IMFN_KLING	"lib/images/kling.xbm"
#define IMFN_HAPPY	"lib/images/happy.xbm"
#define IMFN_HAPPY_M	"lib/images/happy_mask.xbm"
#define IMFN_DS9	"lib/images/ds9.xbm"
#define IMFN_DS9_M	"lib/images/ds9_mask.xbm"
#define IMFN_BORG	"lib/images/borg.xbm"
#define IMFN_BORG_M	"lib/images/borg_mask.xbm"
#define IMFN_CROSSHR	"lib/images/crosshair.xbm"
#define IMFN_CROSSHR_M	"lib/images/crosshair_mask.xbm"
#define IMFN_ROM	"lib/images/rom.xbm"
#define IMFN_ROM_M	"lib/images/rom_mask.xbm"
#define AMULET_GIF	"lib/images/Am_med.gif"
#endif

void testfiles(wins *wins_ar, int ar_len )
{
  static const char* filenames[5] = { IMFN_GREY, IMFN_HI, IMFN_SMILE,
				IMFN_FUN, IMFN_DIAMOND };
  static const char* colornames[5] = { "purple", "green", "blue", "black", "red" };

  for (int i=0; i<ar_len; i++) {
    Am_Drawonable *d = wins_ar[i].d1;
    for (int n=0; n<5; n++) {
      Am_Image_Array image_array (Am_Merge_Pathname(filenames[n]));
      if (!image_array.Valid ())
	Am_Error ("bitmap not found");
      Am_Style image_style (colornames[n], 8, Am_CAP_BUTT,
			    Am_JOIN_MITER, Am_LINE_ON_OFF_DASH,
			    Am_DEFAULT_DASH_LIST, Am_DEFAULT_DASH_LIST_LENGTH,
			    Am_FILL_STIPPLED, Am_FILL_POLY_EVEN_ODD,
			    image_array);
      int left = 10 + 50*n;
      d->Draw_Rectangle(Am_White, image_style, left, 10, 40, 40);
    }
  }
}

void testpercents(wins *wins_ar, int ar_len ) {
  for (int i=0; i<ar_len; i++) {
    Am_Drawonable *d = wins_ar[i].d2;

    int final_left = 0;
    int final_top = 0;
    
    for (int n=0; n<100; n=n+6) {
      Am_Image_Array image_array (n);
      Am_Style image_style ("red", 8, Am_CAP_BUTT,
			    Am_JOIN_MITER, Am_LINE_ON_OFF_DASH,
			    Am_DEFAULT_DASH_LIST, Am_DEFAULT_DASH_LIST_LENGTH,
			    Am_FILL_STIPPLED, Am_FILL_POLY_EVEN_ODD,
			    image_array);
      int left = 10 + (((n / 6) % 3) * 30);
      int top  = 8 + (((n / 6) / 3) * 29);
      d->Draw_Rectangle(Am_No_Style, image_style, left, top, 25, 25);
      final_left = left;
      final_top = top;
    }

	/*
    static char my_diamond_bits[] = {
        0x80, 0x00, 0xc0, 0x01, 0xe0, 0x03, 0xf0, 0x07, 0xf8, 0x0f, 0xfc, 0x1f,
        0xfe, 0x3f, 0xff, 0x7f, 0xfe, 0x3f, 0xfc, 0x1f, 0xf8, 0x0f, 0xf0, 0x07,
        0xe0, 0x03, 0xc0, 0x01, 0x80, 0x00, 0x00, 0x00};
*/
	static char my_diamond_bits[] = {
		'\200',  '\0', '\300',   '\1', '\340',  '\3', '\360',  '\7', '\370', '\17', '\374', '\37',
		'\376', '\77', '\377', '\177', '\376', '\77', '\374', '\37', '\370', '\17', '\360',  '\7',
		'\340',  '\3', '\300',   '\1', '\200',  '\0',   '\0',  '\0'};
	
    // Style from image array
    
    Am_Image_Array my_diamond_image (my_diamond_bits, 16, 16);
    Am_Style my_diamond_fill ("red", 8, Am_CAP_BUTT,
			    Am_JOIN_MITER, Am_LINE_ON_OFF_DASH,
			    Am_DEFAULT_DASH_LIST, Am_DEFAULT_DASH_LIST_LENGTH,
			    Am_FILL_STIPPLED, Am_FILL_POLY_EVEN_ODD,
			    my_diamond_image);
    
    d->Draw_Rectangle (Am_Black, my_diamond_fill, final_left+30, final_top, 25, 25);
  }
}

void testopaque(wins *wins_ar, int ar_len )
{
  for (int i=0; i<ar_len; i++) {
    Am_Drawonable *d = wins_ar[i].d3;

  d->Draw_Rectangle(Am_White, Am_Blue, 10, 10, 25, 25);

  d->Draw_Rectangle(Am_White, Am_Blue, 30, 30, 25, 25);

  Am_Image_Array grey (50);
  Am_Style grey_style (1.0f, 0.0f, 0.0f, 8, Am_CAP_BUTT,
		     Am_JOIN_MITER, Am_LINE_ON_OFF_DASH,
		     Am_DEFAULT_DASH_LIST, Am_DEFAULT_DASH_LIST_LENGTH,
		     Am_FILL_STIPPLED, Am_FILL_POLY_EVEN_ODD, grey);
  d->Draw_Rectangle(Am_White, grey_style, 10, 10, 25, 25);
    
  Am_Image_Array grey2 (10);
  Am_Style opaque_style (1.0f, 0.0f, 0.0f, 8, Am_CAP_BUTT,
		     Am_JOIN_MITER, Am_LINE_ON_OFF_DASH,
		     Am_DEFAULT_DASH_LIST, Am_DEFAULT_DASH_LIST_LENGTH,
		     Am_FILL_OPAQUE_STIPPLED, Am_FILL_POLY_EVEN_ODD, grey2);
  d->Draw_Rectangle(Am_White, opaque_style, 30, 30, 25, 25);

  Am_Style foo (1.0f, 0.0f, 0.0f, 8, Am_CAP_BUTT,
		     Am_JOIN_MITER, Am_LINE_ON_OFF_DASH,
		     Am_DEFAULT_DASH_LIST, Am_DEFAULT_DASH_LIST_LENGTH,
		     Am_FILL_STIPPLED, Am_FILL_POLY_EVEN_ODD, grey2);

  d->Draw_Rectangle(Am_White, foo, 60, 60, 25, 25);
  }
}

void testimages(wins *wins_ar, int ar_len )
{
  for (int i=0; i<ar_len; i++) {
    Am_Drawonable *d = wins_ar[i].d4;

    Am_Image_Array ent (Am_Merge_Pathname(IMFN_ENT));

    if (!ent.Valid ())
      Am_Error ("ent bitmap not found");
    d->Draw_Image (160, 15, -1, -1, ent, 0, 0, Am_Purple, Am_White);
    d->Flush_Output();
    
    Am_Image_Array entd (Am_Merge_Pathname(IMFN_ENT_D));
    if (!entd.Valid ())
      Am_Error ("entd bitmap not found");
    d->Draw_Image (160, 55, -1, -1, entd, 0, 0, Am_Black, Am_No_Style);
    
    Am_Image_Array kling (Am_Merge_Pathname(IMFN_KLING));
    if (!kling.Valid ())
      Am_Error ("kling bitmap not found");
    d->Draw_Image (160, 95, -1, -1, kling, 0, 0, Am_Red, Am_White);
    
    std::cerr << "testbits: Testing bogus GIF file name." << std::endl;
    Am_Image_Array bogus ("bogus_file.gif");

    Am_Image_Array amulet (Am_Merge_Pathname(AMULET_GIF));
    if (!amulet.Valid())
      Am_Error ("Amulet gif file not found");
    d->Draw_Image (15, 15, -1, -1, amulet, 0, 0, Am_Black, Am_White);
    // the bg/fg styles shouldn't matter
  }
}

void testcursors(wins *wins_ar, int ar_len ) {
  for (int i=0; i<ar_len; i++) {
    Am_Drawonable *d1 = wins_ar[i].d1;
    Am_Drawonable *d2 = wins_ar[i].d2;
    Am_Drawonable *d3 = wins_ar[i].d3;
    Am_Drawonable *d4 = wins_ar[i].d4;
    Am_Drawonable *d5 = wins_ar[i].d5;

  Am_Image_Array happy (Am_Merge_Pathname(IMFN_HAPPY));
  Am_Image_Array happy_mask (Am_Merge_Pathname(IMFN_HAPPY_M));
  Am_Cursor happy_cursor (happy, happy_mask, Am_Purple, Am_White);
  if (happy_cursor.Valid())
    d1->Set_Cursor(happy_cursor);
  else
    fprintf(stderr, "happy_cursor didn't work.\n");

   Am_Image_Array ds9 (Am_Merge_Pathname(IMFN_DS9));
   Am_Image_Array ds9_mask (Am_Merge_Pathname(IMFN_DS9_M));
   Am_Cursor ds9_cursor (ds9, ds9_mask, Am_Blue, Am_White);
   if (ds9_cursor.Valid()) 
     d2->Set_Cursor(ds9_cursor);
   else
     fprintf(stderr, "ds9_cursor didn't work.\n");

   Am_Image_Array borg (Am_Merge_Pathname(IMFN_BORG));
   Am_Image_Array borg_mask (Am_Merge_Pathname(IMFN_BORG_M));
   Am_Cursor borg_cursor (borg, borg_mask, Am_Red, Am_White);
   if (borg_cursor.Valid()) 
     d3->Set_Cursor(borg_cursor);
   else
     fprintf(stderr, "borg_cursor didn't work.\n");

   Am_Image_Array crosshair (Am_Merge_Pathname(IMFN_CROSSHR));
   Am_Image_Array crosshair_mask (Am_Merge_Pathname(IMFN_CROSSHR_M));
   Am_Cursor crosshair_cursor (crosshair, crosshair_mask, Am_Black, Am_White);
   if (crosshair_cursor.Valid()) 
     d4->Set_Cursor(crosshair_cursor);
   else
     fprintf(stderr, "crosshair_cursor didn't work.\n");

   Am_Image_Array rom (Am_Merge_Pathname(IMFN_ROM));
   Am_Image_Array rom_mask (Am_Merge_Pathname(IMFN_ROM_M));
   Am_Cursor rom_cursor (rom, rom_mask, Am_Green, Am_White);
   if (rom_cursor.Valid()) 
     d5->Set_Cursor(rom_cursor);
   else
     fprintf(stderr, "rom_cursor didn't work.\n");
  }  
} 

void test_bits (wins *wins_ar, int ar_len )
{ 

  for (int i=0; i<ar_len; i++) {
    Am_Drawonable *the_bit_win =
      (wins_ar[i].root)->Create (100, 40, 255, 599);
    wins_ar[i].d1 = the_bit_win;
    wins_ar[i].d2 = the_bit_win->Create (0, 60, 110, 180);
    wins_ar[i].d3 = the_bit_win->Create (115, 60, 100, 100,
					 "Opaque Stipple Test",
					 "Opaque Icon", true, false,
					 white);
    wins_ar[i].d4 = the_bit_win->Create (0, 245, 251, 350,
				    "Gemsd4 Am_Image_Array Test",
				    "Am_Image_Array", true, false,
				    Am_Yellow);
    wins_ar[i].d5 = the_bit_win->Create (115, 165, 50, 50);
    the_bit_win->Flush_Output ();
  }

  testfiles(wins_ar, ar_len);
  testpercents(wins_ar, ar_len);
  testopaque(wins_ar, ar_len);
  testimages(wins_ar, ar_len);
  testcursors(wins_ar, ar_len);

}
