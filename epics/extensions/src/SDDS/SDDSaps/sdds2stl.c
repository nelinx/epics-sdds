#include "mdb.h"
#include "SDDS.h"
#include "scan.h"
#if defined(_WIN32)
#include <io.h>
#include <fcntl.h>
#endif

float float_reverse_bytes ( float x );

#define SET_PIPE 1
#define N_OPTIONS 2
 
char *option[N_OPTIONS] = {
  "pipe"
};

char *USAGE = "sdds2stl [-pipe[=input]] [<inputFile>] [<outputFile>]\n\
Converts an SDDS file to binary STL file.\n\
Program by Robert Soliday. ("__DATE__")\n";

int main(int argc, char **argv) {
  char *input=NULL, *output=NULL;
  SDDS_DATASET SDDS_dataset;
  SCANNED_ARG *scanned;
  long iArg;
  unsigned long pipeFlags=0;

  int i;
  int32_t rows=0, row, bsrows;
  float *normalVector[3];
  float *vertex1[3];
  float *vertex2[3];
  float *vertex3[3];
  short attribute = 0;
  FILE *fd;
  int32_t bigEndianMachine = 0;
  float temp;


  SDDS_RegisterProgramName(argv[0]);
  argc = scanargs(&scanned, argc, argv);
  if (argc<2) {
    fprintf(stderr, "%s", USAGE);
    return(1);
  }

  for (iArg=1; iArg<argc; iArg++) {
    if (scanned[iArg].arg_type==OPTION) {
      switch  (match_string(scanned[iArg].list[0], option, N_OPTIONS, 0)) {
      case SET_PIPE:
        if (!processPipeOption(scanned[iArg].list+1, scanned[iArg].n_items-1, &pipeFlags)) {
          fprintf(stderr, "sdds2stl: invalid -pipe syntax\n");
          return(1);
        }
        break;
      default:
        fprintf(stderr, "sdds2stl: invalid option seen\n%s", USAGE);
        return(1);
      }
    } else {
      if (!input)
        SDDS_CopyString(&input,scanned[iArg].list[0]);
      else if (!output)
        SDDS_CopyString(&output,scanned[iArg].list[0]);
      else {
        fprintf(stderr, "sdds2stl: too many filenames\n%s", USAGE);
        return(1);
      }
    }
  }
  if (pipeFlags&USE_STDIN) {
    processFilenames("sdds2tiff", &input, &output, USE_STDIN, 1, NULL);
  }

  if (!SDDS_InitializeInput(&SDDS_dataset, input)) {
    SDDS_PrintErrors(stderr, SDDS_VERBOSE_PrintErrors|SDDS_EXIT_PrintErrors);
  }

  if ((SDDS_CheckColumn(&SDDS_dataset, "NormalVectorX", NULL, SDDS_ANY_NUMERIC_TYPE, NULL)) != SDDS_CHECK_OKAY) {
    fprintf(stderr, "sdds2stl: NormalVectorX column not found.\n");
    return(1);
  } 
  if ((SDDS_CheckColumn(&SDDS_dataset, "NormalVectorY", NULL, SDDS_ANY_NUMERIC_TYPE, NULL)) != SDDS_CHECK_OKAY) {
    fprintf(stderr, "sdds2stl: NormalVectorX column not found.\n");
    return(1);
  } 
  if ((SDDS_CheckColumn(&SDDS_dataset, "NormalVectorZ", NULL, SDDS_ANY_NUMERIC_TYPE, NULL)) != SDDS_CHECK_OKAY) {
    fprintf(stderr, "sdds2stl: NormalVectorX column not found.\n");
    return(1);
  } 
  if ((SDDS_CheckColumn(&SDDS_dataset, "Vertex1X", NULL, SDDS_ANY_NUMERIC_TYPE, NULL)) != SDDS_CHECK_OKAY) {
    fprintf(stderr, "sdds2stl: Vertex1X column not found.\n");
    return(1);
  } 
  if ((SDDS_CheckColumn(&SDDS_dataset, "Vertex1Y", NULL, SDDS_ANY_NUMERIC_TYPE, NULL)) != SDDS_CHECK_OKAY) {
    fprintf(stderr, "sdds2stl: Vertex1Y column not found.\n");
    return(1);
  } 
  if ((SDDS_CheckColumn(&SDDS_dataset, "Vertex1Z", NULL, SDDS_ANY_NUMERIC_TYPE, NULL)) != SDDS_CHECK_OKAY) {
    fprintf(stderr, "sdds2stl: Vertex1Z column not found.\n");
    return(1);
  } 
  if ((SDDS_CheckColumn(&SDDS_dataset, "Vertex2X", NULL, SDDS_ANY_NUMERIC_TYPE, NULL)) != SDDS_CHECK_OKAY) {
    fprintf(stderr, "sdds2stl: Vertex2X column not found.\n");
    return(1);
  } 
  if ((SDDS_CheckColumn(&SDDS_dataset, "Vertex2Y", NULL, SDDS_ANY_NUMERIC_TYPE, NULL)) != SDDS_CHECK_OKAY) {
    fprintf(stderr, "sdds2stl: Vertex2Y column not found.\n");
    return(1);
  } 
  if ((SDDS_CheckColumn(&SDDS_dataset, "Vertex2Z", NULL, SDDS_ANY_NUMERIC_TYPE, NULL)) != SDDS_CHECK_OKAY) {
    fprintf(stderr, "sdds2stl: Vertex2Z column not found.\n");
    return(1);
  } 
  if ((SDDS_CheckColumn(&SDDS_dataset, "Vertex3X", NULL, SDDS_ANY_NUMERIC_TYPE, NULL)) != SDDS_CHECK_OKAY) {
    fprintf(stderr, "sdds2stl: Vertex3X column not found.\n");
    return(1);
  } 
  if ((SDDS_CheckColumn(&SDDS_dataset, "Vertex3Y", NULL, SDDS_ANY_NUMERIC_TYPE, NULL)) != SDDS_CHECK_OKAY) {
    fprintf(stderr, "sdds2stl: Vertex3Y column not found.\n");
    return(1);
  } 
  if ((SDDS_CheckColumn(&SDDS_dataset, "Vertex3Z", NULL, SDDS_ANY_NUMERIC_TYPE, NULL)) != SDDS_CHECK_OKAY) {
    fprintf(stderr, "sdds2stl: Vertex3Z column not found.\n");
    return(1);
  } 


  if (SDDS_ReadTable(&SDDS_dataset) <= 0) {
    fprintf(stderr, "sdds2stl: Unable to read SDDS page.\n");
    return(1);
  }
  rows = SDDS_RowCount(&SDDS_dataset);
  
  for ( i = 0; i < 3; i++ ) {
    normalVector[i] = malloc(sizeof(*(normalVector[i]))*rows);
    vertex1[i] = malloc(sizeof(*(vertex1[i]))*rows);
    vertex2[i] = malloc(sizeof(*(vertex2[i]))*rows);
    vertex3[i] = malloc(sizeof(*(vertex3[i]))*rows);
  }
  normalVector[0] = SDDS_GetColumnInFloats(&SDDS_dataset, "NormalVectorX");
  normalVector[1] = SDDS_GetColumnInFloats(&SDDS_dataset, "NormalVectorY");
  normalVector[2] = SDDS_GetColumnInFloats(&SDDS_dataset, "NormalVectorZ");
  vertex1[0] = SDDS_GetColumnInFloats(&SDDS_dataset, "Vertex1X");
  vertex1[1] = SDDS_GetColumnInFloats(&SDDS_dataset, "Vertex1Y");
  vertex1[2] = SDDS_GetColumnInFloats(&SDDS_dataset, "Vertex1Z");
  vertex2[0] = SDDS_GetColumnInFloats(&SDDS_dataset, "Vertex2X");
  vertex2[1] = SDDS_GetColumnInFloats(&SDDS_dataset, "Vertex2Y");
  vertex2[2] = SDDS_GetColumnInFloats(&SDDS_dataset, "Vertex2Z");
  vertex3[0] = SDDS_GetColumnInFloats(&SDDS_dataset, "Vertex3X");
  vertex3[1] = SDDS_GetColumnInFloats(&SDDS_dataset, "Vertex3Y");
  vertex3[2] = SDDS_GetColumnInFloats(&SDDS_dataset, "Vertex3Z");

  if (!SDDS_Terminate(&SDDS_dataset))
    SDDS_PrintErrors(stderr, SDDS_VERBOSE_PrintErrors|SDDS_EXIT_PrintErrors);

  
  if (!output) {
#if defined(_WIN32)
    if (_setmode(_fileno(stdout), _O_BINARY) == -1) {
      fprintf(stderr, "error: unable to set stdout to binary mode\n");
      exit(1);
    }      
#endif
    fd = stdout;
  } else {
    fd = fopen(output, "wb");
  }

  bigEndianMachine = SDDS_IsBigEndianMachine();

  fprintf(fd, "STL BINARY FILE CREATED BY SDDS2STL --------------------------------------------");
  if (bigEndianMachine) {
    bsrows = rows;
    SDDS_SwapLong((int32_t*)&bsrows);
    fwrite(&bsrows, sizeof(int32_t), 1, fd);
    for ( row = 0; row < rows; row++ ) {
      for ( i = 0; i < 3; i++ ) {
        temp = float_reverse_bytes(normalVector[i][row]);
        fwrite(&temp, sizeof(float), 1, fd);
      }
      for ( i = 0; i < 3; i++ ) {
        temp = float_reverse_bytes(vertex1[i][row]);
        fwrite(&temp, sizeof(float), 1, fd);
      }
      for ( i = 0; i < 3; i++ ) {
        temp = float_reverse_bytes(vertex2[i][row]);
        fwrite(&temp, sizeof(float), 1, fd);
      }
      for ( i = 0; i < 3; i++ ) {
        temp = float_reverse_bytes(vertex3[i][row]);
        fwrite(&temp, sizeof(float), 1, fd);
      }
      fwrite(&attribute, sizeof(short), 1, fd);
    }
  } else {
    fwrite(&rows, sizeof(int32_t), 1, fd);
    for ( row = 0; row < rows; row++ ) {
      for ( i = 0; i < 3; i++ ) {
        fwrite(&(normalVector[i][row]), sizeof(float), 1, fd);
      }
      for ( i = 0; i < 3; i++ ) {
        fwrite(&(vertex1[i][row]), sizeof(float), 1, fd);
      }
      for ( i = 0; i < 3; i++ ) {
        fwrite(&(vertex2[i][row]), sizeof(float), 1, fd);
      }
      for ( i = 0; i < 3; i++ ) {
        fwrite(&(vertex3[i][row]), sizeof(float), 1, fd);
      }
      fwrite(&attribute, sizeof(short), 1, fd);
    }
  }
  fclose(fd);

  return(1);
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
