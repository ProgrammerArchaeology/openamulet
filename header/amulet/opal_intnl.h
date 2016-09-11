class Am_Formula;
class Am_Draw_Method;
class Am_Point_In_Method;
class Am_Translate_Coordinates_Method;
class Am_Window_ToDo;
class Am_Slot;

extern Am_Formula compute_depth;
extern Am_Formula pass_window;
extern Am_Draw_Method generic_mask;
extern Am_Point_In_Method generic_point_in_obj;
extern Am_Point_In_Or_Self_Method generic_point_in_part;
extern Am_Point_In_Or_Self_Method generic_point_in_leaf;
extern Am_Translate_Coordinates_Method generic_translate_coordinates;

extern Am_Window_ToDo* Window_ToDo_Head;
extern Am_Window_ToDo* Window_ToDo_Tail;


void graphics_create (Am_Object gr_object);
void graphics_destroy (Am_Object self);
void graphics_change_owner (Am_Object self, Am_Object, Am_Object);
void graphics_repaint (Am_Slot first_invalidated);
void graphics_move (Am_Slot first_invalidated);

const char* Am_Check_Int_Type (const Am_Value& value);


