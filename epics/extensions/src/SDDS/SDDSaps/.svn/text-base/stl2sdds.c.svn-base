#include "mdb.h"
#include "SDDS.h"
#include "scan.h"

float float_reverse_bytes ( float x );
int leqi ( char* string1, char* string2 );

#define SET_ASCII 0
#define SET_BINARY 1
#define SET_PIPE 2
#define N_OPTIONS 3
 
char *option[N_OPTIONS] = {
  "ascii", "binary", "pipe"
};

char *USAGE = "stl2sdds [<inputFile>] [<outputFile>] [-pipe[=in][,out]]\n\
[-ascii | -binary]\n\n\
pipe            SDDS toolkit pipe option.\n\
ascii           Requests SDDS ASCII output.  Default is binary.\n\
binary          Requests SDDS BINARY output.\n\
Converts STL files to SDDS.\n\
Program by Robert Soliday. ("__DATE__")\n";

#define LINE_MAX_LEN 256
#define FALSE 0
#define TRUE 1


int main(int argc, char **argv) {
  char *inputFile=NULL, *outputFile=NULL;
  SDDS_DATASET SDDSout;
  SCANNED_ARG *scanned;
  long iArg;
  long ascii=0, stlascii=0;
  unsigned long pipeFlags=0;

  int i, n=0, solid=0;
  char c;
  char init[7];
  short attribute = 0;
  int32_t face_num = 0, iface;
  int32_t bigEndianMachine = 0;
  int64_t bytes_num = 0, text_num=0;
  float *normalVector[3];
  float *vertex1[3];
  float *vertex2[3];
  float *vertex3[3];
  FILE *fd;
  char input[LINE_MAX_LEN];
  char *next;
  char token[LINE_MAX_LEN];
  int width;


  SDDS_RegisterProgramName(argv[0]);
  argc = scanargs(&scanned, argc, argv);
  if (argc<2) {
    fprintf(stderr, "%s", USAGE);
    return(1);
  }

  for (iArg=1; iArg<argc; iArg++) {
    if (scanned[iArg].arg_type==OPTION) {
      switch (match_string(scanned[iArg].list[0], option, N_OPTIONS, 0)) {
      case SET_ASCII:
        ascii = 1;
        break;
      case SET_BINARY:
        ascii = 0;
        break;
      case SET_PIPE:
        if (!processPipeOption(scanned[iArg].list+1, scanned[iArg].n_items-1, &pipeFlags)) {
          fprintf(stderr, "invalid -pipe syntax\n");
          return(1);
        }
        break;
      default:
        fprintf(stderr, "stl2sdds: invalid option seen\n%s", USAGE);
        return(1);
      }
    } else {
      if (!inputFile)
        SDDS_CopyString(&inputFile,scanned[iArg].list[0]);
      else if (!outputFile)
        SDDS_CopyString(&outputFile,scanned[iArg].list[0]);
      else {
        fprintf(stderr, "stl2sdds: too many filenames\n%s", USAGE);
        return(1);
      }
    }
  }

  processFilenames("stl2sdds", &inputFile, &outputFile, pipeFlags, 0, NULL);

  if (inputFile) {
    if (!fexists(inputFile)) {
      fprintf(stderr, "input file not found\n");
      return(1);
    }
    if (!(fd = fopen(inputFile, "rb"))) {
      fprintf(stderr, "problem opening input file\n");
      return(1);
    }
  } else {
#if defined(_WIN32)
      if (_setmode(_fileno(stdin), _O_BINARY) == -1) {
	fprintf(stderr, "error: unable to set stdin to binary mode\n");
	return(1);
      }      
#endif
    fd = stdin;
  }


  bigEndianMachine = SDDS_IsBigEndianMachine();
  

  /*
    Check for ASCII or BINARY STL type
  */
  bytes_num += fread(&init, 1, 6, fd);
  init[6] = 0;
  if (strcasecmp("solid ", init) == 0) {
    stlascii = 1;
  }
  if (stlascii == 0) {
    /* 
       80 byte Header. (74 bytes remaining)
    */
    for ( i = 0; i < 74; i++ ) {
      c = fgetc ( fd );
      bytes_num++;
    }
    
    /*
      Number of faces.
    */
    bytes_num += fread(&face_num, 1, sizeof(int32_t), fd);
    if (bigEndianMachine) {
      SDDS_SwapLong((int32_t*)&face_num);
    }
    
    for ( i = 0; i < 3; i++ ) {
      normalVector[i] = malloc(sizeof(*(normalVector[i]))*face_num);
      vertex1[i] = malloc(sizeof(*(vertex1[i]))*face_num);
      vertex2[i] = malloc(sizeof(*(vertex2[i]))*face_num);
      vertex3[i] = malloc(sizeof(*(vertex3[i]))*face_num);
    }
    
    /*
      For each (triangular) face,
      components of normal vector,
      coordinates of three vertices,
      2 byte "attribute".
    */
    for ( iface = 0; iface < face_num; iface++ ) {
      for ( i = 0; i < 3; i++ ) {
        bytes_num  += fread(&(normalVector[i][iface]), 1, sizeof(float), fd);
        if (bigEndianMachine) {
          normalVector[i][iface] = float_reverse_bytes(normalVector[i][iface]);
        }
      }
      for ( i = 0; i < 3; i++ ) {
        bytes_num  += fread(&(vertex1[i][iface]), 1, sizeof(float), fd);
        if (bigEndianMachine) {
          vertex1[i][iface] = float_reverse_bytes(vertex1[i][iface]);
        }
      }
      for ( i = 0; i < 3; i++ ) {
        bytes_num  += fread(&(vertex2[i][iface]), 1, sizeof(float), fd);
        if (bigEndianMachine) {
          vertex2[i][iface] = float_reverse_bytes(vertex2[i][iface]);
        }
      }
      for ( i = 0; i < 3; i++ ) {
        bytes_num  += fread(&(vertex3[i][iface]), 1, sizeof(float), fd);
        if (bigEndianMachine) {
          vertex3[i][iface] = float_reverse_bytes(vertex3[i][iface]);
        }
      }
      bytes_num  += fread(&(attribute), 1, sizeof(short int), fd);
    }
  } else {
    for ( i = 0; i < 3; i++ ) {
      normalVector[i] = NULL;
      vertex1[i] = NULL;
      vertex2[i] = NULL;
      vertex3[i] = NULL;
    }
    fseek(fd, 0, SEEK_SET);
    while (fgets(input, LINE_MAX_LEN, fd) != NULL) {
      text_num = text_num + 1;
      for ( next = input; *next != '\0' && isspace(*next); next++ ) {
      }
      if ( *next == '\0' || *next == '#' || *next == '!' || *next == '$' ) {
        continue;
      }
      sscanf ( next, "%s%n", token, &width );
      next = next + width;

      if ( leqi ( token, "facet" ) == TRUE ) {

        if (n==0) {
          n++;
          for ( i = 0; i < 3; i++ ) {
            normalVector[i] = malloc(sizeof(*(normalVector[i]))*n);
            vertex1[i] = malloc(sizeof(*(vertex1[i]))*n);
            vertex2[i] = malloc(sizeof(*(vertex2[i]))*n);
            vertex3[i] = malloc(sizeof(*(vertex3[i]))*n);
          }
        } else {
          n++;
          for ( i = 0; i < 3; i++ ) {
            normalVector[i] = realloc(normalVector[i], sizeof(*(normalVector[i]))*n);
            vertex1[i] = realloc(vertex1[i], sizeof(*(vertex1[i]))*n);
            vertex2[i] = realloc(vertex2[i], sizeof(*(vertex2[i]))*n);
            vertex3[i] = realloc(vertex3[i], sizeof(*(vertex3[i]))*n);
          }
        }

        sscanf ( next, "%*s %e %e %e", 
                 &(normalVector[0][n-1]), 
                 &(normalVector[1][n-1]), 
                 &(normalVector[2][n-1]));

        fgets(input, LINE_MAX_LEN, fd);
        fgets(input, LINE_MAX_LEN, fd);
        text_num = text_num + 2;
        
        sscanf ( next, "%*s %e %e %e", 
                 &(vertex1[0][n-1]), 
                 &(vertex1[1][n-1]), 
                 &(vertex1[2][n-1]));

        fgets(input, LINE_MAX_LEN, fd);
        text_num = text_num + 1;
        
        sscanf ( next, "%*s %e %e %e", 
                 &(vertex2[0][n-1]), 
                 &(vertex2[1][n-1]), 
                 &(vertex2[2][n-1]));

        fgets(input, LINE_MAX_LEN, fd);
        text_num = text_num + 1;
        
        sscanf ( next, "%*s %e %e %e", 
                 &(vertex3[0][n-1]), 
                 &(vertex3[1][n-1]), 
                 &(vertex3[2][n-1]));

        fgets(input, LINE_MAX_LEN, fd);
        fgets(input, LINE_MAX_LEN, fd);
        text_num = text_num + 2;
      } else if ( leqi ( token, "color" ) == TRUE ) {
        fprintf(stderr, "stl2sdds: color field seen in STL file ignored.\n");
      } else if ( leqi ( token, "solid" ) == TRUE ) {
        solid++;
        if (solid > 1) {
          fprintf(stderr, "stl2sdds: More than solid field seen in STL file.\n");
          return(1);
        }
      } else if ( leqi ( token, "endsolid" ) == TRUE ) {
      } else {
        fprintf(stderr, "stl2sdds: Unrecognized first word on line. %s\n", token);
        return(1);
      }
    }
    face_num = n;
  }


  if (!SDDS_InitializeOutput(&SDDSout, ascii?SDDS_ASCII:SDDS_BINARY,1, NULL, NULL, outputFile)) {
    SDDS_PrintErrors(stderr, SDDS_VERBOSE_PrintErrors);
    return(1);
  }
  if (!ascii) {
    SDDSout.layout.data_mode.column_major = 1;
  }
  if (!SDDS_DefineSimpleColumn(&SDDSout, "NormalVectorX", NULL, SDDS_FLOAT)) {
    SDDS_PrintErrors(stderr, SDDS_VERBOSE_PrintErrors);
    return(1);
  }
  if (!SDDS_DefineSimpleColumn(&SDDSout, "NormalVectorY", NULL, SDDS_FLOAT)) {
    SDDS_PrintErrors(stderr, SDDS_VERBOSE_PrintErrors);
    return(1);
  }
  if (!SDDS_DefineSimpleColumn(&SDDSout, "NormalVectorZ", NULL, SDDS_FLOAT)) {
    SDDS_PrintErrors(stderr, SDDS_VERBOSE_PrintErrors);
    return(1);
  }
  if (!SDDS_DefineSimpleColumn(&SDDSout, "Vertex1X", NULL, SDDS_FLOAT)) {
    SDDS_PrintErrors(stderr, SDDS_VERBOSE_PrintErrors);
    return(1);
  }
  if (!SDDS_DefineSimpleColumn(&SDDSout, "Vertex1Y", NULL, SDDS_FLOAT)) {
    SDDS_PrintErrors(stderr, SDDS_VERBOSE_PrintErrors);
    return(1);
  }
  if (!SDDS_DefineSimpleColumn(&SDDSout, "Vertex1Z", NULL, SDDS_FLOAT)) {
    SDDS_PrintErrors(stderr, SDDS_VERBOSE_PrintErrors);
    return(1);
  }
  if (!SDDS_DefineSimpleColumn(&SDDSout, "Vertex2X", NULL, SDDS_FLOAT)) {
    SDDS_PrintErrors(stderr, SDDS_VERBOSE_PrintErrors);
    return(1);
  }
  if (!SDDS_DefineSimpleColumn(&SDDSout, "Vertex2Y", NULL, SDDS_FLOAT)) {
    SDDS_PrintErrors(stderr, SDDS_VERBOSE_PrintErrors);
    return(1);
  }
  if (!SDDS_DefineSimpleColumn(&SDDSout, "Vertex2Z", NULL, SDDS_FLOAT)) {
    SDDS_PrintErrors(stderr, SDDS_VERBOSE_PrintErrors);
    return(1);
  }
  if (!SDDS_DefineSimpleColumn(&SDDSout, "Vertex3X", NULL, SDDS_FLOAT)) {
    SDDS_PrintErrors(stderr, SDDS_VERBOSE_PrintErrors);
    return(1);
  }
  if (!SDDS_DefineSimpleColumn(&SDDSout, "Vertex3Y", NULL, SDDS_FLOAT)) {
    SDDS_PrintErrors(stderr, SDDS_VERBOSE_PrintErrors);
    return(1);
  }
  if (!SDDS_DefineSimpleColumn(&SDDSout, "Vertex3Z", NULL, SDDS_FLOAT)) {
    SDDS_PrintErrors(stderr, SDDS_VERBOSE_PrintErrors);
    return(1);
  }
  if (!SDDS_WriteLayout(&SDDSout)) {
    SDDS_PrintErrors(stderr, SDDS_VERBOSE_PrintErrors);
    return(1);
  }
  if (!SDDS_StartTable(&SDDSout, face_num)) {
    SDDS_PrintErrors(stderr, SDDS_VERBOSE_PrintErrors);
    return(1);
  }
  if (!SDDS_SetColumnFromFloats(&SDDSout, SDDS_SET_BY_NAME, normalVector[0], face_num, "NormalVectorX")) {
    SDDS_PrintErrors(stderr, SDDS_VERBOSE_PrintErrors);
    return(1);
  }
  if (!SDDS_SetColumnFromFloats(&SDDSout, SDDS_SET_BY_NAME, normalVector[1], face_num, "NormalVectorY")) {
    SDDS_PrintErrors(stderr, SDDS_VERBOSE_PrintErrors);
    return(1);
  }
  if (!SDDS_SetColumnFromFloats(&SDDSout, SDDS_SET_BY_NAME, normalVector[2], face_num, "NormalVectorZ")) {
    SDDS_PrintErrors(stderr, SDDS_VERBOSE_PrintErrors);
    return(1);
  }
  if (!SDDS_SetColumnFromFloats(&SDDSout, SDDS_SET_BY_NAME, vertex1[0], face_num, "Vertex1X")) {
    SDDS_PrintErrors(stderr, SDDS_VERBOSE_PrintErrors);
    return(1);
  }
  if (!SDDS_SetColumnFromFloats(&SDDSout, SDDS_SET_BY_NAME, vertex1[1], face_num, "Vertex1Y")) {
    SDDS_PrintErrors(stderr, SDDS_VERBOSE_PrintErrors);
    return(1);
  }
  if (!SDDS_SetColumnFromFloats(&SDDSout, SDDS_SET_BY_NAME, vertex1[2], face_num, "Vertex1Z")) {
    SDDS_PrintErrors(stderr, SDDS_VERBOSE_PrintErrors);
    return(1);
  }
  if (!SDDS_SetColumnFromFloats(&SDDSout, SDDS_SET_BY_NAME, vertex2[0], face_num, "Vertex2X")) {
    SDDS_PrintErrors(stderr, SDDS_VERBOSE_PrintErrors);
    return(1);
  }
  if (!SDDS_SetColumnFromFloats(&SDDSout, SDDS_SET_BY_NAME, vertex2[1], face_num, "Vertex2Y")) {
    SDDS_PrintErrors(stderr, SDDS_VERBOSE_PrintErrors);
    return(1);
  }
  if (!SDDS_SetColumnFromFloats(&SDDSout, SDDS_SET_BY_NAME, vertex2[2], face_num, "Vertex2Z")) {
    SDDS_PrintErrors(stderr, SDDS_VERBOSE_PrintErrors);
    return(1);
  }
  if (!SDDS_SetColumnFromFloats(&SDDSout, SDDS_SET_BY_NAME, vertex3[0], face_num, "Vertex3X")) {
    SDDS_PrintErrors(stderr, SDDS_VERBOSE_PrintErrors);
    return(1);
  }
  if (!SDDS_SetColumnFromFloats(&SDDSout, SDDS_SET_BY_NAME, vertex3[1], face_num, "Vertex3Y")) {
    SDDS_PrintErrors(stderr, SDDS_VERBOSE_PrintErrors);
    return(1);
  }
  if (!SDDS_SetColumnFromFloats(&SDDSout, SDDS_SET_BY_NAME, vertex3[2], face_num, "Vertex3Z")) {
    SDDS_PrintErrors(stderr, SDDS_VERBOSE_PrintErrors);
    return(1);
  }

  if (!SDDS_WriteTable(&SDDSout)) {
    SDDS_PrintErrors(stderr, SDDS_VERBOSE_PrintErrors);
    return(1);
  }
  if (!SDDS_Terminate(&SDDSout)) {
    SDDS_PrintErrors(stderr, SDDS_VERBOSE_PrintErrors);
    return(1);
  }

  free_scanargs(&scanned, argc);



  for ( i = 0; i < 3; i++ ) {
    free(normalVector[i]);
    free(vertex1[i]);
    free(vertex2[i]);
    free(vertex3[i]);
  }
 
  return(0);

}

float float_reverse_bytes ( float x ) {
  char c;
  union {
    float yfloat;
    char ychar[4];
  } y;

  y.yfloat = x;
  
  c = y.ychar[0];
  y.ychar[0] = y.ychar[3];
  y.ychar[3] = c;

  c = y.ychar[1];
  y.ychar[1] = y.ychar[2];
  y.ychar[2] = c;

  return ( y.yfloat );
}

int leqi ( char* string1, char* string2 ) {
  int i;
  int nchar;
  int nchar1;
  int nchar2;

  nchar1 = strlen ( string1 );
  nchar2 = strlen ( string2 );

  if ( nchar1 < nchar2 ) {
    nchar = nchar1;
  }
  else {
    nchar = nchar2;
  }
/*
  The strings are not equal if they differ over their common length.
*/
  for ( i = 0; i < nchar; i++ ) {

    if ( toupper ( string1[i] ) != toupper ( string2[i] ) ) {
      return FALSE;
    }
  }
/*
  The strings are not equal if the longer one includes nonblanks
  in the tail.
*/
  if ( nchar1 > nchar ) {
    for ( i = nchar; i < nchar1; i++ ) {
      if ( string1[i] != ' ' ) {
        return FALSE;
      }
    } 
  }
  else if ( nchar2 > nchar ) {
    for ( i = nchar; i < nchar2; i++ ) {
      if ( string2[i] != ' ' ) {
        return FALSE;
      }
    } 
  }
  return TRUE;
}
