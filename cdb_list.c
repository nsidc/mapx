/*========================================================================
 * cdb_list - list segment index of cdb file
 *========================================================================*/
static const char cdb_list_c_rcsid[] = "$Header: /tmp_mnt/FILES/mapx/cdb_list.c,v 1.3 2003-06-23 15:50:05 haran Exp $";

#include <stdio.h>
#include <math.h>
#include <define.h>
#include <cdb.h>

#define usage "usage: cdb_list [-v] file.cdb ... \n"

char *id_cdb_list(void)
{
  return((char *)cdb_list_c_rcsid);
}

int main(int argc, char *argv[])
{
  register char *option;
  cdb_class *this;
  int verbose = FALSE;

/*
 *	get command line options
 */
  while (--argc > 0 && (*++argv)[0] == '-')
  { for (option = argv[0]+1; *option != '\0'; option++)
    { switch (*option)
      { case 'v':
	  verbose = TRUE;
	  break;
	default:
	  fprintf(stderr, "invalid option %c\n", *option);
	  error_exit(usage);
      }
    }
  }

/*
 *	process command line arguments
 */
  if (argc < 1) error_exit(usage);

  for (; argc > 0; --argc, ++argv)
  { this = init_cdb(*argv);
    if (this == NULL) continue;
    list_cdb(this, verbose);
    free_cdb(this);
  }

  exit(EXIT_SUCCESS);
}


