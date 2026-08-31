#include <config.h>
#include <libintl.h>
#include <locale.h>
#include <gtkmm.h>
#include "lw.h"

int max_vector_width;

int
main (int argc, char *argv[])
{
  setlocale (LC_ALL, "");
  bindtextdomain ("lw", "/usr/share/locale");
  textdomain ("lw");

  auto lw = Lw::create ();
  lw->run (argc, argv);
  lw->cleanup ();
  return Lw::exit_code;
}
