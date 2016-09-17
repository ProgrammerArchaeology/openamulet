/* ************************************************************************
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

// TODO:
//   -- If you start the demo fresh, click to create a ship, Undo, then Redo,
//      the ship appears in the short-range scan but not in the long-range scan
//   -- Ships can be dragged out of the window, and also outside the bbox
//      of SP_Ship_Agg, so that they are not selectable again.
//   -- Cloaked movement: use interim feedback when shift-leftdown
//   -- Beep when click tractor beam in background
//   -- Should be able to destroy tractor beams w/o destroying ship
//   -- Tractor beams should affect behavior of ships:
//      -- A ship that is the destination of a tractor beam should be
//         highlighted somehow, and it should be disabled: not able to fire
//         phasers, establish new tractor beams, or move.
//      -- Add a "Springy" button that changes whether tractor beams cause
//         attached ships to stay fixed relative to each other.
//         -- Springy behavior is currently exhibited by the demo
//         -- Non-springy behavior:
//            -- When a TB is attached to a TB_DEST, the TB_DEST's left and top
//               should have a dependency on the left and top of the TB_SOURCE,
//               so that the TB_DEST moves as the TB_SOURCE moves.
//            -- The TB_DEST should not be movable
//            -- When the TB is destroyed, remove the constraints from TB_DEST

#include <amulet.h>

#include <time.h>   //time, for random numbers
#include <stdlib.h> //srand, rand

#ifndef RAND_MAX
#define RAND_MAX 0x7FFFFFFF
#endif

/* ********************************************* */
/*   Parameters for initial appearance of demo   */
/*                                               */
int outer_left_init = 50;
int outer_top_init = 50;
int outer_width_init = 600;
int outer_height_init = 400;

int scrollable_width_init = 1200;
int scrollable_height_init = 800;
/*                                               */
/* ********************************************* */

#define IMFN_FEDERATION "images/ent.gif"
#define IMFN_KLINGON "images/kling.gif"
// these were in place for the sun
// #define IMFN_FEDERATION "images/ent.xbm"
// #define IMFN_KLINGON "images/kling.xbm"

Am_Object SP_Outer_Win;  // The top-level window
Am_Object SP_Clip_Win;   // Clipping window for Short-Range scan,
                         //   large subwindow of SP_Outer_Win
Am_Object SP_Scroll_Agg, // Main group for Short-Range scan, contains
                         //   ship and beam aggs, clipped by SP_Clip_Win
    SP_Star_Agg,         // Group for stars, part of SP_Scroll_Agg
    SP_Long_Win;         // Long-Range scan, small subwin of SP_Outer_Win

Am_Object SP_Ship_Agg, // Aggregate for holding ships
    SP_Beam_Agg,       // Aggregate for holding phasers & tractor beams
                       //   drawn between ships
    SP_Ship,           // Prototype for Federation and Klingon ships
                       //   each ship has a TB_LIST of tractor beams
    SP_Star_Proto,     // Prototype for stars
    SP_Klingon_Phaser_Beam,
    SP_Federation_Phaser_Beam, // Feedback for phasers
    SP_Tractor_Beam,           // Prototype for tractor beams
                               //   each tractor beam has a TB_SOURCE and a
                               //   TB_DEST indicating connected ships
    SP_Long_Feed,              // The white rectangle in the long-range window
                               //   that controls the short-range scroll
    SP_Long_Map,               // A small version of what is shown in the
                               //   short-range scan
    SP_Long_Range_Ship,        // Item prototype for SP_Long_Map
    SP_Springy_Button,         // Makes tractor beams rigid or springy
    SP_Button_Panel,           // all buttons
    SP_Springy_Command;        // command object that shows if springy

Am_Object SP_Short_Label, // Decoration
    SP_Long_Label;        //

Am_Object SP_Ship_Creator, // Makes Federation and Klingon ships
    SP_Ship_Mover,         // Moves Federation and Klingon ships
    SP_Ship_Editor,        // Edits labels of Federation and Klingon ships
    SP_Phaser_Inter,       // Draws phasers from all kinds of ships
    SP_Tractor_Inter,      // Draws tractor beams from all kinds of ships
    SP_Long_Inter,         // Moves the white rectangle and scrolls short win
    SP_Button_Simulator_Inter; // TEMP, until buttons are working

//  Prototype for parts of SP_Ship
Am_Image_Array SP_Federation_Image;
Am_Image_Array SP_Klingon_Image;
Am_Font SP_Short_Font(Am_FONT_SERIF, false, true, false, Am_FONT_VERY_LARGE);
Am_Font SP_Long_Font(Am_FONT_SERIF, false, true, false, Am_FONT_LARGE);
Am_Style SP_Federation_Blue(0.0f, 0.75f, 1.0f);

Am_Style SP_Federation_Phaser_Style, SP_Klingon_Phaser_Style,
    SP_Tractor_Beam_Style, SP_Background_Style;

int SP_Margin = 5;       // The offset between windows and gadgets
int SP_Label_Offset = 2; // The offset between labels and objects

typedef int SP_Ship_Type;
#define FEDERATION 1
#define KLINGON 2

Am_Slot_Key SHIP_TYPE = Am_Register_Slot_Name("SHIP_TYPE");
Am_Slot_Key SHIP_BITMAP = Am_Register_Slot_Name("SHIP_BITMAP");
Am_Slot_Key SHIP_BBOX = Am_Register_Slot_Name("SHIP_BBOX");
Am_Slot_Key SHIP_LABEL = Am_Register_Slot_Name("SHIP_LABEL");
// Am_Slot_Key SOURCE_SHIP = Am_Register_Slot_Name ("SOURCE_SHIP");
Am_Slot_Key PHASER_BEAM = Am_Register_Slot_Name("PHASER_BEAM");
Am_Slot_Key TB_SOURCE = Am_Register_Slot_Name("TB_SOURCE");
Am_Slot_Key TB_DEST = Am_Register_Slot_Name("TB_DEST");
Am_Slot_Key TB_LIST = Am_Register_Slot_Name("TB_LIST");

Am_Slot_Key SHIP_CREATOR = Am_Register_Slot_Name("SHIP_CREATOR");
Am_Slot_Key SHIP_MOVER = Am_Register_Slot_Name("SHIP_MOVER");
Am_Slot_Key SHIP_EDITOR = Am_Register_Slot_Name("SHIP_EDITOR");
Am_Slot_Key PHASER_INTER = Am_Register_Slot_Name("PHASER_INTER");
Am_Slot_Key TRACTOR_INTER = Am_Register_Slot_Name("TRACTOR_INTER");
Am_Slot_Key LONG_INTER = Am_Register_Slot_Name("LONG_INTER");
Am_Slot_Key IS_SPRINGY = Am_Register_Slot_Name("IS_SPRINGY");

/* ******************************* */
/*  Window and Agg Initialization  */
/* ******************************* */

Am_Define_Formula(const char *, springy_label)
{
  if ((bool)self.Get(IS_SPRINGY))
    return " Rigid Tractors ";
  else
    return "Springy Tractors";
}

Am_Define_No_Self_Formula(int, clip_win_top)
{
  return ((int)(SP_Short_Label.Get(Am_TOP)) +
          (int)(SP_Short_Label.Get(Am_HEIGHT)) + SP_Label_Offset + 2);
}

Am_Define_No_Self_Formula(int, clip_win_width)
{
  int outer_width = SP_Outer_Win.Get(Am_WIDTH);
  return ((int)(.75 * (outer_width - SP_Margin - SP_Margin - SP_Margin)));
}

Am_Define_Formula(int, clip_win_height)
{
  int outer_height = SP_Outer_Win.Get(Am_HEIGHT);
  int my_top = self.Get(Am_TOP);
  return (outer_height - my_top - SP_Margin);
}

Am_Define_No_Self_Formula(int, clip_win_bottom)
{
  return (int)SP_Clip_Win.Get(Am_TOP) + (int)SP_Clip_Win.Get(Am_HEIGHT);
}

Am_Define_No_Self_Formula(int, long_win_left)
{
  return ((int)SP_Clip_Win.Get(Am_LEFT) + (int)SP_Clip_Win.Get(Am_WIDTH) +
          SP_Margin);
}

Am_Define_Formula(int, long_win_top)
{
  int outer_height = SP_Outer_Win.Get(Am_HEIGHT);
  int long_height = self.Get(Am_HEIGHT);
  return (outer_height - long_height - SP_Margin);
}

Am_Define_Formula(int, long_win_width)
{
  int outer_width = SP_Outer_Win.Get(Am_WIDTH);
  return outer_width - (int)self.Get(Am_LEFT) - SP_Margin;
}

Am_Define_No_Self_Formula(int, long_win_height)
{
  int outer_height = SP_Outer_Win.Get(Am_HEIGHT);
  return ((int)(.25 * (outer_height - SP_Margin - SP_Margin)));
}

Am_Define_Formula(int, right_margin_window)
{
  Am_Object window = (Am_Object)(self.Get(Am_WINDOW));
  if (window)
    return (int)window.Get(Am_WIDTH) - (int)self.Get(Am_WIDTH) - 5;
  else
    return 470;
}

/* ******************* */
/*    Ship Creators    */
/* ******************* */

Am_Define_Style_Formula(ship_color)
{
  if (Am_Screen.Valid())
    if (!(bool)Am_Screen.Get(Am_IS_COLOR))
      return Am_White;
  SP_Ship_Type st = (int)(self.Get_Owner().Get(SHIP_TYPE));
  if (st == FEDERATION)
    return (SP_Federation_Blue);
  else if (st == KLINGON)
    return (Am_Green);
  else {
    //Am_Error("ship_color: Invalid ship type.\n");
    return NULL;
  }
}

Am_Define_Image_Formula(ship_image)
{
  SP_Ship_Type st = (int)(self.Get_Owner().Get(SHIP_TYPE));
  if (st == FEDERATION)
    return (SP_Federation_Image);
  else if (st == KLINGON)
    return (SP_Klingon_Image);
  else {
    //Am_Error("ship_image: Invalid ship type.\n");
    return NULL;
  }
}

Am_Define_Formula(int, ship_bbox_width)
{
  return (self.Get_Owner().Get(Am_WIDTH));
}

Am_Define_Formula(int, ship_bbox_height)
{
  return (self.Get_Owner().Get(Am_HEIGHT));
}

Am_Define_Formula(int, ship_label_left)
{
  Am_Object ship = self.Get_Owner();
  if (!ship.Valid())
    return 0;

  Am_Object ship_bitmap = ship.Get_Object(SHIP_BITMAP);
  int ship_left = ship.Get(Am_LEFT);
  int bitmap_left = ship_bitmap.Get(Am_LEFT);
  int bitmap_width = ship_bitmap.Get(Am_WIDTH);
  int my_width = self.Get(Am_WIDTH);

  if (my_width <= bitmap_width) {
    if (bitmap_left) {
      ship.Set(Am_LEFT, ship_left + bitmap_left);
      ship_bitmap.Set(Am_LEFT, 0);
    }
    return ((bitmap_width - my_width) / 2);
  } else {
    int new_ship_left = ship_left + bitmap_left + (bitmap_width - my_width) / 2;
    if (new_ship_left != ship_left) {
      ship.Set(Am_LEFT, new_ship_left);
      ship_bitmap.Set(Am_LEFT, bitmap_left + ship_left - new_ship_left);
    }
  }
  return 0;
}

Am_Define_Formula(int, ship_label_top)
{
  Am_Object bottom_obj = self.Get_Sibling(SHIP_BITMAP);
  int bottom = (int)(bottom_obj.Get(Am_TOP)) + (int)(bottom_obj.Get(Am_HEIGHT));
  return (bottom + SP_Label_Offset);
}

void
init_styles()
{

  Am_Image_Array SP_Phaser_Stipple(25);

  if ((bool)Am_Screen.Get(Am_IS_COLOR)) {
    SP_Background_Style = Am_Motif_Gray;
    SP_Tractor_Beam_Style = Am_Style(1.0f, 0.0f, 0.0f, 4);
    SP_Federation_Phaser_Style =
        Am_Style(0.6f, 0.6f, 1.0f, 4, Am_CAP_BUTT, Am_JOIN_MITER, Am_LINE_SOLID,
                 Am_DEFAULT_DASH_LIST, Am_DEFAULT_DASH_LIST_LENGTH,
                 Am_FILL_STIPPLED, Am_FILL_POLY_EVEN_ODD, SP_Phaser_Stipple);
    SP_Klingon_Phaser_Style =
        Am_Style(0.0f, 1.0f, 0.0f, 4, Am_CAP_BUTT, Am_JOIN_MITER, Am_LINE_SOLID,
                 Am_DEFAULT_DASH_LIST, Am_DEFAULT_DASH_LIST_LENGTH,
                 Am_FILL_STIPPLED, Am_FILL_POLY_EVEN_ODD, SP_Phaser_Stipple);
  } else {
    SP_Background_Style = Am_White;
    SP_Tractor_Beam_Style =
        Am_Style(1.0f, 1.0f, 1.0f, 4, Am_CAP_BUTT, Am_JOIN_MITER, Am_LINE_SOLID,
                 Am_DEFAULT_DASH_LIST, Am_DEFAULT_DASH_LIST_LENGTH,
                 Am_FILL_STIPPLED, Am_FILL_POLY_EVEN_ODD, Am_Image_Array(75));
    SP_Federation_Phaser_Style =
        Am_Style(1.0f, 1.0f, 1.0f, 4, Am_CAP_BUTT, Am_JOIN_MITER, Am_LINE_SOLID,
                 Am_DEFAULT_DASH_LIST, Am_DEFAULT_DASH_LIST_LENGTH,
                 Am_FILL_STIPPLED, Am_FILL_POLY_EVEN_ODD, SP_Phaser_Stipple);
    SP_Klingon_Phaser_Style =
        Am_Style(1.0f, 1.0f, 1.0f, 4, Am_CAP_BUTT, Am_JOIN_MITER, Am_LINE_SOLID,
                 Am_DEFAULT_DASH_LIST, Am_DEFAULT_DASH_LIST_LENGTH,
                 Am_FILL_STIPPLED, Am_FILL_POLY_EVEN_ODD, Am_Image_Array(50));
  }
}

void
init_protos()
{

  const char *federation_image_file = Am_Merge_Pathname(IMFN_FEDERATION);
  SP_Federation_Image = Am_Image_Array(federation_image_file);
  delete[] federation_image_file;
  if (!SP_Federation_Image.Valid())
    Am_Error("Federation image not found");

  const char *klingon_image_file = Am_Merge_Pathname(IMFN_KLINGON);
  SP_Klingon_Image = Am_Image_Array(klingon_image_file);
  delete[] klingon_image_file;
  if (!SP_Klingon_Image.Valid())
    Am_Error("Klingon image not found");

  SP_Ship = Am_Group.Create("SP_Ship")
                .Add(SHIP_TYPE, KLINGON) // Set in instances of SP_Ship
                .Set(Am_WIDTH, Am_Width_Of_Parts)
                .Set(Am_HEIGHT, Am_Height_Of_Parts)
                .Add_Part(SHIP_BITMAP, Am_Bitmap.Create("SP_Ship_Bitmap")
                                           .Set(Am_LEFT, 0)
                                           .Set(Am_TOP, 0)
                                           .Set(Am_IMAGE, (ship_image))
                                           .Set(Am_LINE_STYLE, (ship_color))
                                           .Set(Am_DRAW_MONOCHROME, true))
                .Add_Part(SHIP_LABEL, Am_Text.Create("SP_Ship_Label")
                                          .Set(Am_LEFT, (ship_label_left))
                                          .Set(Am_TOP, (ship_label_top))
                                          .Set(Am_TEXT, "Label")
                                          .Set(Am_LINE_STYLE, Am_White));

  SP_Tractor_Beam = Am_Line.Create("SP_Tractor_Beam")
                        .Set(Am_VISIBLE, false) // Set by interactor
                        .Set(Am_LINE_STYLE, SP_Tractor_Beam_Style);
  SP_Clip_Win.Add_Part(SP_Tractor_Beam);
}

Am_Define_Method(Am_Object_Method, void, create_ship, (Am_Object command_obj))
{
  // Incoming coordinates are w.r.t. clip-win.  Translate them to short-agg.
  int clip_x = SP_Ship_Creator.Get(Am_FIRST_X);
  int clip_y = SP_Ship_Creator.Get(Am_FIRST_Y);
  int x, y;
  Am_Translate_Coordinates(SP_Clip_Win, clip_x, clip_y, SP_Ship_Agg, x, y);
  // SP_Trans_Clip_Ship_Coords (clip_x, clip_y, x, y);
  SP_Ship_Type ship_type;

  // Decide type of ship by whether event was in upper half or lower half
  // of clip window
  int l = SP_Clip_Win.Get(Am_HEIGHT);
  const char *ship_name;
  if (clip_y < (l / 2)) {
    ship_type = FEDERATION;
    ship_name = "Federation";
  } else {
    ship_type = KLINGON;
    ship_name = "Klingon";
  }

  Am_Object new_ship = SP_Ship.Create()
                           .Set(SHIP_TYPE, ship_type)
                           .Set(Am_LEFT, x - 10)
                           .Set(Am_TOP, y - 10)
                           .Add(TB_LIST, Am_Value_List());
  SP_Ship_Agg.Add_Part(new_ship);

  // Don't write a formula for SHIP_LABEL because it is set by
  // the text interactor.
  new_ship.Get_Object(SHIP_LABEL).Set(Am_TEXT, ship_name);

  // now set up for Undo
  command_obj.Set(Am_VALUE, new_ship);
  //  Am_Call(Am_Object_Proc, Am_Command, Am_DO_METHOD, (command_obj));
}

/* ************************* */
/*   Phaser Beam Creators    */
/* ************************* */

void
remove_tractor_beam_from_ship(Am_Object beam, Am_Object ship)
{
  Am_Value_List beams;
  beams = ship.Get(TB_LIST);
  beams.Start();
  if (beams.Member(beam))
    beams.Delete();
  ship.Set(TB_LIST, beams);
}

void
remove_tractor_beam_from_both_ships(Am_Object beam)
{
  Am_Object ship = (Am_Object)(beam.Get(TB_SOURCE));
  if (ship.Valid())
    remove_tractor_beam_from_ship(beam, ship);
  ship = beam.Get(TB_DEST);
  if (ship.Valid())
    remove_tractor_beam_from_ship(beam, ship);
}

void
add_tractor_beam_to_ship(Am_Object beam, Am_Object ship)
{
  Am_Value_List beams;
  beams = ship.Get(TB_LIST);
  beams.Add(beam);
  ship.Set(TB_LIST, beams);
}

void
add_tractor_beam_to_both_ships(Am_Object beam)
{
  Am_Object ship = (Am_Object)(beam.Get(TB_SOURCE));
  if (ship.Valid())
    add_tractor_beam_to_ship(beam, ship);
  ship = beam.Get(TB_DEST);
  if (ship.Valid())
    add_tractor_beam_to_ship(beam, ship);
}

void
destroy_tractor_beams(Am_Object ship_obj)
{
  Am_Object beam, other_ship;
  Am_Value_List beams;
  beams = ship_obj.Get(TB_LIST);
  for (beams.Start(); !beams.Last(); beams.Next()) {
    beam = beams.Get();

    // Remove tractor beam from the value list of its other attached ship
    other_ship = beam.Get(TB_SOURCE);
    if (other_ship == ship_obj)
      // Oops, ship_obj was the source, we meant to get TB_DEST
      other_ship = beam.Get(TB_DEST);
    remove_tractor_beam_from_ship(beam, other_ship);

    beams.Delete(); // remove the current beam from my list
    beam.Destroy(); // destroy the actual beam object
  }
  ship_obj.Set(TB_LIST, beams); //beams should now be empty
}

void
hide_tractor_beams(Am_Object ship_obj, bool become_visible)
{
  Am_Object beam, other_ship;
  Am_Value_List beams, other_beams;
  beams = ship_obj.Get(TB_LIST);
  for (beams.Start(); !beams.Last(); beams.Next()) {
    beam = beams.Get();

    // Remove tractor beam from the value list of its other attached ship
    other_ship = beam.Get(TB_SOURCE);
    if (other_ship == ship_obj)
      // Oops, ship_obj was the source, we meant to get TB_DEST
      other_ship = beam.Get(TB_DEST);
    other_beams = other_ship.Get(TB_LIST);
    if (become_visible) { // then add it to other ships
      other_beams.Add(beam);
    } else { // remove it from other ship's list
      other_beams.Start();
      if (other_beams.Member(beam)) //sets the pointer to beam
        other_beams.Delete();
    }
    beam.Set(Am_VISIBLE, become_visible);
    other_ship.Set(TB_LIST, other_beams);
  }
}

void
hide_ship(Am_Object ship_obj)
{
  bool new_vis = !(bool)ship_obj.Get(Am_VISIBLE);

  ship_obj.Set(Am_VISIBLE, new_vis);
  hide_tractor_beams(ship_obj, new_vis);
  if (new_vis)
    SP_Ship_Agg.Add_Part(ship_obj);
  else
    SP_Ship_Agg.Remove_Part(ship_obj);
}

// undo for create_ship, and phaser destroyer
Am_Define_Method(Am_Object_Method, void, hide_ship_command,
                 (Am_Object command_obj))
{
  Am_Object new_object = command_obj.Get(Am_VALUE);
  if (new_object.Valid())
    hide_ship(new_object);
}

// actually just hides it so it can be undone.
Am_Define_Method(Am_Current_Location_Method, void, destroy_ship,
                 (Am_Object command_obj, Am_Object /* obj_modified */,
                  Am_Inter_Location data))
{
  // We use the second endpoint of the phaser beam to determine the location
  // of the ship to destroy.
  Am_Object ref_obj;
  int x1, y1, x2, y2;
  bool flag;
  // the phaser is from (x1, y1) to (x2, y2) wrt ref_obj.
  data.Get_Location(flag, ref_obj, x1, y1, x2, y2);
  if (ref_obj != SP_Ship_Agg)
    Am_Translate_Coordinates(ref_obj, x2, y2, SP_Ship_Agg, x2, y2);

  Am_Object obj_over = Am_Point_In_Part(SP_Ship_Agg, x2, y2, SP_Ship_Agg);
  if (obj_over.Valid()) {
    // Don't allow a ship to destroy itself.
    // get object the interactor started over, which will be the source ship
    Am_Object inter = command_obj.Get_Owner();
    Am_Object source_ship = inter.Get(Am_START_OBJECT);
    if (obj_over != source_ship) {
      // destroy_tractor_beams (obj_over);
      // obj_over.Destroy();
      hide_ship(obj_over);
      command_obj.Set(Am_VALUE, obj_over);
      return; //everything is fine
    }
  }
  // if get here, then nothing destroyed
  Am_Abort_Widget(command_obj); //make this not be queued for undo
}

void
install_ship_inters()
{
  SP_Ship_Creator = Am_One_Shot_Interactor.Create("SP_Ship_Creator")
                        .Set(Am_START_WHEN, "LEFT_DOWN")
                        .Set(Am_CONTINUOUS, 0)
                        .Get_Object(Am_COMMAND)
                        .Set(Am_DO_METHOD, create_ship)
                        .Set(Am_UNDO_METHOD, hide_ship_command)
                        .Set(Am_REDO_METHOD, hide_ship_command)
                        .Get_Owner(); // Return inter to bind SP_Ship_Creator
  // SP_Ship_Creator is attached to the window, so has higher priority
  SP_Clip_Win.Add_Part(SHIP_CREATOR, SP_Ship_Creator);

  SP_Ship_Mover = Am_Move_Grow_Interactor.Create("SP_Ship_Mover")
                      .Set(Am_START_WHEN, "LEFT_DOWN")
                      .Set(Am_START_WHERE_TEST, Am_Inter_In_Part);
  //** used to be SP_In_Ship;

  // SP_Ship_Creator is attached to the aggregate, so has lower priority
  SP_Ship_Agg.Add_Part(SHIP_MOVER, SP_Ship_Mover);

  SP_Ship_Editor =
      Am_Text_Edit_Interactor.Create("SP_Ship_Editor")
          .Set(Am_START_WHERE_TEST, Am_Inter_In_Text_Leaf)
          .Set(Am_PRIORITY, 3.0) // higher priority than move or create
      ;
  SP_Ship_Agg.Add_Part(SHIP_EDITOR, SP_Ship_Editor);
  //** used to be SP_In_Ship;
}

Am_Define_Object_Formula(pick_phaser_beam)
{
  Am_Object ship;
  // base the type of phaser on the type of ship that clicked on first
  ship = self.Get(Am_START_OBJECT);
  if (ship.Valid() && (int)ship.Get(SHIP_TYPE) == FEDERATION)
    return SP_Federation_Phaser_Beam;
  else
    return SP_Klingon_Phaser_Beam;
}

void
install_phaser_inter()
{
  // Add the phaser beams to the SP_Clip_Win so that it is not iterated
  // over by do_springy when the tractor beam endpoints are adjusted, and
  //
  SP_Federation_Phaser_Beam =
      Am_Line.Create("SP_Phaser_Beam")
          .Set(Am_VISIBLE, false) // Set by interactor
          .Set(Am_LINE_STYLE, SP_Federation_Phaser_Style);
  SP_Klingon_Phaser_Beam = Am_Line.Create("SP_Phaser_Beam")
                               .Set(Am_VISIBLE, false) // Set by interactor
                               .Set(Am_LINE_STYLE, SP_Klingon_Phaser_Style);
  SP_Phaser_Inter = Am_New_Points_Interactor
                        .Create("SP_Phaser_Inter")
#if defined(_WINDOWS)
                        .Set(Am_START_WHEN, "META_LEFT_DOWN")
#else
                        .Set(Am_START_WHEN, "MIDDLE_DOWN")
#endif
                        .Set(Am_START_WHERE_TEST, Am_Inter_In_Part)
                        .Set(Am_FEEDBACK_OBJECT, (pick_phaser_beam))

                        .Set(Am_AS_LINE, true)
                        .Set(Am_CREATE_NEW_OBJECT_METHOD,
                             0L) //not really creating something
                        .Get_Object(Am_COMMAND)
                        .Set(Am_DO_METHOD, destroy_ship)
                        .Set(Am_UNDO_METHOD, hide_ship_command)
                        .Set(Am_REDO_METHOD, hide_ship_command)
                        .Get_Owner(); // Return inter to bind SP_Phaser_Inter

  SP_Clip_Win.Add_Part(SP_Federation_Phaser_Beam)
      .Add_Part(SP_Klingon_Phaser_Beam);
  SP_Ship_Agg.Add_Part(PHASER_INTER, SP_Phaser_Inter);
}

/* ************************** */
/*   Tractor Beam Creators    */
/* ************************** */

// The formulas tractor_beam_x1 and tractor_beam_x2 rely on the fact
// that SP_Ship_Agg and SP_Beam_Agg overlap precisely, with the same
// left and top.  You cannot call Am_Translate_Coordinates in these
// formulas because the ships that the tractor beams are connected to
// are removed from the window before the beams, and the translation
// fails, setting X1-Y2 with bogus values.  These bogus values cause
// bad positioning of the tractor beams when the delete is undone.

// calculate and return x1 and explicitly set y1 so don't need 2 formulas
Am_Define_Formula(int, tractor_beam_x1)
{
  Am_Object tb_source = (Am_Object)(self.Get(TB_SOURCE));
  Am_Object tb_source_bitmap = (Am_Object)(tb_source.Get(SHIP_BITMAP));
  int tb_source_x =
      ((int)(tb_source.Get(Am_LEFT)) + (int)(tb_source_bitmap.Get(Am_LEFT)) +
       ((int)(tb_source_bitmap.Get(Am_WIDTH)) / 2));
  int tb_source_y = ((int)(tb_source.Get(Am_TOP)) +
                     ((int)(tb_source_bitmap.Get(Am_HEIGHT)) / 2));
  self.Set(Am_Y1, tb_source_y);
  return (tb_source_x);
}

// calculate and return x2 and explicitly set y2 so don't need 2 formulas
Am_Define_Formula(int, tractor_beam_x2)
{
  Am_Object tb_dest = (Am_Object)(self.Get(TB_DEST));
  Am_Object tb_dest_bitmap = (Am_Object)(tb_dest.Get(SHIP_BITMAP));
  int tb_dest_x =
      ((int)(tb_dest.Get(Am_LEFT)) + (int)(tb_dest_bitmap.Get(Am_LEFT)) +
       ((int)(tb_dest_bitmap.Get(Am_WIDTH)) / 2));
  int tb_dest_y =
      ((int)(tb_dest.Get(Am_TOP)) + ((int)(tb_dest_bitmap.Get(Am_HEIGHT)) / 2));
  self.Set(Am_Y2, tb_dest_y);
  //self.Set(Am_X2, tb_dest_x);//** NEEDED FOR NO APPARENT REASON BY PC VERSION
  return (tb_dest_x);
}

// The interactor this is called from is attached to the SP_Ship_Agg
// so the coordinates are w.r.t that group.
Am_Define_Method(Am_Create_New_Object_Method, Am_Object, create_tractor_beam,
                 (Am_Object inter, Am_Inter_Location data,
                  Am_Object /* old_object */))
{
  // We use the second endpoint of the phaser beam to determine the location
  // of the ship to destroy.
  Am_Object ref_obj;
  int x1, y1, x2, y2;
  bool flag;
  // the phaser is from (x1, y1) to (x2, y2) wrt ref_obj.
  data.Get_Location(flag, ref_obj, x1, y1, x2, y2);
  if (ref_obj != SP_Ship_Agg)
    Am_Translate_Coordinates(ref_obj, x2, y2, SP_Ship_Agg, x2, y2);

  Am_Object dest_ship = Am_Point_In_Part(SP_Ship_Agg, x2, y2, SP_Ship_Agg);
  if (dest_ship.Valid()) {
    // get object the interactor started over, which will be the source ship
    Am_Object source_ship = (Am_Object)inter.Get(Am_START_OBJECT);
    if (dest_ship != source_ship) {
      // Don't allow a tractor beam from a ship to itself
      Am_Object new_tractor_beam = SP_Tractor_Beam.Create()
                                       .Add(TB_SOURCE, source_ship)
                                       .Add(TB_DEST, dest_ship)
                                       .Set(Am_X1, (tractor_beam_x1))
                                       //	.Set (Am_Y1, (tractor_beam_y1))
                                       .Set(Am_X2, (tractor_beam_x2))
                                       //	.Set (Am_Y2, (tractor_beam_y2))
                                       .Set(Am_VISIBLE, true);
      SP_Beam_Agg.Add_Part(new_tractor_beam);

      add_tractor_beam_to_ship(new_tractor_beam, source_ship);
      add_tractor_beam_to_ship(new_tractor_beam, dest_ship);

      return new_tractor_beam;
    }
  }
  //if get here, then not going to create a beam
  Am_Abort_Interactor(inter); //make this not be queued for undo
  return (NULL);
}

// undo for create_tractor_beam
Am_Define_Method(Am_Object_Method, void, hide_tractor_beam_command,
                 (Am_Object command_obj))
{
  Am_Object beam = (Am_Object)(command_obj.Get(Am_VALUE));
  if (beam.Valid()) {
    bool vis = beam.Get(Am_VISIBLE);
    beam.Set(Am_VISIBLE, !vis);
    if (!vis)
      add_tractor_beam_to_both_ships(beam);
    else
      remove_tractor_beam_from_both_ships(beam);
    command_obj.Set(Am_HAS_BEEN_UNDONE, vis); // opposite of visible
  }
}

void
install_tractor_inter()
{

  SP_Tractor_Inter = Am_New_Points_Interactor.Create("SP_Tractor_Inter")
                         .Set(Am_START_WHEN, "RIGHT_DOWN")
                         .Set(Am_START_WHERE_TEST, Am_Inter_In_Part)
                         // *** used to be SP_In_Ship
                         .Set(Am_AS_LINE, true)
                         .Set(Am_FEEDBACK_OBJECT, SP_Tractor_Beam)
                         .Set(Am_CREATE_NEW_OBJECT_METHOD, create_tractor_beam)
                         .Get_Object(Am_COMMAND)
                         .Set(Am_UNDO_METHOD, hide_tractor_beam_command)
                         .Set(Am_REDO_METHOD, hide_tractor_beam_command)
                         .Get_Owner(); // Return inter to bind SP_Tractor_Inter
  SP_Ship_Agg.Add_Part(TRACTOR_INTER, SP_Tractor_Inter);
}

/* ******************* */
/*   Long-Range Scan   */
/* ******************* */

// Notes for formulas and functions that scale movement in the short-range
// display to the long-range display:
//
// do_scroll_short_agg, long_range_ship_left, long_range_ship_top:
//
//   The long-range window contains a miniature display of the objects
//   contained in the short-range group.  The scaling factor for reflecting
//   changes between these two containers is the ratio of size of the
//   containers.
//
//
// long_feed_width, long_feed_height:
//
//   To calculate the dimensions of the white feedback rectangle SP_Long_Feed,
//   you have to additionally consider the ratio of the size of the clip win
//   to the size of the short-range group.
//
//    clip      feed                         clip * long
//    -----  =  ----      ---->     feed  =  -----------
//    short     long                            short
//

// Map from the range [0, max_long_x/y] to [0, max_short_x/y].
// That is, as the left of the long-range feedback changes, map that value
// into the range of values selectable by the scrolling window.
// This is very similar to finding the ratio of sizes of the containers.
float
get_long_short_ratio_x()
{
  int max_long_x =
      (int)(SP_Long_Win.Get(Am_WIDTH)) - (int)(SP_Long_Feed.Get(Am_WIDTH));
  int max_short_x = SP_Scroll_Agg.Get_Object(Am_H_SCROLLER).Get(Am_VALUE_2);
  return ((float)max_long_x / max_short_x);
}

float
get_long_short_ratio_y()
{
  int max_long_y =
      (int)(SP_Long_Win.Get(Am_HEIGHT)) - (int)(SP_Long_Feed.Get(Am_HEIGHT));
  int max_short_y = SP_Scroll_Agg.Get_Object(Am_V_SCROLLER).Get(Am_VALUE_2);
  return ((float)max_long_y / max_short_y);
}

// Use a custom gridding procedure to keep long range scan box inside window
Am_Define_Method(Am_Custom_Gridding_Method, void, keep_inside_window,
                 (Am_Object /*inter*/, const Am_Object & /* */, int x, int y,
                  int &out_x, int &out_y))
{
  int height = SP_Long_Feed.Get(Am_HEIGHT);
  int width = SP_Long_Feed.Get(Am_WIDTH);
  int lrs_height = SP_Long_Win.Get(Am_HEIGHT);
  int lrs_width = SP_Long_Win.Get(Am_WIDTH);
  // If the checks below do not crop the incoming event, then we will return
  // these values for out_x and out_y.
  out_x = x;
  out_y = y;
  // If the incoming event would put the feedback object outside of the
  // long-range window, adjust out_x and out_y to keep it inside.
  if (y < 0)
    out_y = 0;
  else if ((y + height) > lrs_height)
    out_y = lrs_height - height;

  if (x < 0)
    out_x = 0;
  else if ((x + width) > lrs_width)
    out_x = lrs_width - width;
}

// See notes above regarding do_scroll_short_agg
//void scroll_short_agg (Am_Object inter, Am_Object object,
//		       Am_Object event_window,
//		       Am_Input_Event *ev) {

Am_Define_Method(Am_Object_Method, void, do_scroll_short_agg,
                 (Am_Object /* command */))
{
  float ratio_x = get_long_short_ratio_x();
  float ratio_y = get_long_short_ratio_y();

  //constraints set the SP_Short_Agg based on the scroll bars
  SP_Scroll_Agg.Set(Am_X_OFFSET,
                    (int)((int)(SP_Long_Feed.Get(Am_LEFT)) / ratio_x));
  SP_Scroll_Agg.Set(Am_Y_OFFSET,
                    (int)((int)(SP_Long_Feed.Get(Am_TOP)) / ratio_y));
}

// See notes above regarding long_feed_width and long_feed_height
Am_Define_No_Self_Formula(int, long_feed_width)
{
  int clip_win_w = SP_Clip_Win.Get(Am_WIDTH);
  int long_win_w = SP_Long_Win.Get(Am_WIDTH);
  // do calculations in float
  return (
      (int)(clip_win_w * ((float)long_win_w / (float)scrollable_width_init)));
}

Am_Define_No_Self_Formula(int, long_feed_height)
{
  int clip_win_h = SP_Clip_Win.Get(Am_HEIGHT);
  int long_win_h = SP_Long_Win.Get(Am_HEIGHT);
  // do calculations in float
  return (
      (int)(clip_win_h * ((float)long_win_h / (float)scrollable_height_init)));
}

Am_Define_No_Self_Formula(Am_Wrapper *, long_map_items)
{
  Am_Value_List components;
  components = SP_Ship_Agg.Get(Am_GRAPHICAL_PARTS);
  return (components);
}

// See notes above regarding long_range_ship_left and long_range_ship_top
Am_Define_Formula(int, long_range_ship_left)
{
  // Set up dependency on long-range win width so that long-range ships
  // will reposition if the window is resized.
  /*int long_win_width =*/SP_Long_Win.Get(Am_WIDTH);
  Am_Object ship = (Am_Object)(self.Get(Am_ITEM));
  if (((Am_Object)ship.Get(Am_WINDOW)).Valid()) {
    Am_Object bitmap = (Am_Object)(ship.Get(SHIP_BITMAP));
    int ship_center_x = ((int)(ship.Get(Am_LEFT)) + (int)(bitmap.Get(Am_LEFT)) +
                         ((int)(bitmap.Get(Am_WIDTH)) / 2));
    float ratio_x = get_long_short_ratio_x();
    int long_center_x = (int)(ship_center_x * ratio_x);
    int long_width = self.Get(Am_WIDTH);
    return (long_center_x - (long_width / 2));
  } else
    // Ship has no window, so might be item-prototype
    return 0;
}

Am_Define_Formula(int, long_range_ship_top)
{
  // Set up dependency on long-range win height so that long-range ships
  // will reposition if the window is resized.
  /*int long_win_height =*/SP_Long_Win.Get(Am_HEIGHT);
  Am_Object ship = (Am_Object)(self.Get(Am_ITEM));
  if (((Am_Object)ship.Get(Am_WINDOW)).Valid()) {
    int ship_center_y =
        ((int)(ship.Get(Am_TOP)) + ((int)(ship.Get(Am_HEIGHT)) / 2));
    float ratio_y = get_long_short_ratio_y();
    int long_center_y = (int)(ship_center_y * ratio_y);
    int long_height = self.Get(Am_HEIGHT);
    return (long_center_y - (long_height / 2));
  } else
    // Ship has no window, so might be item_prototype
    return 0;
}

Am_Define_Style_Formula(long_range_ship_fill)
{
  Am_Object item, bitmap;
  item = self.Get(Am_ITEM);
  bitmap = item.Get(SHIP_BITMAP);
  return (bitmap.Get(Am_LINE_STYLE));
}

void
install_long_range_feedback()
{
  SP_Long_Feed = Am_Rectangle.Create("SP_Long_Feed")
                     .Set(Am_LEFT, 0)
                     .Set(Am_TOP, 0)
                     .Set(Am_WIDTH, (long_feed_width))
                     .Set(Am_HEIGHT, (long_feed_height))
                     .Set(Am_LINE_STYLE, Am_White);
  SP_Long_Win.Add_Part(SP_Long_Feed);

  SP_Long_Inter = Am_Move_Grow_Interactor.Create("SP_Long_Inter")
                      .Set(Am_START_WHEN, "LEFT_DOWN")
                      .Set(Am_START_WHERE_TEST, Am_Inter_In)
                      .Set(Am_GRID_METHOD, keep_inside_window);
  SP_Long_Feed.Add_Part(LONG_INTER, SP_Long_Inter);
  SP_Long_Inter.Get_Object(Am_COMMAND).Set(Am_DO_METHOD, do_scroll_short_agg);

  SP_Long_Range_Ship = Am_Arc.Create("SP_Long_Range_Ship")
                           .Add(Am_ITEM, SP_Ship) // Set by SP_Long_Map
                           .Set(Am_LEFT, (long_range_ship_left))
                           .Set(Am_TOP, (long_range_ship_top))
                           .Set(Am_WIDTH, 5)
                           .Set(Am_HEIGHT, 5)
                           .Set(Am_LINE_STYLE, Am_No_Style)
                           .Set(Am_FILL_STYLE, (long_range_ship_fill));

  SP_Long_Map = Am_Map.Create("SP_Long_Map")
                    .Set(Am_LEFT, 0)
                    .Set(Am_TOP, 0)
                    .Set(Am_LEFT_OFFSET, SP_Margin)
                    .Set(Am_TOP_OFFSET, SP_Margin)
                    .Set(Am_WIDTH, Am_Fill_To_Right)
                    .Set(Am_HEIGHT, Am_Fill_To_Bottom)
                    //    .Set (Am_WIDTH, (long_win_width))
                    //    .Set (Am_HEIGHT, (long_win_height))
                    .Set(Am_ITEMS, (long_map_items))
                    .Set_Part(Am_ITEM_PROTOTYPE, SP_Long_Range_Ship);

  SP_Long_Win.Add_Part(SP_Long_Map);
}

/* ***************************************** */
/*    Make Tractor Beams Springy or Rigid    */
/* ***************************************** */

void make_beam_springy(Am_Object /* beam */)
{
  //  cout << "SPRINGY beam == " << beam << endl;
}

void make_beam_rigid(Am_Object /* beam */)
{
  //  cout << "RIGID beam == " << beam << endl;
}

Am_Define_Method(Am_Object_Method, void, do_springy, (Am_Object))
{
  bool is_springy = SP_Springy_Command.Get(IS_SPRINGY);
  is_springy = !is_springy;
  SP_Springy_Command.Set(IS_SPRINGY, is_springy);

  Am_Value_List beams;
  Am_Object beam;
  beams = SP_Beam_Agg.Get(Am_GRAPHICAL_PARTS);
  for (beams.Start(); !beams.Last(); beams.Next()) {
    beam = beams.Get();
    if (is_springy)
      make_beam_springy(beam);
    else
      make_beam_rigid(beam);
  }
}

Am_Define_Method(Am_Object_Method, void, do_exit, (Am_Object /* cmd */))
{
  Am_Exit_Main_Event_Loop();
}

Am_Define_Method(Am_Object_Method, void, h_scroll_do, (Am_Object cmd))
{
  //first, call the standard method
  Am_Object_Method proc;
  proc = Am_Scrolling_Group.Get_Object(Am_H_SCROLLER)
             .Get_Object(Am_COMMAND)
             .Get(Am_DO_METHOD);
  proc.Call(cmd);
  //update the long range scroller
  int val = cmd.Get(Am_VALUE);
  float ratio_x = get_long_short_ratio_x();
  SP_Long_Feed.Set(Am_LEFT, (int)(val * ratio_x));
}
Am_Define_Method(Am_Object_Method, void, v_scroll_do, (Am_Object cmd))
{
  //first, call the standard method
  Am_Object_Method proc;
  proc = Am_Scrolling_Group.Get_Object(Am_V_SCROLLER)
             .Get_Object(Am_COMMAND)
             .Get(Am_DO_METHOD);
  proc.Call(cmd);
  //update the long range scroller
  int val = cmd.Get(Am_VALUE);
  float ratio_y = get_long_short_ratio_y();
  SP_Long_Feed.Set(Am_TOP, (int)(val * ratio_y));
}

/* ******************* */
/*       Stars         */
/* ******************* */

#define NUM_STARS 60

void
install_stars()
{

  static int star_ar[12] = {100, 0, 159, 181, 5, 69, 195, 69, 41, 181, 100, 0};
  Am_Point_List star_pl(star_ar, 12);
  int left, top, width, height;

  Am_Object star_proto =
      Am_Polygon.Create("SP_Star_Proto").Set(Am_POINT_LIST, star_pl);

  srand((unsigned)time(NULL)); //init random generator
  Am_Style color;

  for (int i = 0; i < NUM_STARS; i++) {
    float size_r = ((float)rand()) / RAND_MAX;
    float color_r = ((float)rand()) / RAND_MAX;
    width = (int)(size_r * 20);
    height = (int)(size_r * 20);
    left = (int)((((float)rand()) / RAND_MAX) * scrollable_width_init);
    top = (int)((((float)rand()) / RAND_MAX) * scrollable_height_init);
    if (!(bool)Am_Screen.Get(Am_IS_COLOR))
      color = Am_White;
    else if (color_r < 0.15)
      color = Am_Yellow;
    else if (color_r < 0.3)
      color = Am_Red;
    else if (color_r < 0.45)
      color = Am_Blue;
    else if (color_r < 0.6)
      color = Am_Purple;
    else
      color = Am_White;

    SP_Star_Agg.Add_Part(star_proto.Create()
                             .Set(Am_LINE_STYLE, color)
                             .Set(Am_LEFT, left)
                             .Set(Am_TOP, top)
                             .Set(Am_WIDTH, width)
                             .Set(Am_HEIGHT, height));
  }
}

/* ******************* */
/*        Init_wins    */
/* ******************* */

Am_Define_Formula(int, long_label_top)
{
  return (int)(SP_Long_Win.Get(Am_TOP)) - (int)(self.Get(Am_HEIGHT)) -
         SP_Label_Offset;
}

void
init_wins()
{

  // need to define SP_Short_Label before clip_win_top is used
  SP_Short_Label = Am_Text.Create("SP_Short_Label")
                       .Add(Am_CENTER_X_OBJ, SP_Clip_Win)
                       .Set(Am_LEFT, Am_Center_X_Is_Center_Of)
                       .Set(Am_TOP, SP_Label_Offset)
                       .Set(Am_TEXT, "Short Range Scan")
                       .Set(Am_FONT, SP_Short_Font);

  SP_Outer_Win = Am_Window.Create("SP_Outer_Win")
                     .Set(Am_LEFT, outer_left_init)
                     .Set(Am_TOP, outer_top_init)
                     .Set(Am_WIDTH, outer_width_init)
                     .Set(Am_HEIGHT, outer_height_init)
                     .Set(Am_TITLE, "Amulet Space Demo")
                     .Set(Am_FILL_STYLE, SP_Background_Style)
      //    .Set (Am_DOUBLE_BUFFER, false)
      ;

  SP_Long_Win = Am_Window.Create("SP_Long_Win")
                    .Set(Am_LEFT, (long_win_left))
                    .Set(Am_TOP, (long_win_top))
                    .Set(Am_WIDTH, (long_win_width))
                    .Set(Am_HEIGHT, (long_win_height))
                    .Set(Am_FILL_STYLE, Am_Black)
      //    .Set (Am_DOUBLE_BUFFER, false)
      ;

  SP_Clip_Win =
      Am_Window.Create("SP_Clip_Win")
          .Set(Am_LEFT, SP_Margin)
          .Set(Am_TOP, (clip_win_top))
          .Set(Am_WIDTH, (clip_win_width))
          .Set(Am_HEIGHT, (clip_win_height))
          .Set(Am_FILL_STYLE, SP_Background_Style)
          .Set(Am_UNDO_HANDLER,
               Am_Multiple_Undo_Object.Create("Undo From Outer Space"))
      //    .Set (Am_DOUBLE_BUFFER, false)
      ;

  SP_Scroll_Agg =
      Am_Scrolling_Group.Create("scroll_agg")
          .Set(Am_LEFT, 0)
          .Set(Am_TOP, 0)
          .Set(Am_HEIGHT, Am_From_Owner(Am_HEIGHT))
          .Set(Am_WIDTH, Am_From_Owner(Am_WIDTH))
          .Set(Am_INNER_WIDTH, scrollable_width_init)
          .Set(Am_FILL_STYLE, SP_Background_Style) //color of scroll bars
          .Set(Am_INNER_FILL_STYLE, Am_Black)      //color of inside
          .Set(Am_INNER_HEIGHT, scrollable_height_init);
  SP_Scroll_Agg.Get_Object(Am_H_SCROLLER)
      .Get_Object(Am_COMMAND)
      .Set(Am_DO_METHOD, h_scroll_do);
  SP_Scroll_Agg.Get_Object(Am_V_SCROLLER)
      .Get_Object(Am_COMMAND)
      .Set(Am_DO_METHOD, v_scroll_do);

  Am_Screen.Add_Part(SP_Outer_Win);
  SP_Outer_Win.Add_Part(SP_Clip_Win);
  SP_Clip_Win.Add_Part(SP_Scroll_Agg);
  SP_Outer_Win.Add_Part(SP_Long_Win);

  // ** Part 2:  Ship and Beam groups

  SP_Ship_Agg = Am_Group.Create("SP_Ship_Agg")
                    .Set(Am_WIDTH, scrollable_width_init)
                    .Set(Am_HEIGHT, scrollable_height_init);
  SP_Beam_Agg = Am_Group.Create("SP_Beam_Agg")
                    .Set(Am_WIDTH, scrollable_width_init)
                    .Set(Am_HEIGHT, scrollable_height_init);
  SP_Star_Agg = Am_Group.Create("SP_Star_Agg")
                    .Set(Am_WIDTH, scrollable_width_init)
                    .Set(Am_HEIGHT, scrollable_height_init);
  SP_Scroll_Agg.Add_Part(SP_Star_Agg); //star_agg added first so behind ships
  SP_Scroll_Agg.Add_Part(SP_Ship_Agg);
  SP_Scroll_Agg.Add_Part(SP_Beam_Agg);

  // ** Part 3:  Create buttons and decoration

  SP_Long_Label = Am_Text.Create("SP_Long_Label")
                      .Add(Am_CENTER_X_OBJ, SP_Long_Win)
                      .Set(Am_LEFT, Am_Center_X_Is_Center_Of)
                      .Set(Am_TOP, (long_label_top))
                      .Set(Am_TEXT, "Long Range Scan")
                      .Set(Am_FONT, SP_Long_Font);

  SP_Button_Panel =
      Am_Button_Panel.Create("space_buttons")
          .Add(Am_CENTER_X_OBJ, SP_Long_Win)
          .Set(Am_LEFT, Am_Center_X_Is_Center_Of)
          .Set(Am_TOP, (clip_win_top))
          .Set(Am_FILL_STYLE, SP_Background_Style)
          .Set(Am_H_ALIGN, Am_CENTER_ALIGN)
          .Set(Am_FIXED_WIDTH, true)
          .Set(
              Am_ITEMS,
              Am_Value_List()
                  .Add(SP_Springy_Command = Am_Command.Create("springy_command")
                                                .Add(IS_SPRINGY, false)
                                                .Set(Am_LABEL, (springy_label))
                                                .Set(Am_DO_METHOD, do_springy))
                  //set Am_SELECTION_WIDGET with SP_Clip_Win because the
                  //undohandler is in that window.  If not supplied, then
                  //Am_Undo_Command.Create looks in the window of the
                  //SP_Button_Panel which is SP_Outer_Win, which does NOT have
                  //the undo handler.
                  .Add(Am_Undo_Command.Create().Set(Am_SELECTION_WIDGET,
                                                    SP_Clip_Win))
                  .Add(Am_Redo_Command.Create().Set(Am_SELECTION_WIDGET,
                                                    SP_Clip_Win))
                  .Add(Am_Quit_No_Ask_Command.Create()));

  SP_Outer_Win.Add_Part(SP_Button_Panel)
      .Add_Part(SP_Short_Label)
      .Add_Part(SP_Long_Label);
}

/* ******************* */
/*        Main         */
/* ******************* */

int
main()
{
  Am_Initialize();

  init_styles();
  init_wins();   // defines SP_Outer_Win, SP_Clip_Win, SP_Short_Agg,
                 //    and SP_Long_Win
  init_protos(); // defines SP_Ship

  //  cout << "Turning on debugging\n";
  //  Am_Set_Inter_Trace(Am_INTER_TRACE_ALL);

  install_ship_inters();
  install_phaser_inter();
  install_tractor_inter();
  install_long_range_feedback();
  install_stars();

  Am_Main_Event_Loop();
  Am_Cleanup();

  return 0;
}
