/* ************************************************************************
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

#include <amulet.h>

using namespace std;

extern "C" int rand();

Am_Object Ship, Bullet, Bomb, my_window, Aliens;
Am_Object star_animator, alien_animator;

int num_bullets = 0;
#define MAX_BULLETS 2
#define START_NUM_ALIENS 36
Am_Slot_Key DESTROY, NUM_ALIENS, DIRECTION;

// Bullet bug:
// We still want to Really Destroy bullets so they don't take up
// unnecessary memory.
void
blot_bullet(Am_Object bullet)
{
  bullet.Set(Am_VISIBLE, false);
  bullet.Set(Am_TOP, bullet.Get(Am_TOP));
  bullet.Get_Owner().Remove_Part(bullet);
// we'd rather destroy it...
#if 0
  bullet.Destroy();
#endif
  // but there seems to be a bug with that

  num_bullets--;
}

void
blot_bomb(Am_Object bomb)
{
  bomb.Set(Am_VISIBLE, false);
  bomb.Set(Am_TOP, bomb.Get(Am_TOP));
  bomb.Get_Owner().Remove_Part(bomb);

// we'd rather destroy it...
#if 0
  bomb.Destroy();
#endif
  // but there seems to be a bug with that
}

Am_Define_Method(Am_Object_Method, void, bullet_done, (Am_Object cmd))
{
  Am_Object_Method(Am_Animator.Get(Am_DO_METHOD)).Call(cmd);
  blot_bullet(cmd.Get(Am_OPERATES_ON));
}

Am_Define_Method(Am_Object_Method, void, bomb_done, (Am_Object cmd))
{
  Am_Object_Method(Am_Animator.Get(Am_DO_METHOD)).Call(cmd);
  Am_Object bomb = cmd.Get(Am_OPERATES_ON);
}

Am_Define_Method(Am_Object_Method, void, drop_bomb, (Am_Object /*cmd*/))
{
  // Only drop a bomb 50% of the time we call this, for some variation.
  if (rand() >> 5 & 0x1)
    return; // test one of the middle bits
  int num_aliens = Aliens.Get(NUM_ALIENS);
  if (!num_aliens)
    return; // no one is left to shoot!

  // Not an even distribution, but it'll do.
  int alien_shooting = (rand() >> 4) % num_aliens + 1;
  Am_Value_List aliens = Aliens.Get(Am_GRAPHICAL_PARTS);
  Am_Object alien;
  for (aliens.Start(); !aliens.Last(); aliens.Next()) {
    alien = aliens.Get();
    //    if (alien.Get(Am_LINE_STYLE) != Am_No_Style)  // alien is not dead
    if (alien.Get(Am_IMAGE).Valid())
      alien_shooting--;
    if (alien_shooting)
      alien = Am_No_Object;
    else
      break;
  }
  if (alien == Am_No_Object) {
    cout << "Oops.\n";
    return;
  }
  // now alien contains the alien we're dealing with.
  Am_Object bomb_owner = Ship.Get_Owner();
  Am_Object new_bomb;
  int bomb_left = (int)alien.Get(Am_LEFT) + (int)alien.Get(Am_WIDTH) / 2;
  int bomb_top = (int)alien.Get(Am_TOP) + (int)alien.Get(Am_HEIGHT);
  Am_Translate_Coordinates(alien.Get_Owner(), bomb_left, bomb_top, bomb_owner,
                           bomb_left, bomb_top);
  bomb_owner.Add_Part(new_bomb = Bomb.Create("Live_Bomb")
                                     .Set(Am_LEFT, bomb_left)
                                     .Set(Am_TOP, bomb_top));
  // send the bomb downscreen
  new_bomb.Set(Am_TOP, 1000);
}

Am_Define_Method(Am_Object_Method, void, keyboard_controls, (Am_Object cmd))
{
  Am_Object inter = cmd.Get_Owner();
  Am_Input_Char c = inter.Get(Am_START_CHAR);
  switch (c.As_Char()) {
  case 'p': {
    int i = 0;
    while (true) { //pause, wait forever
      i++;
    }
  }
  case 'j': // left
    Ship.Set(Am_LEFT, 10);
    break;
  case 'l': // right
    Ship.Set(Am_LEFT, (int)Ship.Get_Owner().Get(Am_WIDTH) -
                          (int)Ship.Get(Am_WIDTH) - 10);
    break;
  case ' ':
  case 'i': // shoot
    if (num_bullets < MAX_BULLETS && (bool)Ship.Get(Am_VISIBLE)) {
      Ship.Get_Owner().Add_Part(
          Bullet.Create()
              .Set(Am_LEFT,
                   (int)Ship.Get(Am_LEFT) + ((int)Ship.Get(Am_WIDTH) / 2))
              .Set(Am_TOP, -(int)Bullet.Get(Am_HEIGHT)));
      num_bullets++;
    }
    break;
  case 'q':
    Am_Exit_Main_Event_Loop();
    break;
  case '?':
  case 'h':
    cout << "q = quit\n"
         << "p = pause (wait forever)\n"
         << "j = move left\n"
         << "l = move right\n"
         << "i or space = shoot\n"
         << endl
         << flush;
    break;
  default: // stop
    // stop the ship where it is
    Ship.Set(Am_LEFT, Ship.Get(Am_LEFT));
  }
}

Am_Define_Formula(int, top_of_ship)
{
  return (int)Am_Bottom_Is_Bottom_Of_Owner(self) - 10;
}

Am_Define_Formula(int, bomb_destroy_ship) // in the bomb, not its group
{
  if (!self.Get_Owner().Valid())
    return 0;
  if (!(bool)self.Get(Am_VISIBLE))
    return 0;

  int bomb_top = (int)self.Get(Am_TOP);
  int bomb_bottom = bomb_top + (int)self.Get(Am_HEIGHT);
  int bomb_left = (int)self.Get(Am_LEFT);
  int bomb_right = bomb_left + (int)self.Get(Am_WIDTH);
  int ship_left = (int)Ship.Get(Am_LEFT);
  int ship_top = (int)Ship.Get(Am_TOP);
  int ship_right = ship_left + (int)Ship.Get(Am_WIDTH);
  int ship_bottom = ship_top + (int)Ship.Get(Am_HEIGHT);
  if (((bomb_left >= ship_left && bomb_left <= ship_right) ||
       (bomb_right >= ship_left && bomb_right <= ship_right)) &&
      ((bomb_top <= ship_bottom && bomb_top >= ship_top) ||
       (bomb_bottom <= ship_bottom && bomb_bottom >= ship_top))) {
    Ship.Set(Am_VISIBLE, false);
    self.Set(Am_VISIBLE, false);
    blot_bomb(self);
    Am_Stop_Animator(alien_animator);
    Am_Stop_Animator(star_animator);
  }
  return 0;
}

Am_Define_Formula(int, destroy_alien)
{
  //  if I'm invisible, I've already destroyed something.
  if (!(bool)self.Get(Am_VISIBLE))
    return 0;

  Am_Value_List aliens = Aliens.Get(Am_GRAPHICAL_PARTS);
  int x_off = Aliens.Get(Am_LEFT);
  int y_off = Aliens.Get(Am_TOP);
  int bullet_top =
      self.Get(Am_TOP); // The ONLY Get: don't want to re-eval too often
  int bullet_bottom = bullet_top + (int)self.Get(Am_HEIGHT);
  int bullet_left = self.Get(Am_LEFT);
  int bullet_right = bullet_left + (int)self.Get(Am_WIDTH);
  int alien_left, alien_top, alien_right, alien_bottom;
  Am_Object owner = self.Get_Owner();
  Am_Object alien;
  for (aliens.Start(); !aliens.Last(); aliens.Next()) {
    alien = aliens.Get();
    if (!alien.Get(Am_IMAGE).Valid())
      continue;
    alien_left = (int)alien.Get(Am_LEFT) + x_off;
    alien_top = (int)alien.Get(Am_TOP) + y_off;
    alien_right = alien_left + (int)alien.Get(Am_WIDTH);
    alien_bottom = alien_top + (int)alien.Get(Am_HEIGHT);
    // We hit if any of the corners of the bullet are inside the alien
    if (((bullet_left >= alien_left && bullet_left <= alien_right) ||
         (bullet_right >= alien_left && bullet_right <= alien_right)) &&
        ((bullet_top <= alien_bottom && bullet_top >= alien_top) ||
         (bullet_bottom <= alien_bottom && bullet_bottom >= alien_top))) {
      alien.Set(Am_IMAGE, NULL);
      Aliens.Set(NUM_ALIENS, (int)Aliens.Get(NUM_ALIENS) - 1);

      blot_bullet(self);
      return 0;
    }
  }
  return 1;
}

Am_Define_No_Self_Formula(int, left_of_aliens)
{
  Am_Value_List aliens = Aliens.Get(Am_GRAPHICAL_PARTS);
  Am_Object this_alien;
  for (aliens.Start(); !aliens.Last(); aliens.Next()) {
    this_alien = aliens.Get();
    if (this_alien.Get(Am_IMAGE).Valid())
      return 10 - (int)this_alien.Get(Am_LEFT);
  }
  return 0;
}

Am_Define_No_Self_Formula(int, right_of_aliens)
{
  Am_Value_List aliens = Aliens.Get(Am_GRAPHICAL_PARTS);
  Am_Object this_alien;
  int right = Aliens.Get_Owner().Get(Am_WIDTH);
  for (aliens.End(); !aliens.First(); aliens.Prev()) {
    this_alien = aliens.Get();
    if (this_alien.Get(Am_IMAGE).Valid()) {
      return right - (int)this_alien.Get(Am_LEFT) -
             (int)this_alien.Get(Am_WIDTH) - 10;
    }
  }
  return right;
}

Am_Define_Formula(int, alien_position)
{
  return ((int)self.Get(DIRECTION) > 0) ? left_of_aliens(self)
                                        : right_of_aliens(self);
}

// Makes the ships move faster when there are fewer of them.
Am_Define_Formula(int, alien_velocity)
{
  int n = self.Get_Object(Am_OPERATES_ON).Get(NUM_ALIENS);
  if (n == 0)
    n = 1;
  return 50 + 900 / n;
}

Am_Define_Method(Am_Object_Method, void, move_aliens_down, (Am_Object))
{
  Aliens.Set(DIRECTION, -(int)Aliens.Get(DIRECTION));
  Aliens.Set(Am_TOP, (int)Aliens.Get(Am_TOP) + 10);
  drop_bomb.Call(Am_No_Object);
}

int
main(void)
{
  Am_Initialize();
  Am_Bitmap.Set(Am_FILL_STYLE, Am_Black);
  DESTROY = Am_Register_Slot_Name("Destroy something");
  NUM_ALIENS = Am_Register_Slot_Name("Number of aliens left on the screen");
  DIRECTION = Am_Register_Slot_Name("DIRECTION");

  const char *fn = Am_Merge_Pathname("images/stars.gif");
  Am_Image_Array star_image(fn);
  delete[] fn;
  Am_Object star_bitmap = Am_Bitmap.Create().Set(Am_IMAGE, star_image);
  fn = Am_Merge_Pathname("images/spaceship.gif");
  Am_Image_Array alien_image(fn);
  delete[] fn;
  fn = Am_Merge_Pathname("images/goodship.gif");
  Am_Image_Array ship_image(fn);
  delete[] fn;
  fn = Am_Merge_Pathname("images/bomb.gif");
  Am_Image_Array bomb_image(fn);
  delete[] fn;

  Am_Object Alien = Am_Bitmap.Create()
                        .Set(Am_IMAGE, alien_image)
                        .Set(Am_LINE_STYLE, Am_Green);
  Alien.Set(Am_WIDTH, Alien.Get(Am_WIDTH)).Set(Am_HEIGHT, Alien.Get(Am_HEIGHT));

  Aliens = Am_Map.Create("Aliens");

  my_window =
      Am_Window.Create("My Window")
          .Set(Am_LEFT, 50)
          .Set(Am_TOP, 50)
          .Set(Am_WIDTH, 500)
          .Set(Am_HEIGHT, 500)
          .Set(Am_FILL_STYLE, Am_Motif_Gray)
          .Add_Part(
              Am_Group.Create()
                  .Set(Am_LEFT, 30)
                  .Set(Am_TOP, 30)
                  .Set(Am_WIDTH, 440)
                  .Set(Am_HEIGHT, 440)
                  .Add_Part(
                      Am_Map.Create()
                          .Set(Am_ITEM_PROTOTYPE, star_bitmap)
                          .Set(Am_ITEMS, 5)
                          .Set(Am_LAYOUT, Am_Vertical_Layout)
                          .Set(Am_TOP,
                               Am_Animate_With(
                                   star_animator =
                                       Am_Animator.Create()
                                           .Set(Am_VALUE_1,
                                                -(int)star_bitmap.Get(
                                                    Am_HEIGHT) -
                                                    1)
                                           .Set(Am_VALUE_2, 0)
                                           .Set(Am_VELOCITY, 27)
                                           .Set_Part(Am_COMMAND,
                                                     Am_Animation_Wrap_Command
                                                         .Create())))

#if 0
			.Add_Part(star_animator = Am_Animation_Interactor.Create()
				  .Set(Am_REPEAT_DELAY, Am_Time(20))
				  .Add_Part(Am_COMMAND, Am_Animator_Animation_Command.Create()
					    .Set(Am_SLOT_TO_ANIMATE, Am_TOP)
					    .Set(Am_TIME_FOR_ANIMATION, Am_Time(10000))
					    .Set(Am_VALUE_1, -(int)star_bitmap.Get(Am_HEIGHT)-1)
					    .Set(Am_VALUE_2, 0)
					    .Set(Am_ANIMATE_END_ACTION, Am_ANIMATION_WRAP)
					    )
				  )
#endif
                          )
                  .Add_Part(
                      Ship = Am_Bitmap.Create()
                                 .Set(Am_IMAGE, ship_image)
                                 .Set(Am_LEFT, 250)
                                 .Set(Am_LEFT,
                                      Am_Animate_With(Am_Animator.Create().Set(
                                          Am_VELOCITY, 100) // pixels per sec
                                                      ))
                                 .Set(Am_TOP, top_of_ship))
                  .Add_Part(
                      Aliens.Add(NUM_ALIENS, START_NUM_ALIENS)
                          .Set(Am_TOP, 30)
                          .Add(DIRECTION, 1)
                          .Set(Am_LEFT, 0)
                          .Set(Am_LEFT, alien_position)
                          .Set(Am_LEFT,
                               Am_Animate_With(
                                   alien_animator =
                                       Am_Animator.Create()
                                           .Set(Am_VELOCITY, alien_velocity)
                                           .Set_Part(Am_COMMAND,
                                                     Am_Command.Create().Set(
                                                         Am_DO_METHOD,
                                                         move_aliens_down))))
                          .Set(Am_ITEM_PROTOTYPE, Alien)
                          .Set(Am_ITEMS, START_NUM_ALIENS)
                          .Set(Am_LAYOUT, Am_Vertical_Layout)
                          .Set(Am_MAX_RANK, 6)
                          .Set(Am_H_SPACING, 15)
                          .Set(Am_V_SPACING, 15)
#if 0
			.Add_Part(alien_animator = Am_Animation_Interactor.Create()
				  .Set(Am_REPEAT_DELAY, Am_Time(20))
				  .Add_Part(Am_COMMAND, Am_Constant_Velocity_Command.Create("foo24")
					    .Set (Am_SLOT_TO_ANIMATE, Am_LEFT)
					    .Set (Am_VALUE_1, left_of_aliens)
					    .Set (Am_VALUE_2, right_of_aliens)
					    .Set (Am_VELOCITY, alien_velocity)
					    .Set (Am_ANIMATE_END_METHOD, move_aliens_down)
					    .Set (Am_ANIMATE_END_ACTION, Am_ANIMATION_BOUNCE)
					    )
				  )
#endif
                          ))
          .Add_Part(Am_One_Shot_Interactor.Create()
                        .Set(Am_START_WHEN, "ANY_KEYBOARD")
                        .Set(Am_START_WHERE_TEST, true)
                        .Get_Object(Am_COMMAND)
                        .Set(Am_DO_METHOD, keyboard_controls)
                        .Get_Owner()
                        .Set(Am_PRIORITY, 200));

  Bullet = Am_Rectangle.Create("Bullet")
               .Set(Am_WIDTH, 4)
               .Set(Am_HEIGHT, 16)
               .Set(Am_FILL_STYLE, Am_Red)
               .Set(Am_LINE_STYLE, Am_No_Style)
               .Add(DESTROY, destroy_alien);
  Bullet.Set(Am_TOP, (int)Ship.Get(Am_TOP) - (int)Bullet.Get(Am_HEIGHT))
      .Set(Am_TOP, Am_Animate_With(Am_Animator.Create("Bullet Animator")
                                       .Set(Am_VELOCITY, 300)
                                       .Set(Am_DO_METHOD, bullet_done)));

  Bomb = Am_Bitmap.Create("Bomb")
             .Add(DESTROY, bomb_destroy_ship)
             .Set(Am_IMAGE, bomb_image)
             .Set(Am_TOP, 0)
             .Set(Am_TOP, Am_Animate_With(Am_Animator.Create("Bomb Animator")
                                              .Set(Am_VELOCITY, 200)
                                              .Set(Am_DO_METHOD, bomb_done)));

  // Problems with testing to see if the goodguy's ship is destroyed
  // or not: Where do I put the destructo test?  Best I can think of
  // is a method which is periodically run which tests whether
  // anything bad intersects with the ship.

  Am_Screen.Add_Part(my_window);

  Am_Start_Animator(star_animator);
  //Am_Start_Animator (alien_animator);
  Am_Main_Event_Loop();
  Am_Cleanup();
  return 0;
}
