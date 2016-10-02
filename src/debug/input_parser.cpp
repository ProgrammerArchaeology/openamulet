/* ************************************************************************
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

/* This file contains a procedure to parse strings into Amulet types.
   It is used by the inspector and also by scripting.

   Designed and implemented by Brad Myers
*/

#include <am_inc.h>

#include <amulet/widgets_advanced.h>
#include <amulet/registry.h>

//returns true if input can be parsed as a wanted_type.
//If returns true, then output_value is set with the actual value.
bool
Am_Parse_Input_As_Type(Am_String input, Am_Value_Type wanted_type,
                       Am_Value &output_value)
{
  Am_ID_Tag type_class = Am_Type_Class(wanted_type);
  switch (type_class) {
  case Am_SIMPLE_TYPE:
    switch (wanted_type) {
    case Am_BOOL:
      if (input == "true") {
        output_value = true;
        return true;
      } else if (input == "false") {
        output_value = false;
        return true;
      }
    //else fall through to the INT case
    case Am_INT:
    case Am_LONG:
    case Am_FLOAT:
    case Am_DOUBLE: {
      //Am_String_To_Long_Or_Float is defined in widgets_advanced.h
      bool ok = Am_String_To_Long_Or_Float(input, output_value);
      if (ok) {
        if (output_value.type != wanted_type) {
          if (output_value.type == Am_FLOAT) {
            if (wanted_type == Am_DOUBLE) {
              output_value = (double)output_value;
              return true;
            }
            return false;                            //want int, got float
          } else if (output_value.type == Am_LONG) { //user provided a long
            if (wanted_type == Am_BOOL)
              output_value = output_value.Valid();
            else if (wanted_type == Am_INT)
              output_value = (int)output_value;
            else if (wanted_type == Am_FLOAT)
              output_value = (float)output_value;
            else if (wanted_type == Am_DOUBLE)
              output_value = (double)output_value;
            else
              Am_ERROR("Unknown wanted type " << wanted_type);
          } else
            Am_ERROR("Unexpected return type " << output_value.type << " on "
                                               << output_value);
        }
        return true;
      } else
        return false;
    }
    case Am_STRING:
      output_value = input;
      return true;
    case Am_CHAR: {
      const char *s = input;
      if (strlen(s) == 1) {
        char c = s[0];
        output_value = c;
        return true;
      } else
        return false;
    }
    default:
      return false; //Unsupported simple type (Am_VOIDPTR, Am_PROC, etc.)
    }
    break;
  case Am_METHOD:
  case Am_WRAPPER:
    if (wanted_type == Am_Value_List::Type_ID()) {
      std::cout << "** Can't edit value lists yet.  Sorry.\n";
      return false;
    } else {
      char *s = input;
      if (!strcmp(s, "(0L)") || !strcmp(s, "(NULL)") || !strcmp(s, "0") ||
          !strncmp(s, "Am_No_", 6)) {
        output_value.type = wanted_type;
        output_value.value.wrapper_value = nullptr;
        return true;
      }
      if (wanted_type == Am_OBJECT) {
        if (s[0] == '<') { //remove the <> around the name
          unsigned int i;
          for (i = 1; i < (strlen(s) - 1); i++)
            s[i - 1] = s[i];
          s[i] = 0;
        }
      }
      const Am_Registered_Type *the_item =
          Am_Get_Named_Item((const char *)input);
      if (the_item != nullptr && the_item->ID() == wanted_type) {
        if (type_class == Am_METHOD)
          output_value = (Am_Method_Wrapper *)the_item;
        else {
          Am_Wrapper *wrap = (Am_Wrapper *)the_item;
          wrap->Note_Reference();
          output_value = wrap;
        }
        return true;
      }
      return false;
    }
  case Am_ENUM: {
    Am_Type_Support *ws = Am_Find_Support(wanted_type);
    if (ws) {
      output_value = ws->From_String(input);
      if (output_value.Exists())
        return true;
    }
    return false;
  }
  default:
    std::cerr << "** Unknown type class " << type_class << " on type "
              << wanted_type << std::endl
              << std::flush;
    return false;
  } //end switch
}

bool
Am_Parse_Input_As_List_Of_Type(Am_String input, Am_Value_Type wanted_type,
                               Am_Value &output_value)
{
  const char *s = input;
  if (strncmp(s, "LIST(", 5))
    return false; //not a list
  char *real_part = strchr(s, '[');
  real_part++;
  std::cout << "parse " << input << " found real part '" << real_part << "'\n"
            << std::flush;
  char *token = strtok(real_part, ",]");
  std::cout << "first token `" << token << "'\n" << std::flush;
  Am_String token_str;
  Am_Value_List return_values;
  bool ok;
  Am_Value sub_value;
  while (token) {
    if (token[0] == ' ')
      token++;
    token_str = token;
    ok = Am_Parse_Input_As_Type(token_str, wanted_type, sub_value);
    std::cout << "parsing " << token_str << " ok " << ok << " value "
              << sub_value;
    if (!ok)
      return false;
    return_values.Add(sub_value);
    token = strtok(nullptr, ",]");
    std::cout << "next token is `" << token << "'\n" << std::flush;
  }
  output_value = return_values;
  return true;
}
