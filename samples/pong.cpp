/* ****************************** -*-c++-*- *******************************/
/* ************************************************************************
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

// Game like Pong

#include <amulet.h>

#include <stdlib.h> // for rand()

using namespace std;

Am_Object window, main_group, paddle1, paddle2, ball, start_button, anim,
    anim_cmd;

int y_offset = 0;
int x_offset = 0;
bool player1sturn = true;

#define ball_size 21
#define top_height 50
#define paddle_height 60
#define score_top 12
#define paddle_width 10

Am_Slot_Key PLAYER1_SCORE = Am_Register_Slot_Name("PLAYER1_SCORE");
Am_Slot_Key PLAYER2_SCORE = Am_Register_Slot_Name("PLAYER2_SCORE");
Am_Slot_Key SCORE = Am_Register_Slot_Name("SCORE");
Am_Slot_Key LABEL1 = Am_Register_Slot_Name("LABEL1");
Am_Slot_Key LABEL2 = Am_Register_Slot_Name("LABEL2");
Am_Slot_Key SCORE1 = Am_Register_Slot_Name("SCORE1");
Am_Slot_Key SCORE2 = Am_Register_Slot_Name("SCORE2");

void
reset_game()
{
  Am_Stop_Interactor(anim);
  ball.Set(Am_VISIBLE, false);
  start_button.Set(Am_VISIBLE, true);
}

Am_Define_Method(Am_Current_Location_Method, void, paddle_interim_do,
                 (Am_Object /* inter */, Am_Object object_modified,
                  Am_Inter_Location data))
{
  int left, top, w, h;
  data.Get_Points(left, top, w, h);
  object_modified.Set(Am_TOP, top);
}

Am_Define_Method(Am_Object_Method, void, new_game_method, (Am_Object /*cmd*/))
{
  window.Set(PLAYER1_SCORE, 0);
  window.Set(PLAYER2_SCORE, 0);
  player1sturn = true;
  reset_game();
}

Am_Define_Method(Am_Object_Method, void, connect_to_remote_method,
                 (Am_Object /*cmd*/))
{
  Am_Value v = Am_Get_Input_From_Dialog(
      Am_Value_List().Add("Type machine for remote user:"), "");
  if (v.Valid()) {
    cout << "**Connect to remote user: " << v << endl << flush;
  }
}

Am_Define_Method(Am_Object_Method, void, start_game, (Am_Object /*cmd*/))
{
  start_button.Set(Am_VISIBLE, false);
  ball.Set(Am_LEFT, (int)((int)window.Get(Am_WIDTH) - ball_size) / 2);
  ball.Set(Am_TOP, (int)((int)main_group.Get(Am_HEIGHT) - ball_size) / 2);
  ball.Set(Am_VISIBLE, true);
  paddle1.Set(Am_TOP,
              (int)((int)main_group.Get(Am_HEIGHT) - paddle_height) / 2);
  paddle2.Set(Am_TOP,
              (int)((int)main_group.Get(Am_HEIGHT) - paddle_height) / 2);
  y_offset = (rand() & 7) + 1;
  if (rand() & 1)
    y_offset = -y_offset;
  x_offset = (rand() & 7) + 1;
  if (player1sturn)
    x_offset = -x_offset;
  Am_Start_Interactor(anim);
}

Am_Define_Method(Am_Object_Method, void, ball_anim_method, (Am_Object /*cmd*/))
{
  int new_x = (int)ball.Get(Am_LEFT) + x_offset;
  int new_y = (int)ball.Get(Am_TOP) + y_offset;
  if (new_y <= 0 || new_y >= (int)main_group.Get(Am_HEIGHT) - ball_size) {
    // y bounce
    y_offset = -y_offset;
  }
  if (new_x < paddle_width + 10) {
    if ((new_y + ball_size / 2) < (int)paddle1.Get(Am_TOP) ||
        (new_y + ball_size / 2) > (int)paddle1.Get(Am_TOP) + paddle_height) {
      //player1 loses
      window.Set(PLAYER2_SCORE, (int)window.Get(PLAYER2_SCORE) + 1);
      reset_game();
      player1sturn = false;
    } else { // bounce off paddle1
      x_offset = -x_offset;
    }
  } else if (new_x + ball_size >
             (int)main_group.Get(Am_WIDTH) - paddle_width - 10) {
    if ((new_y + ball_size / 2) < (int)paddle2.Get(Am_TOP) ||
        (new_y + ball_size / 2) > (int)paddle2.Get(Am_TOP) + paddle_height) {
      //player2 loses
      window.Set(PLAYER1_SCORE, (int)window.Get(PLAYER1_SCORE) + 1);
      reset_game();
      player1sturn = true;
    } else { // bounce off paddle2
      x_offset = -x_offset;
    }
  }
  ball.Set(Am_LEFT, new_x);
  ball.Set(Am_TOP, new_y);
}

Am_Define_String_Formula(score_as_string)
{
  int score = self.Get(SCORE);
  char line[100];
  OSTRSTREAM_CONSTR(oss, line, 100, ios::out);
  oss << score << ends;
  OSTRSTREAM_COPY(oss, line, 100);
  Am_String str = line;
  return str;
}
Am_Define_Formula(int, right_of_label1)
{
  Am_Object label = self.Get_Sibling(LABEL1);
  return (int)label.Get(Am_LEFT) + (int)label.Get(Am_WIDTH) + 5;
}
Am_Define_Formula(int, left_of_score2)
{
  Am_Object score = self.Get_Sibling(SCORE2);
  return (int)score.Get(Am_LEFT) - (int)self.Get(Am_WIDTH) - 5;
}
int
main()
{
  Am_Initialize();
  Am_Font bigfont(Am_FONT_SERIF, true, false, false, Am_FONT_LARGE);
  anim =
      Am_Animation_Interactor.Create("anim")
          .Set(Am_START_WHEN, false)
          .Set(Am_STOP_WHEN, false)
          .Set(Am_START_WHERE_TEST, true)
          .Set(Am_RUN_ALSO, true)
          .Set(Am_PRIORITY, -100)            //let other interactors run
          .Set(Am_REPEAT_DELAY, Am_Time(20)) //milliseconds between increments
          .Set_Part(Am_COMMAND,
                    anim_cmd =
                        Am_Animation_Command.Create("anim_cmd")
                            .Set(Am_INTERIM_DO_METHOD, ball_anim_method));
  window =
      Am_Window.Create("Main_Window")
          .Set(Am_TITLE, "Amulet PONG")
          .Set(Am_FILL_STYLE, Am_Amulet_Purple)
          .Set(Am_WIDTH, 600)
          .Set(Am_HEIGHT, 450)
          .Add(PLAYER1_SCORE, 0)
          .Add(PLAYER2_SCORE, 0)
          .Add_Part(LABEL1, Am_Text.Create("label1")
                                .Set(Am_LEFT, 10)
                                .Set(Am_TOP, score_top)
                                .Set(Am_TEXT, "Score:")
                                .Set(Am_FONT, bigfont)
                                .Set(Am_LINE_STYLE, Am_Red))
          .Add_Part(SCORE1, Am_Text.Create("score1")
                                .Set(Am_LINE_STYLE, Am_Red)
                                .Set(Am_LEFT, right_of_label1)
                                .Set(Am_TOP, score_top)
                                .Add(SCORE, Am_From_Owner(PLAYER1_SCORE))
                                .Set(Am_TEXT, score_as_string)
                                .Set(Am_FONT, bigfont))
          .Set(Am_RIGHT_OFFSET, 10)
          .Add_Part(SCORE2, Am_Text.Create("score2")
                                .Set(Am_LINE_STYLE, Am_Blue)
                                .Set(Am_LEFT, Am_Right_Is_Right_Of_Owner)
                                .Set(Am_TOP, score_top)
                                .Add(SCORE, Am_From_Owner(PLAYER2_SCORE))
                                .Set(Am_TEXT, score_as_string)
                                .Set(Am_FONT, bigfont))
          .Add_Part(LABEL2, Am_Text.Create("label2")
                                .Set(Am_LEFT, left_of_score2)
                                .Set(Am_TOP, score_top)
                                .Set(Am_TEXT, "Score:")
                                .Set(Am_FONT, bigfont)
                                .Set(Am_LINE_STYLE, Am_Blue))
          .Add_Part(
              Am_Button_Panel.Create("buttons")
                  .Set(Am_TOP, score_top - 2)
                  .Set(Am_LEFT, Am_Center_X_Is_Center_Of_Owner)
                  .Set(Am_LAYOUT, Am_Horizontal_Layout)
                  .Set(Am_H_SPACING, 10)
                  .Set(Am_FIXED_WIDTH, false)
                  .Set(Am_ITEMS,
                       Am_Value_List()
                           .Add(Am_Command.Create("new game")
                                    .Set(Am_LABEL, "New Game")
                                    .Set(Am_DO_METHOD, new_game_method))
                           .Add(Am_Quit_No_Ask_Command.Create())
                           .Add(Am_Command.Create("connect user")
                                    .Set(Am_LABEL, "Connect to Remote User")
                                    .Set(Am_DO_METHOD,
                                         connect_to_remote_method))))
          .Add_Part(Am_Border_Rectangle.Create()
                        .Set(Am_LEFT, 0)
                        .Set(Am_TOP, top_height)
                        .Set(Am_WIDTH, Am_From_Owner(Am_WIDTH))
                        .Set(Am_HEIGHT, Am_From_Owner(Am_HEIGHT, -top_height))
                        .Set(Am_FILL_STYLE, Am_Black))
          .Add_Part(
              main_group =
                  Am_Group.Create("main group")
                      .Set(Am_LEFT, 0)
                      .Set(Am_TOP, top_height)
                      .Set(Am_WIDTH, Am_From_Owner(Am_WIDTH))
                      .Set(Am_HEIGHT, Am_From_Owner(Am_HEIGHT, -top_height))
                      .Add_Part(
                          Am_Rectangle.Create("center")
                              .Set(Am_TOP, 0)
                              .Set(Am_LEFT, Am_Center_X_Is_Center_Of_Owner)
                              .Set(Am_WIDTH, 3)
                              .Set(Am_HEIGHT, Am_From_Owner(Am_HEIGHT))
                              .Set(Am_LINE_STYLE, Am_No_Style)
                              .Set(Am_FILL_STYLE, Am_White))
                      .Add_Part(paddle1 =
                                    Am_Rectangle.Create()
                                        .Set(Am_TOP, 200)
                                        .Set(Am_LEFT, 10)
                                        .Set(Am_WIDTH, paddle_width)
                                        .Set(Am_HEIGHT, paddle_height)
                                        .Set(Am_LINE_STYLE, Am_No_Style)
                                        .Set(Am_FILL_STYLE, Am_Red)
                                        .Add_Part(Am_Move_Grow_Interactor
                                                      .Create("paddle1move")
                                                      .Set(Am_INTERIM_DO_METHOD,
                                                           paddle_interim_do)
                                                      .Set(Am_DO_METHOD, 0L)))
                      .Set(Am_RIGHT_OFFSET, 10)
                      .Add_Part(
                          paddle2 =
                              Am_Rectangle.Create()
                                  .Set(Am_TOP, 200)
                                  .Set(Am_LEFT, Am_Right_Is_Right_Of_Owner)
                                  .Set(Am_WIDTH, paddle_width)
                                  .Set(Am_HEIGHT, paddle_height)
                                  .Set(Am_LINE_STYLE, Am_No_Style)
                                  .Set(Am_FILL_STYLE, Am_Blue)
                                  .Add_Part(Am_Move_Grow_Interactor
                                                .Create("paddle1move")
                                                .Set(Am_INTERIM_DO_METHOD,
                                                     paddle_interim_do)
                                                .Set(Am_DO_METHOD, 0L)))
                      .Add_Part(ball = Am_Arc.Create()
                                           .Set(Am_VISIBLE, false)
                                           .Set(Am_TOP, 200)
                                           .Set(Am_LEFT, 200)
                                           .Set(Am_WIDTH, ball_size)
                                           .Set(Am_HEIGHT, ball_size)
                                           .Set(Am_LINE_STYLE, Am_No_Style)
                                           .Set(Am_FILL_STYLE, Am_Yellow)
                                           .Add_Part(anim))

                      .Add_Part(
                          start_button =
                              Am_Button.Create()
                                  .Set(Am_LEFT, Am_Center_X_Is_Center_Of_Owner)
                                  .Set(Am_TOP, Am_Center_Y_Is_Center_Of_Owner)
                                  .Set(Am_FONT, bigfont)
                                  .Set(Am_WIDTH, 100)
                                  .Set(Am_HEIGHT, 100)
                                  .Set(Am_COMMAND,
                                       Am_Command.Create()
                                           .Set(Am_LABEL, "Start!")
                                           .Set(Am_DO_METHOD, start_game))))

      ;
  Am_Screen.Add_Part(window);

  Am_Main_Event_Loop();
  Am_Cleanup();
  return 0;
}
