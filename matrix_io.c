/*======================================================================
 * matrix_io.c - 
 *   programs to read and write matrix data
 *   MATRIX_IO_ERROR is return in case of error, and 
 *   error message is written to stderr
 *
 *$Log: not supported by cvs2svn $
 *
 *======================================================================*/
#include <stdio.h>
#include <define.h>
#include <matrix.h>

static const char matrix_io_c_RCSID[]="$Header: /tmp_mnt/FILES/mapx/matrix_io.c,v 1.1 1997-02-27 21:54:29 brodzik Exp $";

#define ZERO_BYTES 0;

/*----------------------------------------------------------------------
 * read_byte4_data
 *
 *	input : file_name - complete data file name
 *		data - pointer to matrix
 *		rows,cols - size of data grid
 *
 *      output : data - returned with 2D matrix read from file_name
 *
 *      result : data is populated with matrix from file_name
 *               return value is number of bytes read, or
 *               0 in case of error, with error message written to stderr
 *
 *----------------------------------------------------------------------*/
size_t read_byte4_data (byte4 **data, char *file_name, int rows, int cols)
{
  int i;
  size_t nbytes, row_bytes;
  size_t read_bytes;
  FILE *fp;

  read_bytes=ZERO_BYTES;

  fp = fopen (file_name, "rb");
  if (fp == NULL) { 
    perror(file_name); 
    return ZERO_BYTES; 
  }

  row_bytes = cols*(sizeof **data);
  for (i=0; i < rows; i++)
    { nbytes = fread (data[i], 1, row_bytes, fp);
      if (nbytes != row_bytes) { 
	perror(file_name); 
	return ZERO_BYTES; 
      }
      read_bytes += nbytes;
    }

  fclose (fp);
  return read_bytes;
}

/*----------------------------------------------------------------------
 * read_matrix_data
 *
 *	input : file_name - complete data file name
 *		data - pointer to matrix
 *		rows,cols - size of data grid
 *              size - size (in bytes) of data element
 *
 *      output : data - returned with 2D matrix read from file_name
 *
 *      result : data is populated with matrix from file_name
 *               return value is number of bytes read, or
 *               0 in case of error, with error message written to stderr
 *
 *----------------------------------------------------------------------*/
size_t read_matrix_data (void **data, char *file_name, 
			 int rows, int cols, size_t size)
{
  int i;
  size_t nbytes, row_bytes;
  size_t read_bytes;
  FILE *fp;

  read_bytes=ZERO_BYTES;

  fp = fopen (file_name, "rb");
  if (fp == NULL) { 
    perror(file_name); 
    return ZERO_BYTES; 
  }

  row_bytes = cols*size;
  for (i=0; i < rows; i++)
    { nbytes = fread (data[i], 1, row_bytes, fp);
      if (nbytes != row_bytes) { 
	perror(file_name); 
	return ZERO_BYTES; 
      }
      read_bytes += nbytes;
    }

  fclose (fp);
  return read_bytes;
}


/*----------------------------------------------------------------------
 * write_byte4_data
 *
 *	input : file_name - complete data file name
 *		data - pointer to matrix
 *		rows,cols - size of data grid
 *
 *      output : n/a
 *
 *      result : data is written to file_name
 *               return value is number bytes written, or
 *               0 in case of error, with error message written to stderr
 *
 *----------------------------------------------------------------------*/
size_t write_byte4_data (char *file_name, byte4 **data, int rows, int cols)
{
  int i;
  size_t nbytes, row_bytes;
  size_t written_bytes;
  FILE *fp;

  written_bytes=ZERO_BYTES;

  fp = fopen (file_name, "wb");
  if (fp == NULL) { 
    perror(file_name); 
    return ZERO_BYTES; 
  }

  row_bytes = cols*(sizeof **data);
  for (i=0; i < rows; i++)
    { nbytes = fwrite (data[i], 1, row_bytes, fp);
      if (nbytes != row_bytes) { 
	perror(file_name); 
	return ZERO_BYTES; 
      }
      written_bytes += nbytes;
    }

  fclose (fp);
  return written_bytes;
}

/*----------------------------------------------------------------------
 * write_matrix_data
 *
 *	input : file_name - complete data file name
 *		data - pointer to matrix
 *		rows,cols - size of data grid
 *              size - size (in bytes) of data element
 *
 *      output : n/a
 *
 *      result : data is written to file_name
 *               return value is number bytes written, or
 *               0 in case of error, with error message written to stderr
 *
 *----------------------------------------------------------------------*/
size_t write_matrix_data (char *file_name, void **data, 
			  int rows, int cols, size_t size)
{
  int i;
  size_t nbytes, row_bytes;
  size_t written_bytes;
  FILE *fp;

  written_bytes=ZERO_BYTES;

  fp = fopen (file_name, "wb");
  if (fp == NULL) { 
    perror(file_name); 
    return ZERO_BYTES; 
  }

  row_bytes = cols*size;
  for (i=0; i < rows; i++)
    { nbytes = fwrite (data[i], 1, row_bytes, fp);
      if (nbytes != row_bytes) { 
	perror(file_name); 
	return ZERO_BYTES; 
      }
      written_bytes += nbytes;
    }

  fclose (fp);
  return written_bytes;
}





