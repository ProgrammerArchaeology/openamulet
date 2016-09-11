#ifndef OPAL_STANDARD_OBJ_H
#define OPAL_STANDARD_OBJ_H

class Am_Style;
class Am_Font;
class Am_Object;

// Predefined Colors
_OA_DL_IMPORT extern Am_Style Am_Red;
_OA_DL_IMPORT extern Am_Style Am_Green;
_OA_DL_IMPORT extern Am_Style Am_Blue;
_OA_DL_IMPORT extern Am_Style Am_Yellow;
_OA_DL_IMPORT extern Am_Style Am_Purple;
_OA_DL_IMPORT extern Am_Style Am_Cyan;
_OA_DL_IMPORT extern Am_Style Am_Orange;
_OA_DL_IMPORT extern Am_Style Am_Black;
_OA_DL_IMPORT extern Am_Style Am_Dark_Gray;
_OA_DL_IMPORT extern Am_Style Am_Gray;
_OA_DL_IMPORT extern Am_Style Am_Light_Gray;
_OA_DL_IMPORT extern Am_Style Am_White;
_OA_DL_IMPORT extern Am_Style Am_Amulet_Purple;

_OA_DL_IMPORT extern Am_Style Am_Motif_Gray;
_OA_DL_IMPORT extern Am_Style Am_Motif_Light_Gray;
_OA_DL_IMPORT extern Am_Style Am_Motif_Blue;
_OA_DL_IMPORT extern Am_Style Am_Motif_Light_Blue;
_OA_DL_IMPORT extern Am_Style Am_Motif_Green;
_OA_DL_IMPORT extern Am_Style Am_Motif_Light_Green;
_OA_DL_IMPORT extern Am_Style Am_Motif_Orange;
_OA_DL_IMPORT extern Am_Style Am_Motif_Light_Orange;

// Default color used for all widgets etc.
_OA_DL_IMPORT extern Am_Style Am_Default_Color;

// Predefined line-styles
_OA_DL_IMPORT extern Am_Style Am_Thin_Line;
_OA_DL_IMPORT extern Am_Style Am_Line_0;
_OA_DL_IMPORT extern Am_Style Am_Line_1;
_OA_DL_IMPORT extern Am_Style Am_Line_2;
_OA_DL_IMPORT extern Am_Style Am_Line_4;
_OA_DL_IMPORT extern Am_Style Am_Line_8;
_OA_DL_IMPORT extern Am_Style Am_Dotted_Line;
_OA_DL_IMPORT extern Am_Style Am_Dashed_Line;

_OA_DL_IMPORT extern Am_Style Am_Gray_Stipple;
_OA_DL_IMPORT extern Am_Style Am_Opaque_Gray_Stipple;
_OA_DL_IMPORT extern Am_Style Am_Light_Gray_Stipple;
_OA_DL_IMPORT extern Am_Style Am_Dark_Gray_Stipple;
_OA_DL_IMPORT extern Am_Style Am_Diamond_Stipple;
_OA_DL_IMPORT extern Am_Style Am_Opaque_Diamond_Stipple;

_OA_DL_IMPORT extern Am_Font Am_Default_Font;
_OA_DL_IMPORT extern Am_Font Am_Japanese_Font;  // Japanese standard font

// The root object.  This object is instanced to create other objects.  The
// Am_Root_Object has no slots and no parts.
_OA_DL_IMPORT extern Am_Object Am_Root_Object;

_OA_DL_IMPORT extern Am_Object Am_Screen;

_OA_DL_IMPORT extern Am_Object Am_Graphical_Object;
_OA_DL_IMPORT extern Am_Object Am_Window;
_OA_DL_IMPORT extern Am_Object Am_Rectangle;
_OA_DL_IMPORT extern Am_Object Am_Roundtangle;
_OA_DL_IMPORT extern Am_Object Am_Line;
_OA_DL_IMPORT extern Am_Object Am_Arrow_Line;
_OA_DL_IMPORT extern Am_Object Am_Polygon;
_OA_DL_IMPORT extern Am_Object Am_Arc;
_OA_DL_IMPORT extern Am_Object Am_Text;
_OA_DL_IMPORT extern Am_Object Am_Hidden_Text; //for passwords
_OA_DL_IMPORT extern Am_Object Am_Bitmap;

_OA_DL_IMPORT extern Am_Object Am_Group;
_OA_DL_IMPORT extern Am_Object Am_Map;
_OA_DL_IMPORT extern Am_Object Am_Resize_Parts_Group;
_OA_DL_IMPORT extern Am_Object Am_Fade_Group;
_OA_DL_IMPORT extern Am_Object Am_Flip_Book_Group;

_OA_DL_IMPORT extern Am_Object Am_Aggregate;  // Usually you want to create instances of
                                // Am_Group or Am_Map

#endif /* OPAL_STANDARD_OBJ_H */
