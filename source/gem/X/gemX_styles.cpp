/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

/* This file contains functions for the style objects in Gem.

*/

#include <X11/Xlib.h>

#include <iostream>

#include <am_inc.h>

#include GEM__H
#include GEMX__H
#include REGISTRY__H

// // // // // // // // // // // // // // // // // // // // // // // // // //
// Am_Style
// // // // // // // // // // // // // // // // // // // // // // // // // //

AM_WRAPPER_IMPL (Am_Style);

const char* Am_Style::Get_Color_Name () const
{
  if (data)
    return data->color_name;
  else
    return (0L);
}

static Am_Style default_style (0.0,0.0,0.0); // used by Get_Values

void Am_Style::Get_Values (float& r, float& g, float& b) const
{
  if (data) {
    r = data->red;
    g = data->green;
    b = data->blue;
  }
  else {
    r = g = b = 0.0;
  }
}

void Am_Style::Get_Values (short& thickness,
			   Am_Line_Cap_Style_Flag& cap,
			   Am_Join_Style_Flag& join,
			   Am_Line_Solid_Flag& line_flag,
			   const char*& dash_l, int& dash_l_length,
			   Am_Fill_Solid_Flag& fill_flag,
			   Am_Fill_Poly_Flag& poly,
			   Am_Image_Array& stipple) const
{
  if (data) {
    thickness = data->line_thickness;
    cap = data->cap_style;
    join = data->join_style;
    line_flag = data->line_solid;
    dash_l = data->dash_list;
    dash_l_length = data->dash_list_length;
    fill_flag = data->fill_solid;
    poly = data->fill_poly;
    stipple = data->stipple_bitmap;
  }
  else {
    // return default values
    default_style.Get_Values (thickness, cap, join, line_flag, dash_l,
			      dash_l_length, fill_flag, poly, stipple);
  }
}

void Am_Style::Get_Values (float& r, float& g, float& b,
			   short& thickness,
			   Am_Line_Cap_Style_Flag& cap,
			   Am_Join_Style_Flag& join,
			   Am_Line_Solid_Flag& line_flag,
			   const char*& dash_l, int& dash_l_length,
			   Am_Fill_Solid_Flag& fill_flag,
			   Am_Fill_Poly_Flag& poly,
			   Am_Image_Array& stipple) const
{
  Get_Values (r, g, b);
  Get_Values (thickness, cap, join, line_flag, dash_l, dash_l_length,
                fill_flag, poly, stipple);
}

Am_Fill_Solid_Flag Am_Style::Get_Fill_Flag () const
{
  if (data)
    return data->fill_solid;
  else
    return Am_FILL_SOLID;
}

Am_Line_Solid_Flag Am_Style::Get_Line_Flag () const
{
  if (data)
    return data->line_solid;
  else
    return Am_LINE_SOLID;
}

Am_Fill_Poly_Flag Am_Style::Get_Fill_Poly_Flag () const
{
  if (data)
    return data->fill_poly;
  else
    return Am_FILL_POLY_EVEN_ODD;
}

Am_Image_Array Am_Style::Get_Stipple () const
{
  if (data)
    return data->stipple_bitmap;
  else
    return Am_No_Image;
}

void Am_Style::Get_Line_Thickness_Values (short& thickness,
					 Am_Line_Cap_Style_Flag& cap) const
{
  if (data) {
    thickness = data->line_thickness;
    cap = data->cap_style;
  }
  else {
    default_style.Get_Line_Thickness_Values (thickness, cap);
  }
}
    
Am_Style::Am_Style (float r, float g, float b,  //color part
		    short thickness,
		    Am_Line_Cap_Style_Flag cap,
		    Am_Join_Style_Flag join,
		    Am_Line_Solid_Flag line_flag,
		    const char *dash_l, int dash_l_length,
		    Am_Fill_Solid_Flag fill_flag,
		    Am_Fill_Poly_Flag poly,
		    // stipple must be an opal bitmap object
		    Am_Image_Array stipple)
{
  data = new Am_Style_Data (r, g, b, thickness, cap, join,
			    line_flag, dash_l, dash_l_length,
			    fill_flag, poly, stipple);
}

Am_Style::Am_Style (const char* color_name,
		    short thickness,
		    Am_Line_Cap_Style_Flag cap,
		    Am_Join_Style_Flag join,
		    Am_Line_Solid_Flag line_flag,
		    const char* dash_l, int dash_l_length,
		    Am_Fill_Solid_Flag fill_flag,
		    Am_Fill_Poly_Flag poly,
		    // stipple must be an opal bitmap object
		    Am_Image_Array stipple)
{
  data = new Am_Style_Data (color_name, thickness, cap, join,
			    line_flag, dash_l, dash_l_length,
			    fill_flag, poly, stipple);
}

// Do not use this 
// Am_Style::Am_Style (char* bit_data, int height, int width)
// {
//   data = new Am_Style_Data (bit_data, height, width);
// }

Am_Style::Am_Style ()
{
  data = (0L);
}

Am_Style Am_Style::Halftone_Stipple (int percent,
				     Am_Fill_Solid_Flag fill_flag)
{
  Am_Style sty (new Am_Style_Data(percent, fill_flag));
  return sty;
}

Am_Style Am_Style::Thick_Line (unsigned short thickness)
{
  Am_Style style (0, 0, 0, thickness);
  return style;
}

Am_Style Am_Style::Clone_With_New_Color (Am_Style& foreground) const
{
  return new Am_Style_Data (data, foreground.data);
}

bool Am_Style::operator== (const Am_Style& style) const
{
  return data == style.data;
}

bool Am_Style::operator!= (const Am_Style& style) const
{
  return data != style.data;
}

void Am_Style::Add_Image(Am_Image_Array image)
{
  if (data)
    data = (Am_Style_Data *)data->Make_Unique();
  else
    data = new Am_Style_Data (0.0, 0.0, 0.0, 0, Am_CAP_BUTT, Am_JOIN_MITER,
			      Am_LINE_SOLID, Am_DEFAULT_DASH_LIST,
			      Am_DEFAULT_DASH_LIST_LENGTH, Am_FILL_SOLID,
			      Am_FILL_POLY_EVEN_ODD, 0);
  data->stipple_bitmap = image;
  data->fill_solid = Am_FILL_STIPPLED;
}

Am_Style Am_No_Style;

Am_Style_Data Am_On_Bits_Data ("Am_On_Bits", true);
Am_Style Am_On_Bits (&Am_On_Bits_Data);

Am_Style_Data Am_Off_Bits_Data ("Am_Off_Bits", false);
Am_Style Am_Off_Bits (static_cast<Am_Wrapper*>(&Am_Off_Bits_Data));

// // // // // // // // // // // // // // // // // // // // // // // // // //
// Am_Style_Data
// // // // // // // // // // // // // // // // // // // // // // // // // //

AM_WRAPPER_DATA_IMPL (Am_Style, (this));

Am_Style_Data* Am_Style_Data::list = (0L);

Am_Style_Data::Am_Style_Data (Am_Style_Data* proto)
  : color_name(0L),
    main_display(0),
    color_head(0L),
    color_allocated(proto->color_allocated),
    red(proto->red),
    green(proto->green),
    blue(proto->blue),
    line_thickness(proto->line_thickness),
    cap_style(proto->cap_style),
    join_style(proto->join_style),
    line_solid(proto->line_solid),
    dash_list_length(proto->dash_list_length),
    fill_solid(proto->fill_solid),
    fill_poly(proto->fill_poly),
    stipple_bitmap(proto->stipple_bitmap)
{
  memcpy (dash_list, proto->dash_list, dash_list_length * sizeof(char));
  if (proto->color_name != 0L) {
    char* color_hold = new char [strlen (proto->color_name) + 1];
    strcpy (color_hold, proto->color_name);
    color_name = color_hold;
  } // else: color_name already 0L

  next = list;
  list = this;     
}
  
Am_Style_Data::Am_Style_Data (float r, float g, float b,
               short thickness,  Am_Line_Cap_Style_Flag cap,
               Am_Join_Style_Flag join,  Am_Line_Solid_Flag line_flag,
               const char* dash_l, int dash_l_length,
               Am_Fill_Solid_Flag fill_flag,
               Am_Fill_Poly_Flag poly,  Am_Image_Array stipple)
  : color_name(0L),
    main_display(0),
    color_head(0L),
    color_allocated(true),
    red(r), green(g), blue(b),
    line_thickness(thickness),
    cap_style(cap),
    join_style(join),
    line_solid(line_flag),
    dash_list_length(dash_l_length),
    fill_solid(fill_flag),
    fill_poly(poly),
    stipple_bitmap(stipple)
{
  dash_list = new char [dash_list_length];
  memcpy (dash_list, dash_l, dash_list_length);
   
  next = list;
  list = this;     
}

Am_Style_Data::Am_Style_Data (const char * cname,
	      short thickness,  Am_Line_Cap_Style_Flag cap,
	      Am_Join_Style_Flag join,  Am_Line_Solid_Flag line_flag,
	      const char* dash_l, int dash_l_length,
	      Am_Fill_Solid_Flag fill_flag,
              Am_Fill_Poly_Flag poly,  Am_Image_Array stipple)
  : main_display(0),
    color_head(0L),
    color_allocated(true),
    line_thickness(thickness),
    cap_style(cap),
    join_style(join),
    line_solid(line_flag),
    dash_list_length(dash_l_length),
    fill_solid(fill_flag),
    fill_poly(poly),
    stipple_bitmap(stipple)
{
  dash_list = new char [dash_list_length];
  memcpy (dash_list, dash_l, dash_list_length);

  char* color_hold = new char [strlen (cname) + 1];
  strcpy (color_hold, cname);
  color_name = color_hold;
      
  next = list;
  list = this;     
#ifdef DEBUG
  Am_Register_Name(this, cname);
#endif
}

Am_Style_Data::Am_Style_Data (int percent, Am_Fill_Solid_Flag fill_flag)
  : color_name(0L),
    main_display(0),
    color_head(0L),
    color_allocated(true),
    red(0.0), green(0.0), blue(0.0),
    line_thickness(0),
    cap_style(Am_CAP_BUTT),
    join_style(Am_JOIN_MITER),
    line_solid(Am_LINE_SOLID),
    dash_list_length(Am_DEFAULT_DASH_LIST_LENGTH),
    fill_solid(fill_flag),
    fill_poly(Am_FILL_POLY_EVEN_ODD),
    stipple_bitmap(Am_Image_Array(percent))
{
  const char* dash_l = Am_DEFAULT_DASH_LIST;
  dash_list = new char [dash_list_length];
  memcpy (dash_list, dash_l, dash_list_length);

  //  stipple_bitmap = Am_Image_Array(percent);

  next = list;
  list = this;     
}

Am_Style_Data::Am_Style_Data (const char* name, bool bit_is_on)
  : color_name(name),
    main_display(0),
    color_head(0L),
    color_allocated(false),
    line_thickness(0),
    cap_style(Am_CAP_BUTT),
    join_style(Am_JOIN_MITER),
    line_solid(Am_LINE_SOLID),
    dash_list_length(Am_DEFAULT_DASH_LIST_LENGTH),
    fill_solid(Am_FILL_SOLID),
    fill_poly(Am_FILL_POLY_EVEN_ODD),
    stipple_bitmap(Am_No_Image)
{
  const char* dash_l = Am_DEFAULT_DASH_LIST;
  dash_list = new char [dash_list_length];
  memcpy (dash_list, dash_l, dash_list_length);
#ifdef DEBUG
  Am_Register_Name(this, name);
#endif
  main_color.pixel = (bit_is_on) ? 1 : 0;
  Note_Reference (); // TODO: Check if this is needed ?
  // TODO: Check why we do not register in list ?
}

Am_Style_Data::Am_Style_Data (Am_Style_Data* proto, Am_Style_Data* new_color)
  : main_display(0),
    color_head(0L),
    color_allocated(new_color->color_allocated), // from other color
    red(new_color->red),
    green(new_color->green),
    blue(new_color->blue),
    line_thickness(proto->line_thickness),       // then from proto
    cap_style(proto->cap_style),
    join_style(proto->join_style),
    line_solid(proto->line_solid),
    dash_list_length(proto->dash_list_length),
    fill_solid(proto->fill_solid),
    fill_poly(proto->fill_poly),
    stipple_bitmap(proto->stipple_bitmap)
{
  if (new_color->color_name && color_allocated) {
    char* color_hold = new char [strlen (new_color->color_name) + 1];
    strcpy (color_hold, new_color->color_name);
    color_name = color_hold;
  }
  else
    color_name = new_color->color_name;

  dash_list = new char [dash_list_length];
  memcpy (dash_list, proto->dash_list, dash_list_length * sizeof(char));

  main_color.pixel = new_color->main_color.pixel;
  // TODO: Check why we do not register in list ?
}

Am_Style_Data::~Am_Style_Data ()
{
  if (color_name && color_allocated) delete (char*)color_name;
  if (dash_list) delete [] dash_list;
  if (main_display) {
    // BUG: X index deletion will not work properly if some application
    // has changed the default colormap.  
    // I'm not sure that's actually a bug (af1x 12-6-95)
    if (color_allocated) {
      int screen_num = DefaultScreen(main_display);
      Colormap c = XDefaultColormap(main_display, screen_num);
      XFreeColors(main_display, c, &main_color.pixel, 1, 0);
      // simulate an array of length 1
    }
  }
  Color_Index* current = color_head;
  Color_Index* next = (0L);
  while (current) {
    next = current->next;
    current->next = (0L);
    if (current->dpy) {
      if (color_allocated) {
        int screen_num = DefaultScreen(current->dpy);
        Colormap c = XDefaultColormap(current->dpy, screen_num);
        XFreeColors(current->dpy, c, &current->index.pixel, 1, 0);
        // simulate an array of length 1
      }
    }
    delete current;
    current = next;
  }
  color_head = (0L);
  remove (this);
}

void Am_Style_Data::remove (Am_Style_Data* style)
{
  Am_Style_Data* prev = (0L);
  Am_Style_Data* curr = list;
  while (curr) {
    if (curr == style) {
      if (prev)
        prev->next = curr->next;
      else
        list = curr->next;
      return;
    }
    prev = curr;
    curr = curr->next;
  }
}

void Am_Style_Data::remove (Display* display)
{
  Am_Style_Data* curr;
  for (curr = list; curr; curr = curr->next) {
    if (curr->main_display == display) {
      int screen_num = DefaultScreen (curr->main_display);
      Colormap c = XDefaultColormap (curr->main_display, screen_num);
      XFreeColors (curr->main_display, c, &curr->main_color.pixel, 1, 0);
      curr->main_display = (0L);
    }
    Color_Index* prev = (0L);
    Color_Index* curr_index = curr->color_head;
    while (curr_index) {
      if (curr_index->dpy == display) {
        if (prev)
          prev->next = curr_index->next;
        else
          curr->color_head = curr_index->next;
        int screen_num = DefaultScreen (curr_index->dpy);
        Colormap c = XDefaultColormap (curr_index->dpy, screen_num);
        XFreeColors (curr_index->dpy, c, &curr_index->index.pixel, 1, 0);
        delete curr_index;
        break;
      }
      prev = curr_index;
      curr_index = curr_index->next;
    }
  }
}

//gets the index to use on display
unsigned long Am_Style_Data::Get_X_Index (const Am_Drawonable_Impl* d)
{
  XColor temp = Get_X_Color(d);
  return temp.pixel;
}

XColor Am_Style_Data::Get_X_Color (const Am_Drawonable_Impl* d)
{
  Display* disp = d->Get_Display();
  //unsigned long index;
  XColor x;
  if (disp == main_display)    {
    return main_color;
  }
  else if (Get_Color_Index(disp, x)) { // test for other displays
    return x;
  }
  else {
    if (color_allocated) {
      Colormap c = d->Get_Colormap();
      if (color_name) {
        if (!XParseColor (disp, c, color_name, &x)) {
         std::cerr << "** Color name " << color_name << " not in database:";
         std::cerr <<std::endl << "**   using black instead." <<std::endl;
          x.red = 0;
          x.blue = 0;
          x.green = 0;
        }
      }    
      else {
        x.red   = (unsigned short) (red*0xFFFF + 0.5);
        x.green = (unsigned short) (green*0xFFFF + 0.5);
        x.blue  = (unsigned short) (blue*0xFFFF + 0.5);
        x.flags = DoRed | DoGreen | DoBlue;
      }
      d->Allocate_Closest_Color(x);
    }
    else {
      unsigned long pixel = 0;
      if (main_color.pixel & 1) {
        int depth = d->screen->depth;
        int i;
        for (i = 0; i < depth; ++i) {
          pixel <<= 1;
          pixel |= 1;
        }
      }
      x.pixel = pixel;
    }
    if (!main_display) {
      main_display = disp;
      main_color = x;
    }
    else
      Add_Color_Index (disp, x);
    return x;
  }
}

void Am_Style_Data::Add_Color_Index (Display* display, XColor index)
{
  Color_Index* new_node = new Color_Index (display, index);
  new_node->next = color_head;
  color_head = new_node;
}

bool Am_Style_Data::Get_Color_Index (Display* dpy, XColor& index)
{
  Color_Index* current;
  for (current = color_head; current != (0L); current = current->next)
    if (current->dpy == dpy) {
      index = current->index;
      return true;
    }
  return false;
}

void Am_Style_Data::Print (std::ostream& os) const {
  const char *lookup_name = (0L);
#ifdef DEBUG
  lookup_name = Am_Get_Name_Of_Item(this);
#endif
  if (lookup_name) os << lookup_name;
  else {
    os << "Am_Style(" <<std::hex << (unsigned long)this <<std::dec << ")=[";
    if (color_name) os << "color=" << color_name;
    else os << "color=(" << red << "," << green << "," << blue << ")";
    os << " thickness=" << line_thickness;
    //don't bother with the rest of the fields
    os << " ...]";
  }
}
