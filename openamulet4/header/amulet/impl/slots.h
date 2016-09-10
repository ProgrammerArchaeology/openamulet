#ifndef SLOTS_LIST_H
#define SLOTS_LIST_H

enum Am_Standard_Slot_Keys {
  //////
  // Graphical Object slots -- Opal
  //////
  Am_LEFT = 100, Am_TOP, Am_WIDTH, Am_HEIGHT,
  Am_WINDOW, Am_VISIBLE,
  Am_FILL_STYLE, Am_LINE_STYLE,

  // Windows, Screens
  Am_TITLE, Am_ICON_TITLE,
  Am_MAX_WIDTH, Am_MAX_HEIGHT, Am_MIN_WIDTH, Am_MIN_HEIGHT,
  Am_USE_MAX_WIDTH, Am_USE_MAX_HEIGHT, Am_USE_MIN_WIDTH, Am_USE_MIN_HEIGHT,
  Am_ICONIFIED, Am_QUERY_POSITION, Am_QUERY_SIZE, Am_OMIT_TITLE_BAR,
  Am_LEFT_BORDER_WIDTH, Am_TOP_BORDER_WIDTH, Am_RIGHT_BORDER_WIDTH,
  Am_BOTTOM_BORDER_WIDTH,
  Am_CURSOR, Am_SAVE_UNDER, Am_CLIP_CHILDREN, Am_CLIP, Am_GRAPHICAL_PARTS,
  Am_FLIP_BOOK_PARTS,
  Am_PRETEND_TO_BE_LEAF, Am_RANK,
  Am_OWNER_DEPTH, // Num owners from this obj up to window
  Am_IS_COLOR, // boolean for Am_Window and Am_Screen: color screen?
  Am_OFFSCREEN_DRAWONABLE, // Am_Screen: store an offscreen drawonable
  Am_DESTROY_WINDOW_METHOD, //Am_Window: user asked for destroy
  Am_DOUBLE_BUFFER, // Am_Window: double buffered? bool.

  // Lines
  Am_X1, Am_Y1, Am_X2, Am_Y2, Am_DIRECTIONAL,

  // Arrow Lines
  Am_HEAD_LENGTH, Am_HEAD_WIDTH,
  Am_TAIL_LENGTH, Am_TAIL_WIDTH,

  // Layout directions
  Am_LAYOUT, Am_H_SPACING, Am_V_SPACING, Am_H_ALIGN, Am_V_ALIGN,
  Am_FIXED_WIDTH, Am_FIXED_HEIGHT, Am_INDENT, Am_MAX_RANK, Am_MAX_SIZE,
  Am_LEFT_OFFSET, Am_TOP_OFFSET,

  // For layout constraints like Am_Fill_To_Right_Of_Owner
  Am_RIGHT_OFFSET, Am_BOTTOM_OFFSET,

  // Am_Map
  Am_ITEMS, Am_ITEM, Am_ITEM_METHOD, Am_ITEM_PROTOTYPE,

  Am_HIT_THRESHOLD, //how close need to be to count as a hit

  // Arcs
  Am_ANGLE1, Am_ANGLE2,

  // Roundtangle
  Am_RADIUS, Am_DRAW_RADIUS,

  // Text
  Am_TEXT, Am_FONT, Am_CURSOR_INDEX, Am_CURSOR_OFFSET,
  Am_INVERT,

  // Bitmap
  Am_IMAGE, Am_DRAW_MONOCHROME,

  // Polygon
  Am_POINT_LIST,

  Am_CENTER_X_OBJ, //for center_x_is_center_of formula
  Am_CENTER_Y_OBJ, //for center_y_is_center_of formula
  Am_SELECT_OUTLINE_ONLY,
  Am_SELECT_FULL_INTERIOR,

  Am_SAVE_OBJECT_METHOD, // for storing a save method
  Am_SLOTS_TO_SAVE, //for the standard save/load method

  /////
  // Interactor Slots
  /////

  Am_START_WHEN, Am_START_WHERE_TEST,

  Am_RUNNING_WHERE_TEST,

  Am_ABORT_WHEN, Am_STOP_WHEN,

  Am_INTER_START_METHOD, Am_INTER_RUNNING_METHOD, Am_INTER_OUTSIDE_METHOD,
  Am_INTER_BACK_INSIDE_METHOD, Am_INTER_STOP_METHOD,
  Am_INTER_OUTSIDE_STOP_METHOD, Am_INTER_ABORT_METHOD,

  Am_COMMAND,
  Am_IMPLEMENTATION_COMMAND, //inter allocates this command for undo

  Am_START_DO_METHOD, Am_INTERIM_DO_METHOD, Am_ABORT_DO_METHOD, Am_DO_METHOD,

  Am_CONTINUOUS, Am_ACTIVE,

  Am_PRIORITY,

  Am_GRID_X, Am_GRID_Y, Am_GRID_ORIGIN_X, Am_GRID_ORIGIN_Y, Am_GRID_METHOD,

  Am_INTER_BEEP_ON_ABORT, //whether beeps when abort, default is true

  ///// slots of specific interactors

  // choice interactor
  Am_HOW_SET, Am_FIRST_ONE_ONLY, // whether like button or menu

  // move grow interactor
  Am_WHERE_ATTACH, Am_MINIMUM_WIDTH, Am_MINIMUM_HEIGHT, Am_MINIMUM_LENGTH,
  Am_GROWING, Am_AS_LINE,
  Am_FEEDBACK_OBJECT,
  Am_SET_SELECTED, // whether to set SELECTED and INTERIM_SELECTED of objs
  // NIY  Am_SLOTS_TO_SET, //which slots of obj to set

  // new points interactor
  Am_CREATE_NEW_OBJECT_METHOD, Am_HOW_MANY_POINTS, Am_FLIP_IF_CHANGE_SIDES,
  Am_ABORT_IF_TOO_SMALL,

  // text edit interactor
  Am_EDIT_TRANSLATION_TABLE, Am_TEXT_EDIT_METHOD,
  Am_PENDING_DELETE, //of the text object
  Am_WANT_PENDING_DELETE, //in the text_interactor
  Am_TEXT_CHECK_LEGAL_METHOD,
  Am_CREATE_IF_ZERO_LENGTH, // whether delete if edit to zero length

  //animation interactor
  Am_ANIMATION_METHOD,   //slot to put the method to be called by an
                         //object registered for a timer
  Am_REPEAT_DELAY,       //delay time before animation command called again
  Am_ELAPSED_TIME,       //slot in animation command objects set by anim inter
  Am_SLOT_TO_ANIMATE,    // slot or list of slots to set with values
  Am_TIME_FOR_ANIMATION, //length of time the animation should take
  Am_VALUES_FOR_SLOT_TO_ANIMATE, //list of values for slot
  Am_INDICES_FOR_VALUES_FOR_SLOT_TO_ANIMATE, // indirect indices into list
  Am_CURRENT_DIRECTION,  //which direction through the list
  Am_ANIMATE_END_ACTION, // stop, wrap, or bounce
  Am_ANIMATE_END_METHOD, // method to call before it wraps or bounces
  Am_ANIMATE_FORWARD,    // Internal slot: is bounce in forward or backward interp mode?

  ///// Animators for animated constraints
  Am_ANIM_INITIALIZE_METHOD,
  Am_ANIM_UPDATE_METHOD,
  Am_INTERRUPT_METHOD,
  Am_ANIM_CONSTRAINT,

  Am_RUNNING,           // boolean, true iff interpolator is interpolating
  Am_PATH_LENGTH,
  Am_INTERRUPTIBLE,
  Am_VALUES,            // list of values that arrived during animation
  Am_CURRENT_PHASE,     // current position (integer) on Am_VALUES list
  Am_VELOCITY,          // constant average velocity in units per second
                        // (or 0 if derived from duration)
  Am_DURATION,          // constant duration of animation as Am_Time
                        // (or 0 if derived from velocity)
  Am_CURRENT_VELOCITY,    // velocity of current animation run
  Am_CURRENT_DURATION,    // duration of current animation run
  Am_TIMING_MODE,       // stores slot key (Am_VELOCITY or Am_DURATION)
                        //    indicating which timing parameter is held constant
  Am_MIN_REPEAT_DELAY,  // minimum frame time
  Am_TIMING_FUNCTION,   // monotonic timing function (maps milliseconds to [0,1])
  Am_PATH_FUNCTION,     // path function (maps [0,1] to Am_VALUE)

  Am_INITIAL_DELAY,     // pre-animation delay for Am_Delayed_Timing

  Am_SHARPNESS_1,       // curvature of velocity curve in Am_Slow_In_Slow_Out
                        // timing.
  Am_SHARPNESS_2,       // SHARPNESS_1 applies to speedup away from VALUE_1,
                        // SHARPNESS_2 applies to slowdown approaching VALUE_2.
                        //   value near 1 ==> sharp velocity change
                        //   value > 100 ==> no velocity change (linear timing)
  Am_RGB_COLOR,         // Boolean whether to use RGB color path or HSV.

  // Slots for Am_Visible_By_Move_Grow_Animator
  Am_SUB_ANIMATOR,
  Am_VISIBLE_LOCATION,
  Am_INVISIBLE_LOCATION,
  Am_USE_FADING,

  // Slots for Am_Exaggerated_Animator
  Am_WINDUP_AMOUNT,     // distance (in pixels) to jump back before beginning
                        // motion
  Am_WINDUP_DELAY,      // Am_TIME to wait in windup state
  Am_WIGGLES,           // number of wiggles, counting both overshoots and
                        // undershoots.  I.e. 1 ==> one overshoot,
                        // 2 ==> one overshoot and one undershoot,
                        // 3 ==> two overshoots and one undershoot, etc.
  Am_WIGGLE_AMOUNT,     // distance (in pixels) of first overshoot (subsequent
                        //    wiggles decrease linearly)
  Am_WIGGLE_DELAY,      // Am_TIME between each wiggle at end of motion
  Am_EXAGGERATOR_STATE, // stage of Am_Exaggerated_Animator animation:
                        // windup, interpolation, wiggle0, wiggle1, ...

  // Slots for Am_Point_List_Animator
  Am_POINT_VALUE_1, Am_POINT_VALUE_2,  //modified old and new point lists

  // Slots for Am_Through_List_Animator
  Am_LIST_OF_VALUES,    // master list for interpolation
  Am_CURRENT_PATH,      // internal: current sublist of Am_LIST being
                        // interpolated

  ///// "Advanced" interactor slots sometimes used externally

  Am_MULTI_OWNERS,  // when multi-window interactor, put owner list here
  Am_MULTI_FEEDBACK_OWNERS, //multi-window feedback owners for move- and new-
  Am_ALL_WINDOWS,   // used for multi-window action when all windows must
                    // react but need multi-owners to hold an actual list.
  Am_CURRENT_OBJECT,  // set with object currently over
  Am_START_OBJECT,   //first object over
  Am_START_CHAR,    // first am_input_char that started the interactor

  Am_RUN_ALSO, // whether other interactors can also run
  Am_X_OFFSET, // move-grow X offset for points
  Am_Y_OFFSET, // move-grow Y offset for points
  Am_FIRST_X, //initial point where hit
  Am_FIRST_Y, //set for all interactors
  Am_INTERIM_X, // interim point where hit.
  Am_INTERIM_Y, // set for move grow interactors

  ///// Internal slots of Interactors (do not set or access)
  Am_CURRENT_STATE,
  Am_INTER_LIST, // list of interactors for a window
  Am_WHERE_HIT_WHERE_ATTACH,  // move-grow real attach for where-hit
  Am_DEPENDS_ON_PRIORITY, //formula in here maintains inter_list
  Am_DEPENDS_ON_RANK_DEPTH, //formula in here maintains inter_list
  Am_DEPENDS_ON_MULTI_OWNERS, //formula in here maintains inter_list
  Am_LAST_WINDOW,  //inter_list window
  Am_LAST_WINDOWS,  //inter_list window in more than one
  Am_WINDOW_WANT_MOVE_CNT, //number of want_move inters active on this window
  Am_REAL_RUN_ALSO,  // text_inter: copy of run_also
  Am_SET_COMMAND_OLD_OWNER, // formula: sets Am_SAVED_OLD_OWNER of
                           // cmd to the inter or widget
  Am_INITIAL_REF_OBJECT, //new_points
  //Gesture Interactor: features of a gesture
  Am_START_X, Am_START_Y, Am_INITIAL_SIN, Am_INITIAL_COS,
  Am_DX2, Am_DY2, Am_MAGSQ2, Am_END_X, Am_END_Y,
  Am_MIN_X, Am_MAX_X, Am_MIN_Y, Am_MAX_Y,
  Am_TOTAL_LENGTH, Am_TOTAL_ANGLE, Am_ABS_ANGLE, Am_SHARPNESS,
  Am_CLASSIFIER, Am_NONAMBIGUITY_PROB, Am_DIST_TO_MEAN,
  Am_MIN_NONAMBIGUITY_PROB, Am_MAX_DIST_TO_MEAN,
  Am_SUB_COMMAND,

  //////
  // Slots of Command Objects
  //////

  // Am_DO_METHOD,           // defined with interactors
  Am_UNDO_METHOD, Am_REDO_METHOD, Am_SELECTIVE_UNDO_METHOD,
  Am_SELECTIVE_REPEAT_SAME_METHOD, Am_SELECTIVE_REPEAT_ON_NEW_METHOD,
  Am_SELECTIVE_UNDO_ALLOWED, Am_SELECTIVE_REPEAT_SAME_ALLOWED,
  Am_SELECTIVE_REPEAT_NEW_ALLOWED,

  Am_SHORT_HELP,             // one-line or balloon help message
  Am_LONG_HELP,              // long help message to put into a window
  // Am_ACTIVE,              // defined with interactors.

  Am_LABEL,                  // used for menu's label and for "UNDO -"
  Am_SHORT_LABEL,            // used for menu's label and for "UNDO -"
  Am_ACCELERATOR,            // event to also execute this
  Am_ID,                     // if non-zero, identified the cmd instead of label

  Am_IMPLEMENTATION_PARENT,
  Am_COMPOSITE_PARENT,

  Am_CHECK_INACTIVE_COMMANDS, //tell interactor or command whether to look for
                              //inactive_commands or not
  Am_INACTIVE_COMMANDS,      //slot of an object containing cmd obj with
                             //slots for disabled commands
  Am_SLOT_FOR_THIS_COMMAND_INACTIVE, //Am_Active_If_Selection uses this to
                                     //decide what slot to check
  //slots of command obj in Am_INACTIVE_COMMANDS slot of obj to turn off cmds
  Am_MOVE_INACTIVE, Am_GROW_INACTIVE, Am_SELECT_INACTIVE,
  Am_TEXT_EDIT_INACTIVE,
  Am_SET_PROPERTY_INACTIVE, Am_CLEAR_INACTIVE,
  Am_COPY_INACTIVE, Am_CUT_INACTIVE, Am_DUPLICATE_INACTIVE,
  Am_TO_TOP_INACTIVE, Am_TO_BOTTOM_INACTIVE,
  Am_GROUP_INACTIVE, Am_UNGROUP_INACTIVE,

  // internal slots of command objects (do not set)
  Am_COMMAND_IS_ABORTING,
  Am_IMPLEMENTATION_CHILD,
  Am_COMPOSITE_CHILDREN,

  ///// slots of specific kinds of command objects

  Am_VALUE,                  // main value
  Am_INTERIM_VALUE,          // interim (running) value
  Am_OLD_INTERIM_VALUE,      // old interim (running) value for choice
  Am_OLD_VALUE,              // used to store old value for UNDO
  Am_OBJECT_MODIFIED,        // actual obj being modified
  Am_TOO_SMALL,              // new-point: true if object is too small
  Am_HAS_BEEN_UNDONE,        // new-point: true if new object removed
  Am_SAVED_OLD_OWNER,        // old owner (inter or widget for command)
  Am_SAVED_OLD_OBJECT_OWNER, // new_points: new_obj's old owner

  // slots set into the actual objects
  Am_SELECTED,               // set in objects that are selected
  Am_INTERIM_SELECTED,       // set in objects that mouse is over

  // Slots of Undo Objects (450-499)

  Am_UNDO_HANDLER,            // slot of Window or Appl holding undo obj
  Am_REGISTER_COMMAND,        // slot of undo obj holding func to accept
                              // new commands that have been executed
  Am_PERFORM_UNDO,            // slot of undo obj holding func for undo
  Am_PERFORM_REDO,            // slot of undo obj holding func for redo
  Am_UNDO_ALLOWED,            // if can undo now, contains the command
                              // object that will be undone, otherwise
                              // contains 0
  Am_REDO_ALLOWED, // command object to be redone or 0
  // Am_SELECTIVE_UNDO_METHOD
  // Am_SELECTIVE_REPEAT_SAME_METHOD
  // Am_SELECTIVE_REPEAT_ON_NEW_METHOD
  // Am_SELECTIVE_UNDO_ALLOWED
  // Am_SELECTIVE_REPEAT_SAME_ALLOWED
  // Am_SELECTIVE_REPEAT_NEW_ALLOWED

  // internal slots of undo objects
  Am_LAST_UNDONE_COMMAND,     //used by the multiple_undo_object

  /////
  ///// Widget Slots
  /////

  Am_FINAL_FEEDBACK_WANTED,   // whether show final feedback for buttons
  Am_KEY_SELECTED,            // true on the widget the keyboard is on
  Am_ACTIVE_2,                // only for interactive tools
  Am_ITEM_OFFSET,             // vertical and horizontal border around
                              // object or text in button
  Am_WIDGET_LOOK,             // motif, windows or mac
  Am_DEFAULT,                 // is widget default

  Am_VALUE_1,                 // scroll bar or animator value 1
  Am_VALUE_2,                 // scroll bar or animator value 2
  Am_SMALL_INCREMENT,         // scroll bar Incr on arrows
  Am_LARGE_INCREMENT,         // scroll bar page Incr
  Am_PERCENT_VISIBLE,         // percent of size that is visible

  Am_BOX_ON_LEFT,             // checkbox boolean where to put box
  Am_BOX_HEIGHT,              // checkbox box height
  Am_BOX_WIDTH,               // checkbox box width
  Am_TEXT_OFFSET,             // space on sides of text in menu item

  Am_H_SCROLL_BAR,            //scrolling group, has Horiz scroll bar?
  Am_V_SCROLL_BAR,            //scrolling group, has vert scroll bar?
  Am_H_SCROLL_BAR_ON_TOP,     //scrolling group
  Am_V_SCROLL_BAR_ON_LEFT,    //scrolling group
  Am_INNER_WIDTH,             //scrolling group: total size of inside
  Am_INNER_HEIGHT,            //scrolling group: total area to inside
  Am_INNER_FILL_STYLE,        //scrolling group: inside of clip window

  Am_LABEL_FONT,              //text_input: label's font
  Am_USE_ERROR_DIALOG,        //text_input: otherwise, beeps

  Am_OPERATES_ON,             //selection handles: group of sel objs
  Am_REGION_WHERE_TEST,       //selection handles: region test
  Am_MOVE_GROW_COMMAND,       //selection handles: cmd when move/grow
  Am_SELECTION_MOVE_CHARS,    //selection handles: list of chars for move sel
  Am_SELECTION_WIDGET,        //set into commands to tell
                              //where selections handles widget is
  Am_GET_WIDGET_VALUE_METHOD, //property command
  Am_GET_OBJECT_VALUE_METHOD, //property command, for old val: undo
  Am_SET_OBJECT_VALUE_METHOD, //property command
  Am_HANDLE_OPEN_SAVE_METHOD, //open and save commands
  Am_SLOT_FOR_VALUE,          //property command, for default methods
  Am_CLIPBOARD,               //cut,copy,paste command
  Am_CHANGED_SINCE_SAVED,     //all editing commands, count of # operations
  Am_DEFAULT_LOAD_SAVE_FILENAME, //open and save cmds; goes in sel-widget

  Am_MULTI_SELECTIONS,        // used to share selection widget areas
  Am_DROP_TARGET,             // holder for target command for drag-drop
  Am_DROP_TARGET_TEST,        // Am_Where_Method used for detecting drop targets
  Am_SELECT_CLOSEST_POINT_STYLE,   // style for handle closest to cursor
  Am_SELECT_CLOSEST_POINT_OBJ,     // object that closest point is in
  Am_SELECT_CLOSEST_POINT_WHERE,   // where_attach to say which point it is
  Am_SELECT_CLOSEST_POINT_METHOD,  // method to figure out which end

  Am_SPECIAL_HANDLE,      // supports multiple users
                          // for pic or polygon selection handles

  Am_VALID_INPUT,             // in a text input dialog box, is the text input valid

  Am_LIST_OF_TEXT_WIDGETS,    //Am_Tab_To_Next_Widget_Command: list of textwidgets
  Am_LABEL_LIST, // Am_Cycle_Value_Command, list of labels for the Am_LABEL slot
  Am_CHECKED_ITEM, // in a menu, is this item checked?

  ///// internal slots of widgets (do not change)

  Am_INTERACTOR,              // interactor that makes the widget work
  Am_STYLE_RECORD,            // colors computed based on foreground
  Am_REAL_STRING_OR_OBJ,
  Am_LABEL_OR_ID,             //formula that is id of cmd or label
  Am_ITEM_TO_COMMAND,         // formula to map ITEM to COMMAND slots
  Am_SCROLL_ARROW_DIRECTION,  // direction arrow points
  Am_SCROLL_AREA_MIN,         // top or left of scroll area of scrollbar
  Am_SCROLL_AREA_MAX,         // scrollbar max coordinate
  Am_SCROLL_AREA_SIZE,        // scrollbar indicator area
  Am_SCROLL_ARROW1,           // parts of scrollbar
  Am_SCROLL_ARROW2,           // parts of scrollbar
  Am_SCROLL_INDICATOR,        // parts of scrollbar
  Am_ARROW_INTERACTOR,        // scrollbar arrow inter
  Am_BACKGROUND_INTERACTOR,   // scrollbar page inter; also selection_handles
  Am_FIRST_TICK,              // scrollbar in initial delay of arrow or
                              // background interactor
  Am_H_SMALL_INCREMENT,       //scrolling group
  Am_H_LARGE_INCREMENT,       //scrolling group
  Am_V_SMALL_INCREMENT,       //scrolling group
  Am_V_LARGE_INCREMENT,       //scrolling group
  Am_CLIP_LEFT,               //scrolling group: top of clip area
  Am_CLIP_TOP,                //scrolling group: left of clip area
  Am_CLIP_WIDTH,              //scrolling group: size of clip area
  Am_CLIP_HEIGHT,             //scrolling group: size of clip area
  Am_H_SCROLLER,              //scrolling group: the H scroll object
  Am_V_SCROLLER,              //scrolling group: the V scroll object
  Am_ATTACHED_OBJECT,         //used to attach objects to buttons
  Am_ATTACHED_COMMAND,        //used to note attached command objects

  Am_LEAVE_ROOM_FOR_FRINGE,   // fringe is extra area around the button
  Am_MENU_BORDER,             // internal slot -- see button_widgets.cc
  Am_MENU_SELECTED_COLOR,     // selected menu button color -- only for windows
  Am_MENU_LINE_HEIGHT,        // internal slot -- see button_widgets.cc
  Am_MENU_ITEM_TOP_OFFSET,    // internal slot -- see button_widgets.cc
  Am_MENU_ITEM_BOT_OFFSET,    // internal slot -- see button_widgets.cc
  Am_MENU_ITEM_LEFT_OFFSET,   // internal slot -- see button_widgets.cc
  Am_MENU_ITEM_ACCEL_OFFSET,  // internal slot -- see button_widgets.cc
  Am_MENU_ITEM_RIGHT_OFFSET,  // internal slot -- see button_widgets.cc

  Am_REAL_WIDTH,              // for menu items: calculated width
  Am_REAL_HEIGHT,             //  of the button
  Am_SUB_MENU,                // for menu_bars: slot to hold sub-menu
  Am_FOR_ITEM,                // for menu_bars: sub-menu's main item
  Am_COMPUTE_INTER_VALUE,     //option buttons

  Am_WEB_CONSTRAINT,          //holds web constraint for button panels
  Am_TEXT_WIDGET_BOX_LEFT,    //text_input widget: left of entry box
  Am_TEXT_WIDGET_TEXT_OBJ,    //text_input widget: text value part
  Am_LAST_TEXT_WIDGET,        //Tab_To_Next_Widget_Command: last text_input

  Am_SELECTIONS_HANDLES_USE_LT_RB,  //selection handles
  Am_MOVE_INTERACTOR,         //selection handles
  Am_GROW_INTERACTOR,         //selection handles
  Am_SELECT_OUTSIDE_INTERACTOR,   //selection handles
  Am_MOVE_SEL_FOR_BACK_INTERACTOR,   //selection handles
  Am_LINE_FEEDBACK_OBJECT,    //selection handles
  Am_RECT_FEEDBACK_OBJECT,    //selection handles
  Am_FAKE_GROUP,              //selection handles
  Am_ACCELERATOR_STRING,      // string for accel char in a menu
  Am_ACCELERATOR_INTER,       // inter for accel in a window
  Am_ACCELERATOR_LIST,        // list for accel in a window
  Am_OBJECT_MODIFIED_PLACES,  //set into cmd with where
                              //OBJECT_MODIFIED used to be in group
  Am_OLD_CLIPBOARD_OBJECTS,   //set into cmd for old value
  Am_CREATED_GROUP,           //set into group if created by GROUP cmd
  Am_WIDGET_START_METHOD,     //method slot to for Am_Start_Widget
  Am_WIDGET_ABORT_METHOD,     //method slot to for Am_Abort_Widget
  Am_WIDGET_STOP_METHOD,      //method slot to for Am_Stop_Widget

  Am_DIALOG_BUTTONS,          //parts slots for the dialog box widgets
  Am_TEXT_WIDGET,
  Am_DIALOG_GROUP,
  Am_ICON_IN_ABOUT_DB,

  Am_TEXT_VIEWER,                  //holds the Am_MFText_Viewing_Context wrapper
  Am_LAST_USED_OLD_INTERIM_VALUE,  // used in scrolling_menu to hold last used
  Am_LAST_INITIAL_REF_OBJECT,      // used in scrolling_menu to hold old ref

  Am_USER_ID,			// supports multiple users
  Am_INTERNAL_USER_ID,	// supports multiple users
  Am_LAST_USER_ID		// supports multiple users

};
// Internal opal slot range (2000-9999)
#define Am_FIRST_OPAL_INTERNAL_SLOT 2000

// User Specified Slot Range (10000-29999)
#define Am_MINIMUM_USER_SLOT_KEY 10000
#define Am_MAXIMUM_USER_SLOT_KEY 29999

#endif //SLOTS_LIST_H
