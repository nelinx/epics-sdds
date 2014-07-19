/*************************************************************************\
* Copyright (c) 2002 The University of Chicago, as Operator of Argonne
* National Laboratory.
* Copyright (c) 2002 The Regents of the University of California, as
* Operator of Los Alamos National Laboratory.
* This file is distributed subject to a Software License Agreement found
* in the file LICENSE that is included with this distribution. 
\*************************************************************************/

/* program: sddsfft
 * purpose: SDDS-format hilbert tranfer program
 *
 * Hairong Shang, Louis Emery, Jan. 2014
 
 * the algorithm is: 1) first do complex fft of input signal,  
 * 2) remove the second half of the real and imaginary part  (make them zero).
 * this removes the negative frequencies.
 * 3) first half (n/2+1) arrays by 2, special points at f=0 (DC) and n/2 for even n are not multiplied by 2.
 * 4) do inverse fft of the modified complex fft arrays.
 *
 */
#include "mdb.h"
#include "SDDS.h"
#include "scan.h"
#include "fftpackC.h"
#include "SDDSutils.h"
#include <ctype.h>

#define SET_COLUMN 0
#define SET_MAJOR_ORDER 1
#define SET_PIPE 2
#define SET_UNWRAP_LIMIT 3
#define N_OPTIONS 4

char *option[N_OPTIONS] = {
  "columns", "majorOrder", "pipe", "unwrapLimit",
    };

static char *USAGE="sddsanalyticsignal [<inputfile>] [<outputfile>]\n\
[-pipe=[input][,output]] [-unwrapLimit=<value>] \n\
[-columns=<indep-variable>[,<depen-quantity>[,...]]] [-majorOder=row|column] \n\n\
-pipe            The standard SDDS Toolkit pipe option.\n\
-columns         Provides the name of the independent variable \n\
                 and the data to signal analyze.  <depen-quantity>\n\
                 entries may contain wildcards.\n\
-unwrapLimit     give the relative limit of the magnitude when unwrapping the phase, i.e., \n\
                 only unwrap the phase when the relative magnitude to the maximum is greater than this limit.\n\
-majorOrder      Specifies the output file written in row or column major order\n\n\
sddsanalyticsignal calculates the complex output of the real signal, which outputs following columns:\n\
                 Real<signal>, Imag<signal>, Mag<signal>, and Arg<signal> for the \n\
                 Real, Imaginary, Magnitude, and Phase of the signal respectively.\n\
Program by Hairong Shang and Louis Emery.  (This is version 1, January 2014.)\n";


long process_data(SDDS_DATASET *SDDSout, SDDS_DATASET *SDDSin, long rows,
                  char *depenQuantity, double unwrapLimit);

long create_fft_columns(SDDS_DATASET *SDDSout, SDDS_DATASET *SDDSin, char *origName);

int main(int argc, char **argv)
{
  int iArg;
  char *indepQuantity, **depenQuantity, **exclude=NULL;
  long depenQuantities, excludes=0;
  char *input, *output;
  long i, rows, readCode;
  int32_t  page=0;
  unsigned long flags, pipeFlags, majorOrderFlag;
  SCANNED_ARG *scanned;
  SDDS_DATASET SDDSin, SDDSout;
  short columnMajorOrder = -1;
  double unwrapLimit = 0;
  
  SDDS_RegisterProgramName(argv[0]);
  argc = scanargs(&scanned, argc, argv); 
  if (argc<3) {
    fprintf(stderr, "%s\n", USAGE);
    exit(1);
  }
  output = input = NULL;
  flags = pipeFlags  = 0;
  indepQuantity = NULL;
  depenQuantity = NULL;
  depenQuantities = 0;
  for (iArg=1; iArg<argc; iArg++) {
    if (scanned[iArg].arg_type==OPTION) {
      /* process options here */
      switch (match_string(scanned[iArg].list[0], option, N_OPTIONS, 0)) {
      case SET_COLUMN:
	if (indepQuantity)
	  SDDS_Bomb("only one -columns option may be given");
	if (scanned[iArg].n_items<2)
	  SDDS_Bomb("invalid -columns syntax");
	indepQuantity = scanned[iArg].list[1];
	if (scanned[iArg].n_items>=2) {
	  depenQuantity = tmalloc(sizeof(*depenQuantity)*(depenQuantities=scanned[iArg].n_items-2));
	  for (i=0; i<depenQuantities; i++)
	    depenQuantity[i] = scanned[iArg].list[i+2];
	}
	break;
      case SET_PIPE:
	if (!processPipeOption(scanned[iArg].list+1, scanned[iArg].n_items-1, &pipeFlags))
	  SDDS_Bomb("invalid -pipe syntax");
	break;
      case SET_MAJOR_ORDER:
        majorOrderFlag=0;
        scanned[iArg].n_items--;
        if (scanned[iArg].n_items>0 &&
	    (!scanItemList(&majorOrderFlag, scanned[iArg].list+1, &scanned[iArg].n_items, 0,
			   "row", -1, NULL, 0, SDDS_ROW_MAJOR_ORDER,
			   "column", -1, NULL, 0, SDDS_COLUMN_MAJOR_ORDER,
			   NULL)))
          SDDS_Bomb("invalid -majorOrder syntax/values");
        if (majorOrderFlag&SDDS_COLUMN_MAJOR_ORDER)
          columnMajorOrder=1;
        else if (majorOrderFlag&SDDS_ROW_MAJOR_ORDER)
          columnMajorOrder=0;
        break;
      case SET_UNWRAP_LIMIT:
        if ( scanned[iArg].n_items!=2)
          SDDS_Bomb("invalid -unwrapLimit syntax/values");
        if (!get_double(&unwrapLimit, scanned[iArg].list[1]))
          SDDS_Bomb("invalid -unwrapLimit syntax/values");
        break;
      default:
	fprintf(stderr, "error: unknown/ambiguous option: %s\n", 
		scanned[iArg].list[0]);
	exit(1);
	break;
      }
    }
    else {
      if (!input)
	input = scanned[iArg].list[0];
      else if (!output)
	output = scanned[iArg].list[0];
      else
	SDDS_Bomb("too many filenames seen");
    }
  }
  processFilenames("sddsanalyticsignal", &input, &output, pipeFlags, 0, NULL);
  
  if (!indepQuantity)
    SDDS_Bomb("supply the independent quantity name with the -columns option");
  
  if (!SDDS_InitializeInput(&SDDSin, input))
    SDDS_PrintErrors(stderr, SDDS_VERBOSE_PrintErrors|SDDS_EXIT_PrintErrors);
  
  if (SDDS_CheckColumn(&SDDSin, indepQuantity, NULL, SDDS_ANY_NUMERIC_TYPE, stderr)!=SDDS_CHECK_OKAY) 
    exit(1);

  
  excludes = appendToStringArray(&exclude, excludes, indepQuantity);
  
  if (!depenQuantities)
    depenQuantities = appendToStringArray(&depenQuantity, depenQuantities, "*"); 


  if ((depenQuantities=expandColumnPairNames(&SDDSin, &depenQuantity, NULL, depenQuantities, 
                                             exclude, excludes, FIND_NUMERIC_TYPE, 0))<=0) {
    SDDS_PrintErrors(stderr, SDDS_VERBOSE_PrintErrors|SDDS_EXIT_PrintErrors);
    SDDS_Bomb("No quantities selected to fft");
  }
  
#if 0
  fprintf(stderr, "%ld dependent quantities:\n", depenQuantities);
  for (i=0; i<depenQuantities; i++)
    fprintf(stderr, "  %s\n", depenQuantity[i]);
#endif
  if (!SDDS_InitializeCopy(&SDDSout, &SDDSin, output, "w"))
    SDDS_PrintErrors(stderr, SDDS_VERBOSE_PrintErrors|SDDS_EXIT_PrintErrors);
 
  if (columnMajorOrder!=-1)
    SDDSout.layout.data_mode.column_major = columnMajorOrder;
  else
    SDDSout.layout.data_mode.column_major = SDDSin.layout.data_mode.column_major;
  
  for (i=0; i<depenQuantities; i++) {
    if (!create_fft_columns(&SDDSout, &SDDSin, depenQuantity[i])) {
      SDDS_PrintErrors(stderr, SDDS_VERBOSE_PrintErrors|SDDS_EXIT_PrintErrors);
      fprintf(stderr, "Error creating output column for %s\n", depenQuantity[i]);
      exit(1);
    }
  }
  if (!SDDS_WriteLayout(&SDDSout))
    SDDS_PrintErrors(stderr, SDDS_VERBOSE_PrintErrors|SDDS_EXIT_PrintErrors);
  
  while ((readCode=SDDS_ReadPage(&SDDSin))>0) {
    page ++;
    if ((rows = SDDS_CountRowsOfInterest(&SDDSin))<0) 
      SDDS_PrintErrors(stderr, SDDS_VERBOSE_PrintErrors|SDDS_EXIT_PrintErrors);
    if (rows) {
      if (!SDDS_StartPage(&SDDSout, rows) ||
          !SDDS_CopyPage(&SDDSout, &SDDSin))
        SDDS_PrintErrors(stderr, SDDS_VERBOSE_PrintErrors|SDDS_EXIT_PrintErrors);
      for (i=0; i<depenQuantities; i++) 
        if (!process_data(&SDDSout, &SDDSin,  rows, depenQuantity[i], unwrapLimit)) {
          SDDS_PrintErrors(stderr, SDDS_VERBOSE_PrintErrors);
          exit(1);
        }
    }  else  {
      if (!SDDS_StartPage(&SDDSout, 0) || !SDDS_CopyParameters(&SDDSout, &SDDSin))
        SDDS_PrintErrors(stderr, SDDS_VERBOSE_PrintErrors|SDDS_EXIT_PrintErrors);
    }
    if (!SDDS_WritePage(&SDDSout))
      SDDS_PrintErrors(stderr, SDDS_VERBOSE_PrintErrors|SDDS_EXIT_PrintErrors);
  }
  
  if (!SDDS_Terminate(&SDDSin)) {
    SDDS_PrintErrors(stderr, SDDS_VERBOSE_PrintErrors);
    exit(1);
  }
  if (!SDDS_Terminate(&SDDSout)) {
    SDDS_PrintErrors(stderr, SDDS_VERBOSE_PrintErrors);
    exit(1);
  }
  /*free_scanargs(&scanned, argc);  */
  return 0;
}

long process_data(SDDS_DATASET *SDDSout, SDDS_DATASET *SDDSin, long rows,
                  char *depenQuantity, double unwrapLimit)
{
  long n_freq, i,  index1,  nhalf=0, fftrows;
  double *real, *imag,  *real_imag, *data, *magData, *arg, *unwrapped, delta, min, max, phase_correction;
  char s[256];
  
  if (!(data = SDDS_GetColumnInDoubles(SDDSin, depenQuantity)))
    return 0;
  
  fftrows = rows;
  
  /* compute forward FFT */
  real_imag = tmalloc(sizeof(double)*(2*fftrows+2));
  for (i=0; i<fftrows; i++) {
    real_imag[2*i] = data[i];
    real_imag[2*i+1] = 0;
  }
  complexFFT(real_imag, fftrows, 0);

  nhalf = fftrows/2 + 1;
  
  /*remove the second half points by setting them to zero */
  for (i=nhalf; i<fftrows; i++) {
    real_imag[2*i] = 0;
    real_imag[2*i+1] = 0;
  }
  
  /*multiply the first half by 2 except f=0 */
  for (i=1; i<nhalf; i++) {
    if (rows%2==0 && i==rows/2) 
      continue;
    real_imag[2*i] *=2;
    real_imag[2*i+1] *=2;
  }
  
  /*compute inverse fft */
  complexFFT(real_imag, fftrows, INVERSE_FFT);
  n_freq = fftrows;

  
  /* convert into amplitudes and frequencies, adding phase factor for t[0]!=0 */
  real = tmalloc(sizeof(double)*n_freq);
  imag = tmalloc(sizeof(double)*n_freq);
  magData = tmalloc(sizeof(double)*n_freq);
  arg = tmalloc(sizeof(*arg)*n_freq);
  unwrapped = tmalloc(sizeof(*arg)*n_freq);
  for (i=0; i<n_freq; i++) {
    real[i] = real_imag[2*i];
    imag[i] = real_imag[2*i+1];
    magData[i] = sqrt(sqr(real[i])+sqr(imag[i]));
    if (real[i] || imag[i]) 
      arg[i] = 180.0/PI*atan2(imag[i], real[i]);
    else
      arg[i] = 0;
  }
  /* find the maximu of magnitude */
  find_min_max(&min, &max, magData, n_freq);
  /*compute unwrapped phase by Louis' algorithm */
  phase_correction = 0;
  for (i=0; i<n_freq; i++) {
    if (i && magData[i]/max>unwrapLimit) {
      delta = arg[i] - arg[i-1];
      if (delta<-180.0) 
        phase_correction +=360;
      else if (delta>180.0) 
        phase_correction -=360;
    }
    unwrapped[i] = arg[i] + phase_correction;
  }
  sprintf(s, "Real%s", depenQuantity);
  if ((index1=SDDS_GetColumnIndex(SDDSout, s))<0)
    return 0;
  

  if ( !SDDS_SetColumn(SDDSout, SDDS_SET_BY_INDEX, real, n_freq, index1) ||
       !SDDS_SetColumn(SDDSout, SDDS_SET_BY_INDEX, imag, n_freq, index1+1) ||
       !SDDS_SetColumn(SDDSout, SDDS_SET_BY_INDEX, magData, n_freq, index1+2) ||
       !SDDS_SetColumn(SDDSout, SDDS_SET_BY_INDEX, arg, n_freq, index1+3) ||
       !SDDS_SetColumn(SDDSout, SDDS_SET_BY_INDEX, unwrapped, n_freq, index1+4))
    return 0;
 
  free(data);
  free(real_imag);
  free(real);
  free(imag);
  free(magData);
  free(arg);
  free(unwrapped);
  return 1;
}


long create_fft_columns(SDDS_DATASET *SDDSout, SDDS_DATASET *SDDSin, char *origName)
{
  char s[SDDS_MAXLINE];
  char *origUnits, *origSymbol;
  char *description, *name, *symbol;
  
  if (SDDS_GetColumnInformation(SDDSin, "units", &origUnits, SDDS_GET_BY_NAME, origName)!=SDDS_STRING ||
      SDDS_GetColumnInformation(SDDSin, "symbol", &origSymbol, SDDS_GET_BY_NAME, origName)!=SDDS_STRING)
    return 0;
  
  sprintf(s,"Real%s", origName);
  SDDS_CopyString(&name, s);
  if (!origSymbol)
    SDDS_CopyString(&origSymbol, origName);
  sprintf(s, "Re[%s]", origSymbol);
  sprintf(s, "Re[%s]", origSymbol);
  SDDS_CopyString(&symbol, s);
  sprintf(s, "Real part of %s", origSymbol);
  SDDS_CopyString(&description, s);
  
  if (SDDS_DefineColumn(SDDSout, name, symbol, NULL, description, NULL, SDDS_DOUBLE, 0)<0)
    return 0;
  free(name);
  free(symbol);
  free(description);
  
  sprintf(s, "Imag%s", origName);
  SDDS_CopyString(&name, s);
  if (!origSymbol)
    SDDS_CopyString(&origSymbol, origName);
  sprintf(s, "Im[%s]", origSymbol);
  SDDS_CopyString(&symbol, s);
  sprintf(s, "Imaginary part of %s", origSymbol);
  SDDS_CopyString(&description, s);
    
  if (SDDS_DefineColumn(SDDSout, name, symbol, NULL, description, NULL, SDDS_DOUBLE, 0)<0)
    return 0;
  free(name);
  free(symbol);
  free(description);
  
  sprintf(s, "Mag%s", origName);
  SDDS_CopyString(&name, s);
  
  if (!origSymbol)
    SDDS_CopyString(&origSymbol, origName);
  sprintf(s, "Mag[%s]", origSymbol);
  SDDS_CopyString(&symbol, s);
  sprintf(s, "Magnitude of %s", origSymbol);
  SDDS_CopyString(&description, s);
  if (SDDS_DefineColumn(SDDSout, name, symbol, NULL, description, NULL, SDDS_DOUBLE, 0)<0)
    return 0;
  free(name);
  free(symbol);
  free(description);

  
  sprintf(s, "Arg%s", origName);
  SDDS_CopyString(&name, s);
  
  if (!origSymbol)
    SDDS_CopyString(&origSymbol, origName);
  sprintf(s, "Arg[%s]", origSymbol);
  SDDS_CopyString(&symbol, s);
  sprintf(s, "Phase of %s", origSymbol);
  SDDS_CopyString(&description, s);
  if (SDDS_DefineColumn(SDDSout, name, symbol, "degrees", description, NULL, SDDS_DOUBLE, 0)<0)
    return 0;  
  free(name);
  free(symbol);
  free(description);

  sprintf(s, "UnwrappedArg%s", origName);
  SDDS_CopyString(&name, s);
  
  if (!origSymbol)
    SDDS_CopyString(&origSymbol, origName);
  sprintf(s, "UnwrappedArg[%s]", origSymbol);
  SDDS_CopyString(&symbol, s);
  sprintf(s, "Unwrapped Phase of %s", origSymbol);
  SDDS_CopyString(&description, s);
  if (SDDS_DefineColumn(SDDSout, name, symbol, "degrees", description, NULL, SDDS_DOUBLE, 0)<0)
    return 0;  
  free(name);
  free(symbol);
  free(description);

  return 1;
}


