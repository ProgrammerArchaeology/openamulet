/* ************************************************************************
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

#include <amulet.h>
 
const char preamble[] = "We the people of the United States, in order to form a more perfect union, establish justice, insure domestic tranquility, provide for the common defense, promote the general welfare, and secure the blessings of liberty to ourselves and our posterity, do ordain and establish this Constitution for the United States of America.";
const char preambleShort[] = "We the people of the United States, in order to form a more perfect union...";

Am_Define_Formula( int, text_width )
{
  return (int)self.Get_Owner().Get( Am_WIDTH ) - 100;
}

Am_Define_Formula( int, text_width2 )
{
  return (int)self.Get_Owner().Get( Am_WIDTH ) - 150;
}

Am_Define_Formula( int, box_width )
{
  return (int)self.Get_Owner().Get( Am_WIDTH ) - 98;
}

Am_Define_Formula( int, box_width2 )
{
  return (int)self.Get_Owner().Get( Am_WIDTH ) - 148;
}

int
main( void )
{
  Am_Initialize();

  Am_Rich_Text preamble_rt = Am_Rich_Text( preamble );

  Am_Object window = Am_Window.Create( "window" )
    .Set( Am_TOP, 50 )
    .Set( Am_LEFT, 50 )
    .Set( Am_WIDTH, 300 )
    .Set( Am_HEIGHT, 400 )
    .Set( Am_DOUBLE_BUFFER, false )
    .Add_Part( Am_Rectangle.Create()
      .Set( Am_TOP, 49 )
      .Set( Am_LEFT, 49 )
      .Set( Am_HEIGHT, 152 )
      .Set( Am_WIDTH, box_width )
      .Set( Am_FILL_STYLE, Am_No_Style )
      )
    .Add_Part( Am_Rectangle.Create()
      .Set( Am_TOP, 249 )
      .Set( Am_LEFT, 74 )
      .Set( Am_HEIGHT, 102 )
      .Set( Am_WIDTH, box_width2 )
      .Set( Am_FILL_STYLE, Am_No_Style )
      )
    .Add_Part( Am_Text_Viewer.Create()
      .Set( Am_TOP, 50 )
      .Set( Am_LEFT, 50 )
      .Set( Am_HEIGHT, 150 )
      .Set( Am_WIDTH, text_width )
      .Set( Am_TEXT, preamble_rt )
      )
    .Add_Part( Am_Text_Viewer.Create()
      .Set( Am_TOP, 250 )
      .Set( Am_LEFT, 75 )
      .Set( Am_HEIGHT, 100 )
      .Set( Am_WIDTH, text_width2 )
      .Set( Am_TEXT, preamble_rt )
      );

  Am_Screen.Add_Part( window );

  Am_Main_Event_Loop();
  Am_Cleanup();
  return 0;
}
