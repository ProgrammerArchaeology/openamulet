/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

// gem_GIF.h
//
// GIF machine independent image format class.

#include <am_inc.h>
#include <amulet/gem_image.h>
#include <fstream>

// GIF format constants
#define HAS_CT 0x80      // has a colortable
#define INTERLACED 0x40  // interlaced image
#define TRANSPARENT 0x01 // transparent image
#define CTSIZE(x) (x & 0x07)

#define GIF_IMGSTART ','
#define GIF_EXTSTART '!'
#define GIF_TRAILER ';'
#define GIF_COMMENT 0xfe
#define GIF_GCE 0xf9 // graphics control extension

#define GIF_MAXCODE 12
#define GIF_TBLSIZE 4096

// Structures for GIF blocks

struct GIFHeader
{
  char signature[3]; // Should contain: "GIF"
  char version[3];   // "87a" or "89a"
};
#define GIF_SIG_LEN 6 /* GIF signature length */
#define GIF_SD_SIZE 7 /* GIF screen descriptor size */
#define GIF_ID_SIZE 9 /* GIF image descriptor size */

// constants for interpreting the Graphics Control Extension block:
// the first byte identifies the block as an extension GIF_EXTSTART
// the second byte identifies the block as a GCE GIF_CGE
// the third byte is the blocksize, always 4
// The next 4 bytes are the data of the extension
//   the first of these (at offset GIF_GCE_PACKED)
//       contains the transparent flag
//   the last (at offset GIF_GCE_COLORINDEX)
//       contains the index of the transparent color
#define GIF_GCE_SIZE 4   /* GIF graphics control extension block data size */
#define GIF_GCE_PACKED 0 /* Offset of Packed byte */
#define GIF_GCE_COLORINDEX 3 /* Offset of ColorIndex byte */

// Structure for GIF file information
// Eventually we should completely get rid of this structure, currently
// used to pass information between Am_GIF_Image routines.  The data
// is only needed temporarily, so it shouldn't be put in Am_GIF_Image.
class GIF_Load_Info
{
public:
  GIFHeader hdr;
  unsigned short interlaced;
  unsigned short pass; // For interlaced images
  unsigned short x, y; // position in image
};

class Am_GIF_Image : public Am_Generic_Image
{
public:
  static Am_Generic_Image *Create(const char *filename);

protected:
  int process_gif(std::ifstream &ifs, GIF_Load_Info &gli);
  void add_to_image(GIF_Load_Info &gli, unsigned short pixval);
  int load_gif_colormap(std::ifstream &ifs, int flags);
  void adjust_colormap();
};
