/* ************************************************************************
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

#include <amulet.h>

Am_Object Player_State;

Am_Slot_Key SQUARE0 = Am_Register_Slot_Name("SQUARE0");
Am_Slot_Key SQUARE1 = Am_Register_Slot_Name("SQUARE1");
Am_Slot_Key SQUARE2 = Am_Register_Slot_Name("SQUARE2");
Am_Slot_Key SQUARE3 = Am_Register_Slot_Name("SQUARE3");
Am_Slot_Key BOARD = Am_Register_Slot_Name("BOARD");
Am_Slot_Key PIECE = Am_Register_Slot_Name("PIECE");
Am_Slot_Key CIRCLE = Am_Register_Slot_Name("CIRCLE");
Am_Slot_Key TEXT1 = Am_Register_Slot_Name("TEXT1");
Am_Slot_Key TEXT2 = Am_Register_Slot_Name("TEXT2");
Am_Slot_Key FEEDBACK = Am_Register_Slot_Name("FEEDBACK");
Am_Slot_Key BUTTON = Am_Register_Slot_Name("BUTTON");
Am_Slot_Key CROWN = Am_Register_Slot_Name("CROWN");
Am_Slot_Key CROWN0 = Am_Register_Slot_Name("CROWN0");
Am_Slot_Key CROWN1 = Am_Register_Slot_Name("CROWN1");
Am_Slot_Key CROWN2 = Am_Register_Slot_Name("CROWN2");
Am_Slot_Key CROWN3 = Am_Register_Slot_Name("CROWN3");
Am_Slot_Key CROWN4 = Am_Register_Slot_Name("CROWN4");
Am_Slot_Key CROWN5 = Am_Register_Slot_Name("CROWN5");
Am_Slot_Key CROWN6 = Am_Register_Slot_Name("CROWN6");
Am_Slot_Key PIECE0 = Am_Register_Slot_Name("PIECE0");
Am_Slot_Key PIECE1 = Am_Register_Slot_Name("PIECE1");
Am_Slot_Key PIECE2 = Am_Register_Slot_Name("PIECE2");
Am_Slot_Key PIECE3 = Am_Register_Slot_Name("PIECE3");
Am_Slot_Key PIECE4 = Am_Register_Slot_Name("PIECE4");
Am_Slot_Key PIECE5 = Am_Register_Slot_Name("PIECE5");
Am_Slot_Key PIECE6 = Am_Register_Slot_Name("PIECE6");
Am_Slot_Key PIECE7 = Am_Register_Slot_Name("PIECE7");
Am_Slot_Key PIECE8 = Am_Register_Slot_Name("PIECE8");
Am_Slot_Key PIECE9 = Am_Register_Slot_Name("PIECE9");
Am_Slot_Key PIECE10 = Am_Register_Slot_Name("PIECE10");
Am_Slot_Key PIECE11 = Am_Register_Slot_Name("PIECE11");
Am_Slot_Key ROW = Am_Register_Slot_Name("ROW");
Am_Slot_Key COLUMN = Am_Register_Slot_Name("COLUMN");
Am_Slot_Key KINGED = Am_Register_Slot_Name("KINGED");
Am_Slot_Key JUMPED = Am_Register_Slot_Name("JUMPED");
Am_Slot_Key LAST_JUMPED = Am_Register_Slot_Name("LAST JUMPED");
Am_Slot_Key MOVED = Am_Register_Slot_Name("MOVED");
Am_Slot_Key BLACK_PLAYER = Am_Register_Slot_Name("BLACK PLAYER");
Am_Slot_Key NUM_RED = Am_Register_Slot_Name("NUM RED");
Am_Slot_Key NUM_BLACK = Am_Register_Slot_Name("NUM BLACK");
Am_Slot_Key MOVE_INTER = Am_Register_Slot_Name("MOVE INTER");

Am_Slot_Key ANIMATED = Am_Register_Slot_Name("ANIMATED");

Am_Slot_Key piece_slot[12];

Am_Style Beige("beige");

Am_Object Last_Jumped_Piece;

Am_Object Red_Pieces;
Am_Object Black_Pieces;

class Board_Content
{
public:
  Am_Slot_Key piece;
  bool black;
};

Board_Content Board[32];

Am_Define_Method(Am_Object_Method, void, reset_action, (Am_Object))
{
  int i;
  for (i = 12; i < 20; ++i)
    Board[i].piece = Am_NO_SLOT;
  for (i = 0; i < 12; ++i) {
    Board[31 - i].black = false;
    Board[31 - i].piece = piece_slot[i];
    Red_Pieces.Get_Object(piece_slot[i])
        .Set(ROW, 7 - (i / 4))
        .Set(COLUMN, (3 - (i % 4)) * 2 + ((i / 4) % 2))
        .Set(KINGED, false)
        .Set(Am_VISIBLE, true);
    Board[i].black = true;
    Board[i].piece = piece_slot[i];
    Black_Pieces.Get_Object(piece_slot[i])
        .Set(ROW, i / 4)
        .Set(COLUMN, (i % 4) * 2 + 1 - ((i / 4) % 2))
        .Set(KINGED, false)
        .Set(Am_VISIBLE, true);
  }

  Player_State.Set(BLACK_PLAYER, true).Set(NUM_RED, 12).Set(NUM_BLACK, 12);
  if (Last_Jumped_Piece.Valid()) {
    Last_Jumped_Piece.Set(LAST_JUMPED, false);
    Last_Jumped_Piece = Am_Object();
  }
}

#define DIFF(val1, val2) ((val1 > val2) ? (val1 - val2) : (val2 - val1))

Am_Define_Method(Am_Custom_Gridding_Method, void, snap_to_black,
                 (Am_Object inter, const Am_Object &ref_obj, int x, int y,
                  int &out_x, int &out_y))
{
  Am_Object piece = inter.Get_Owner();
  Am_Object window;
  window = piece.Get(Am_WINDOW);
  Am_Object board = window.Get_Object(BOARD);
  piece.Set(MOVED, false);
  piece.Set(JUMPED, false);
  out_x = x;
  out_y = y;
  Am_Translate_Coordinates(ref_obj, x, y, board, x, y);
  if ((x < 0) || (y < 0) || (x >= 400) || (y >= 400))
    return;
  int row, col;
  row = (y + 25) / 50;
  col = (x + 25) / 50;
  if ((row % 2) == (col % 2))
    return;
  if (Board[(row * 4) + (col / 2)].piece)
    return;
  bool black_player = piece.Get(BLACK_PLAYER);
  int rank = piece.Get(Am_RANK);
  Am_Object piece_state;
  if (black_player)
    piece_state = Black_Pieces.Get_Object(piece_slot[rank]);
  else
    piece_state = Red_Pieces.Get_Object(piece_slot[rank]);
  int piece_row = piece_state.Get(ROW);
  int piece_col = piece_state.Get(COLUMN);
  if (!(bool)piece_state.Get(KINGED)) {
    if (black_player) {
      if (row <= piece_row)
        return;
    } else {
      if (row >= piece_row)
        return;
    }
  }
  int row_diff = DIFF(piece_row, row);
  int col_diff = DIFF(piece_col, col);
  if ((row_diff != col_diff) || (row_diff > 2))
    return;
  if ((row_diff == 1) && (bool)piece.Get(LAST_JUMPED))
    return;
  if (row_diff == 2) {
    int jump_row = (row + piece_row) / 2;
    int jump_col = (col + piece_col) / 2;
    int jump_index = (jump_row * 4) + (jump_col / 2);
    if (!Board[jump_index].piece || (Board[jump_index].black == black_player))
      return;
    piece.Set(JUMPED, true);
  }
  out_x = col * 50 /* + (int)inter.Get (Am_X_OFFSET)*/;
  out_y = row * 50 /* + (int)inter.Get (Am_Y_OFFSET)*/;
  Am_Translate_Coordinates(board, out_x, out_y, ref_obj, out_x, out_y);
  piece.Set(MOVED, true);
  piece.Set(ROW, row);
  piece.Set(COLUMN, col);
}

Am_Define_Method(Am_Object_Method, void, move_piece, (Am_Object inter))
{
  Am_Object piece, feedback;
  feedback = inter.Get(Am_FEEDBACK_OBJECT);
  if (feedback.Valid()) {
    feedback.Set(Am_VISIBLE, false);
  }

  piece = inter.Get(Am_OBJECT_MODIFIED);
  if ((bool)piece.Get(MOVED)) {
    int rank = piece.Get(Am_RANK);
    bool black_player = piece.Get(BLACK_PLAYER);
    Am_Slot_Key piece_key = piece_slot[rank];
    Am_Object piece_state;
    if (black_player)
      piece_state = Black_Pieces.Get_Object(piece_key);
    else
      piece_state = Red_Pieces.Get_Object(piece_key);
    int piece_row = piece_state.Get(ROW);
    int piece_col = piece_state.Get(COLUMN);
    Board[(piece_row * 4) + (piece_col / 2)].piece = Am_NO_SLOT;
    int new_row = piece.Get(ROW);
    int new_col = piece.Get(COLUMN);
    Board[(new_row * 4) + (new_col / 2)].piece = piece_key;
    Board[(new_row * 4) + (new_col / 2)].black = black_player;
    piece_state.Set(ROW, new_row).Set(COLUMN, new_col);
    if (black_player) {
      if (new_row == 7)
        piece_state.Set(KINGED, true);
    } else {
      if (new_row == 0)
        piece_state.Set(KINGED, true);
    }
    if ((bool)piece.Get(JUMPED)) {
      if (Last_Jumped_Piece.Valid() && (Last_Jumped_Piece != piece))
        Last_Jumped_Piece.Set(LAST_JUMPED, false);
      Last_Jumped_Piece = piece;
      piece.Set(LAST_JUMPED, true);
      int jump_row = (new_row + piece_row) / 2;
      int jump_col = (new_col + piece_col) / 2;
      int jump_index = (jump_row * 4) + (jump_col / 2);
      Am_Slot_Key jump_piece = Board[jump_index].piece;
      bool jump_black = Board[jump_index].black;
      Board[jump_index].piece = Am_NO_SLOT;
      if (jump_black) {
        Black_Pieces.Get_Object(jump_piece).Set(Am_VISIBLE, false);
        int num = (int)Player_State.Get(NUM_BLACK) - 1;
        Player_State.Set(NUM_BLACK, num);
        if (!num)
          piece.Set(LAST_JUMPED, false);
      } else {
        Red_Pieces.Get_Object(jump_piece).Set(Am_VISIBLE, false);
        int num = (int)Player_State.Get(NUM_RED) - 1;
        Player_State.Set(NUM_RED, num);
        if (!num)
          piece.Set(LAST_JUMPED, false);
      }
    } else if (Last_Jumped_Piece.Valid()) {
      Last_Jumped_Piece.Set(LAST_JUMPED, false);
      Last_Jumped_Piece = nullptr;
    }
    Player_State.Set(BLACK_PLAYER, !black_player);
  }
}

Am_Define_Formula(int, piece_left)
{
  int rank = self.Get(Am_RANK);
  bool black_player = self.Get(BLACK_PLAYER);
  int col;

#if 0
  bool visible = self.Get (Am_VISIBLE);
  if (!visible)
    col = -1;  // disappear offscreen
  else
#endif
  if (black_player)
    col = Black_Pieces.Get_Object(piece_slot[rank]).Get(COLUMN);
  else
    col = Red_Pieces.Get_Object(piece_slot[rank]).Get(COLUMN);
  return col * 50;
}

Am_Define_Formula(int, piece_top)
{
  int rank = self.Get(Am_RANK);
  bool black_player = self.Get(BLACK_PLAYER);
  int row;

#if 0
  bool visible = self.Get (Am_VISIBLE);
  if (!visible)
    row = black_player ? 8 : -1;
  else
#endif
  if (black_player)
    row = Black_Pieces.Get_Object(piece_slot[rank]).Get(ROW);
  else
    row = Red_Pieces.Get_Object(piece_slot[rank]).Get(ROW);
  return row * 50;
}

Am_Define_Formula(bool, piece_visible)
{
  int rank = self.Get(Am_RANK);
  bool black_player = self.Get(BLACK_PLAYER);
  if (black_player)
    return (bool)Black_Pieces.Get_Object(piece_slot[rank]).Get(Am_VISIBLE);
  else
    return (bool)Red_Pieces.Get_Object(piece_slot[rank]).Get(Am_VISIBLE);
}

Am_Define_Formula(bool, make_kinged)
{
  Am_Object piece = self.Get_Owner();
  int rank = piece.Get(Am_RANK);
  bool black_player = piece.Get(BLACK_PLAYER);
  if (black_player)
    return (bool)Black_Pieces.Get_Object(piece_slot[rank]).Get(KINGED);
  else
    return (bool)Red_Pieces.Get_Object(piece_slot[rank]).Get(KINGED);
}

Am_Define_Object_Formula(find_feedback)
{
  Am_Object window;
  window = self.Get_Owner().Get(Am_WINDOW);
  if (window.Valid())
    return window.Get_Object(FEEDBACK);
  else
    return nullptr;
}

Am_Define_Formula(bool, player_turn)
{
  Am_Object piece = self.Get_Owner();
  return (
      (bool)piece.Get(LAST_JUMPED) ||
      ((bool)piece.Get(BLACK_PLAYER) == (bool)Player_State.Get(BLACK_PLAYER)));
}

Am_Define_Formula(int, window_height)
{
  return 470 + (int)self.Get_Object(BUTTON).Get(Am_HEIGHT);
}

Am_Define_Formula(int, board_top)
{
  return 20 + (int)self.Get_Owner().Get_Object(BUTTON).Get(Am_HEIGHT);
}

Am_Define_Formula(int, border_top)
{
  return 16 + (int)self.Get_Owner().Get_Object(BUTTON).Get(Am_HEIGHT);
}

Am_Define_Formula(int, message_width)
{
  return (int)self.Get_Object(BOARD).Get_Object(TEXT1).Get(Am_WIDTH) + 20;
}

Am_Define_Formula(int, message_height)
{
  return 40 + (int)self.Get_Object(BOARD).Get_Object(TEXT1).Get(Am_HEIGHT) +
         (int)self.Get_Object(BOARD).Get_Object(TEXT2).Get(Am_HEIGHT) +
         (int)self.Get_Object(BOARD).Get_Object(CIRCLE).Get(Am_HEIGHT);
}

Am_Define_No_Self_Formula(const char *, winner_text)
{
  if ((int)Player_State.Get(NUM_RED) == 0)
    return "BLACK";
  else
    return "WHITE";
}

Am_Define_No_Self_Formula(Am_Wrapper *, winner_color)
{
  if ((int)Player_State.Get(NUM_RED) == 0)
    return Am_Black;
  else
    return Beige;
}

Am_Define_No_Self_Formula(bool, found_winner)
{
  return ((int)Player_State.Get(NUM_RED) == 0) ||
         ((int)Player_State.Get(NUM_BLACK) == 0);
}

Am_Define_Formula(int, black_at_bottom)
{
  if ((bool)self.Get_Object(Am_OPERATES_ON).Get(BLACK_PLAYER))
    return 550;
  else
    return -1000;
}

Am_Define_Formula(bool, moving_animator_active)
{
  Am_Object obj = self.Get_Object(Am_OPERATES_ON);

  return (bool)obj.Get_Object(Am_WINDOW).Get(ANIMATED) &&
         (bool)obj.Get(Am_VISIBLE);
}

Am_Define_Formula(bool, visible_animator_active)
{
  Am_Object obj = self.Get_Object(Am_OPERATES_ON);

  return obj.Get_Object(Am_WINDOW).Get(ANIMATED);
}

Am_Define_Method(Am_Object_Method, void, toggle_animation, (Am_Object command))
{
  Am_Object inter = command.Get_Owner();
  Am_Object window = inter.Get_Owner();
  window.Set(ANIMATED, !(bool)window.Get(ANIMATED));
}

int
main(int argc, char **argv)
{
  piece_slot[0] = PIECE0;
  piece_slot[1] = PIECE1;
  piece_slot[2] = PIECE2;
  piece_slot[3] = PIECE3;
  piece_slot[4] = PIECE4;
  piece_slot[5] = PIECE5;
  piece_slot[6] = PIECE6;
  piece_slot[7] = PIECE7;
  piece_slot[8] = PIECE8;
  piece_slot[9] = PIECE9;
  piece_slot[10] = PIECE10;
  piece_slot[11] = PIECE11;

  Am_Initialize();

  Am_Font big_bold_font(Am_FONT_FIXED, true, false, false, Am_FONT_LARGE);

  Red_Pieces = Am_Root_Object.Create("red pieces");
  Black_Pieces = Am_Root_Object.Create("black pieces");

  int i;
  for (i = 12; i < 20; ++i)
    Board[i].piece = Am_NO_SLOT;
  for (i = 0; i < 12; ++i) {
    Board[31 - i].black = false;
    Board[31 - i].piece = piece_slot[i];
    Red_Pieces.Add_Part(piece_slot[i],
                        Am_Root_Object.Create("red pos")
                            .Add(ROW, 7 - (i / 4))
                            .Add(COLUMN, (3 - (i % 4)) * 2 + ((i / 4) % 2))
                            .Add(KINGED, false)
                            .Add(Am_VISIBLE, true));
    Board[i].black = true;
    Board[i].piece = piece_slot[i];
    Black_Pieces.Add_Part(piece_slot[i],
                          Am_Root_Object.Create("blk pos")
                              .Add(ROW, i / 4)
                              .Add(COLUMN, (i % 4) * 2 + 1 - ((i / 4) % 2))
                              .Add(KINGED, false)
                              .Add(Am_VISIBLE, true));
  }

  Player_State = Am_Root_Object.Create("player state")
                     .Add(BLACK_PLAYER, true)
                     .Add(NUM_RED, 12)
                     .Add(NUM_BLACK, 12);

  Am_Object Piece_Feedback =
      Am_Group.Create("piece feedback")
          .Set(Am_WIDTH, 50)
          .Set(Am_HEIGHT, 50)
          .Set(Am_VISIBLE, false)
          .Add_Part(CROWN, Am_Arc.Create("feedback outline")
                               .Set(Am_FILL_STYLE, 0L)
                               .Set(Am_LINE_STYLE, Am_Black)
                               .Set(Am_LEFT, 1)
                               .Set(Am_TOP, 1)
                               .Set(Am_WIDTH, 48)
                               .Set(Am_HEIGHT, 48))
          .Add_Part(CIRCLE, Am_Arc.Create("feedback circle")
                                .Set(Am_FILL_STYLE, 0L)
                                .Set(Am_LINE_STYLE, Am_Yellow)
                                .Set(Am_LEFT, 2)
                                .Set(Am_TOP, 2)
                                .Set(Am_WIDTH, 46)
                                .Set(Am_HEIGHT, 46));

  Am_Object moving_animator = Am_Animator.Create("checker anim")
                                  .Set(Am_ACTIVE, moving_animator_active)
                                  .Set(Am_DURATION, Am_Time(500));
  Am_Object visible_animator = Am_Visible_Animator.Create()
                                   .Set(Am_LEFT, -1000)
                                   .Set(Am_TOP, black_at_bottom)
                                   .Set(Am_ACTIVE, visible_animator_active)
                                   .Set(Am_DURATION, Am_Time(500));

  Am_Object Piece =
      Am_Group.Create("piece")
          .Set(Am_RANK, 0)
          .Add(BLACK_PLAYER, true)
          .Set(Am_WIDTH, 50)
          .Set(Am_HEIGHT, 50)
          .Add(MOVED, false)
          .Add(JUMPED, false)
          .Add(LAST_JUMPED, false)
          .Add(ROW, 0)
          .Add(COLUMN, 0)
          .Set(Am_LEFT, piece_left)
          .Set(Am_TOP, piece_top)
          .Set(Am_LEFT, Am_Animate_With(moving_animator))
          .Set(Am_TOP, Am_Animate_With(moving_animator))
          .Set(Am_VISIBLE, (piece_visible))
          .Set(Am_VISIBLE, Am_Animate_With(visible_animator))
          .Add_Part(CIRCLE, Am_Arc.Create("piece circle")
                                .Set(Am_FILL_STYLE, Am_Black)
                                .Set(Am_LINE_STYLE, Am_White)
                                .Set(Am_LEFT, 2)
                                .Set(Am_TOP, 2)
                                .Set(Am_WIDTH, 46)
                                .Set(Am_HEIGHT, 46))
          .Add_Part(CROWN,
                    Am_Group.Create("crown")
                        .Set(Am_LEFT, 13)
                        .Set(Am_TOP, 19)
                        .Set(Am_WIDTH, 25)
                        .Set(Am_HEIGHT, 13)
                        .Add(CROWN, Am_Yellow)
                        .Set(Am_VISIBLE, (make_kinged))
                        .Add_Part(CROWN0,
                                  Am_Line.Create("crown 0")
                                      .Set(Am_LINE_STYLE, Am_From_Owner(CROWN))
                                      .Set(Am_X1, 0)
                                      .Set(Am_Y1, 12)
                                      .Set(Am_X2, 24)
                                      .Set(Am_Y2, 12))
                        .Add_Part(CROWN1,
                                  Am_Line.Create("crown 1")
                                      .Set(Am_LINE_STYLE, Am_From_Owner(CROWN))
                                      .Set(Am_X1, 0)
                                      .Set(Am_Y1, 0)
                                      .Set(Am_X2, 0)
                                      .Set(Am_Y2, 12))
                        .Add_Part(CROWN2,
                                  Am_Line.Create("crown 2")
                                      .Set(Am_LINE_STYLE, Am_From_Owner(CROWN))
                                      .Set(Am_X1, 24)
                                      .Set(Am_Y1, 0)
                                      .Set(Am_X2, 24)
                                      .Set(Am_Y2, 12))
                        .Add_Part(CROWN3,
                                  Am_Line.Create("crown 3")
                                      .Set(Am_LINE_STYLE, Am_From_Owner(CROWN))
                                      .Set(Am_X1, 0)
                                      .Set(Am_Y1, 0)
                                      .Set(Am_X2, 6)
                                      .Set(Am_Y2, 6))
                        .Add_Part(CROWN4,
                                  Am_Line.Create("crown 4")
                                      .Set(Am_LINE_STYLE, Am_From_Owner(CROWN))
                                      .Set(Am_X1, 6)
                                      .Set(Am_Y1, 6)
                                      .Set(Am_X2, 12)
                                      .Set(Am_Y2, 0))
                        .Add_Part(CROWN5,
                                  Am_Line.Create("crown 5")
                                      .Set(Am_LINE_STYLE, Am_From_Owner(CROWN))
                                      .Set(Am_X1, 12)
                                      .Set(Am_Y1, 0)
                                      .Set(Am_X2, 18)
                                      .Set(Am_Y2, 6))
                        .Add_Part(CROWN6,
                                  Am_Line.Create("crown 6")
                                      .Set(Am_LINE_STYLE, Am_From_Owner(CROWN))
                                      .Set(Am_X1, 18)
                                      .Set(Am_Y1, 6)
                                      .Set(Am_X2, 24)
                                      .Set(Am_Y2, 0)))
          .Add_Part(MOVE_INTER, Am_Move_Grow_Interactor.Create("move piece")
                                    .Set(Am_ACTIVE, (player_turn))
                                    .Set(Am_START_WHERE_TEST, Am_Inter_In)
                                    .Set(Am_START_WHEN, "LEFT_DOWN")

                                    .Set(Am_FEEDBACK_OBJECT, find_feedback)
                                    .Set(Am_GRID_METHOD, snap_to_black)
                                    .Set(Am_DO_METHOD, move_piece)
                    //// NDY: Set Am_ACTIVE to formula.
                    );

  Am_Object window;

  Am_Screen.Add_Part(
      window =
          Am_Window.Create("window")
              .Add(ANIMATED, true)
              .Set(Am_TITLE, "Amulet Checkers")
              .Set(Am_TOP, 50)
              .Set(Am_LEFT, 50)
              .Set(Am_WIDTH, 450)
              .Set(Am_HEIGHT, window_height)
              .Set(Am_USE_MIN_WIDTH, true)
              .Set(Am_USE_MAX_WIDTH, true)
              .Set(Am_USE_MIN_HEIGHT, true)
              .Set(Am_USE_MAX_HEIGHT, true)
              .Set(Am_MAX_WIDTH, 450)
              .Set(Am_MAX_HEIGHT, Am_Same_As(Am_HEIGHT))
              .Set(Am_MIN_WIDTH, 450)
              .Set(Am_MIN_HEIGHT, Am_Same_As(Am_HEIGHT))
              .Set(Am_FILL_STYLE, Am_Motif_Gray)
              .Set(Am_QUERY_POSITION, true)
              .Set(Am_DOUBLE_BUFFER, true)
              .Add_Part(Am_Border_Rectangle.Create("border")
                            .Set(Am_LEFT, 21)
                            .Set(Am_TOP, (border_top))
                            .Set(Am_WIDTH, 408)
                            .Set(Am_HEIGHT, 408)
                            .Set(Am_FILL_STYLE, Am_Motif_Gray)
                            .Set(Am_SELECTED, true))
              .Add_Part(
                  BOARD,
                  Am_Group.Create("playing stuff")
                      .Set(Am_LEFT, 25)
                      .Set(Am_TOP, (board_top))
                      .Set(Am_WIDTH, 400)
                      .Set(Am_HEIGHT, 400)
                      .Add_Part(
                          Am_Map.Create("board")
                              .Set(Am_ITEMS, 16)
                              .Set(Am_LAYOUT, Am_Horizontal_Layout)
                              .Set(Am_MAX_RANK, 4)
                              .Set_Part(
                                  Am_ITEM_PROTOTYPE,
                                  Am_Group.Create("four squares")
                                      .Set(Am_WIDTH, 100)
                                      .Set(Am_HEIGHT, 100)
                                      .Add_Part(
                                          SQUARE0,
                                          Am_Rectangle.Create("red square 0")
                                              .Set(Am_FILL_STYLE,
                                                   Am_Motif_Light_Gray)
                                              .Set(Am_LINE_STYLE, 0L)
                                              .Set(Am_LEFT, 0)
                                              .Set(Am_TOP, 0)
                                              .Set(Am_WIDTH, 50)
                                              .Set(Am_HEIGHT, 50))
                                      .Add_Part(
                                          SQUARE1,
                                          Am_Rectangle.Create("black square 1")
                                              .Set(Am_FILL_STYLE, Am_Black)
                                              .Set(Am_LINE_STYLE, 0L)
                                              .Set(Am_LEFT, 50)
                                              .Set(Am_TOP, 0)
                                              .Set(Am_WIDTH, 50)
                                              .Set(Am_HEIGHT, 50))
                                      .Add_Part(
                                          SQUARE2,
                                          Am_Rectangle.Create("black square 2")
                                              .Set(Am_FILL_STYLE, Am_Black)
                                              .Set(Am_LINE_STYLE, 0L)
                                              .Set(Am_LEFT, 0)
                                              .Set(Am_TOP, 50)
                                              .Set(Am_WIDTH, 50)
                                              .Set(Am_HEIGHT, 50))
                                      .Add_Part(
                                          SQUARE3,
                                          Am_Rectangle.Create("red square 3")
                                              .Set(Am_FILL_STYLE,
                                                   Am_Motif_Light_Gray)
                                              .Set(Am_LINE_STYLE, 0L)
                                              .Set(Am_LEFT, 50)
                                              .Set(Am_TOP, 50)
                                              .Set(Am_WIDTH, 50)
                                              .Set(Am_HEIGHT, 50))))
                      .Add_Part(Am_Map.Create("white pieces")
                                    .Set(Am_WIDTH, 400)
                                    .Set(Am_HEIGHT, 400)
                                    .Set(Am_ITEMS, 12)
                                    .Set_Part(Am_ITEM_PROTOTYPE,
                                              Piece.Create("red proto")
                                                  .Get_Object(CIRCLE)
                                                  .Set(Am_FILL_STYLE, Beige)
                                                  .Set(Am_LINE_STYLE, Am_Black)
                                                  .Get_Owner()
                                                  .Get_Object(CROWN)
                                                  .Set(CROWN, Am_Purple)
                                                  .Get_Owner()
                                                  .Set(BLACK_PLAYER, false)))
                      .Add_Part(Am_Map.Create("black pieces")
                                    .Set(Am_WIDTH, 400)
                                    .Set(Am_HEIGHT, 400)
                                    .Set(Am_ITEMS, 12)
                                    .Set_Part(Am_ITEM_PROTOTYPE,
                                              Piece.Create("black proto"))))
              .Add_Part(FEEDBACK, Piece_Feedback)
              .Add_Part(Am_One_Shot_Interactor.Create("toggle animation")
                            .Set(Am_START_WHEN, "a")
                            .Get_Object(Am_COMMAND)
                            .Set(Am_DO_METHOD, toggle_animation)
                            .Get_Owner())
              .Add_Part(
                  BUTTON,
                  Am_Button.Create("quit button")
                      .Set(Am_LEFT, 20)
                      .Set(Am_TOP, 10)
                      .Set(Am_FILL_STYLE, Am_Motif_Gray)
                      .Set_Part(Am_COMMAND, Am_Quit_No_Ask_Command.Create().Set(
                                                Am_ACCELERATOR, 0)))
              .Add_Part(Am_Button.Create("reset button")
                            .Set(Am_LEFT, 120)
                            .Set(Am_TOP, 10)
                            .Set(Am_FILL_STYLE, Am_Motif_Gray)
                            .Get_Object(Am_COMMAND)
                            .Set(Am_DO_METHOD, reset_action)
                            .Set(Am_LABEL, "Reset")
                            .Get_Owner())
              .Add_Part(
                  Am_Group.Create("message box")
                      .Set(Am_LEFT, Am_Center_X_Is_Center_Of_Owner)
                      .Set(Am_TOP, Am_Center_Y_Is_Center_Of_Owner)
                      .Set(Am_WIDTH, (message_width))
                      .Set(Am_HEIGHT, (message_height))
                      .Set(Am_VISIBLE, (found_winner))
                      .Add_Part(Am_Border_Rectangle.Create("message background")
                                    .Set(Am_WIDTH, Am_From_Owner(Am_WIDTH))
                                    .Set(Am_HEIGHT, Am_From_Owner(Am_HEIGHT))
                                    .Set(Am_FILL_STYLE, Am_Motif_Gray))
                      .Add_Part(
                          BOARD,
                          Am_Group.Create("stuff")
                              .Set(Am_LEFT, Am_Center_X_Is_Center_Of_Owner)
                              .Set(Am_TOP, 10)
                              .Set(Am_WIDTH, Am_From_Part(TEXT1, Am_WIDTH))
                              .Set(Am_HEIGHT, Am_From_Owner(Am_HEIGHT))
                              .Set(Am_LAYOUT, Am_Vertical_Layout)
                              .Set(Am_V_SPACING, 10)
                              .Add_Part(TEXT1,
                                        Am_Text.Create("hooray")
                                            .Set(Am_TOP, 10)
                                            .Set(Am_TEXT, "The Winner Is:")
                                            .Set(Am_LINE_STYLE, Am_Purple)
                                            .Set(Am_FONT, big_bold_font))
                              .Add_Part(TEXT2,
                                        Am_Text.Create("winner")
                                            .Set(Am_TEXT, (winner_text))
                                            .Set(Am_LINE_STYLE, (winner_color))
                                            .Set(Am_FONT, big_bold_font))
                              .Add_Part(CIRCLE, Am_Arc.Create("winner circle")
                                                    .Set(Am_WIDTH, 46)
                                                    .Set(Am_HEIGHT, 46)
                                                    .Set(Am_FILL_STYLE,
                                                         (winner_color))))));

  Am_Object screen2;
  if (argc > 1) {
    screen2 = Am_Create_Screen(argv[1]);
    if (screen2)
      screen2.Add_Part(window.Copy());
  }

  Am_Main_Event_Loop();

  if (screen2.Valid())
    screen2.Destroy();

  Am_Cleanup();

  return 0;
}
