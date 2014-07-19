/*************************************************************************\
* Copyright (c) 2002 The University of Chicago, as Operator of Argonne
* National Laboratory.
* Copyright (c) 2002 The Regents of the University of California, as
* Operator of Los Alamos National Laboratory.
* This file is distributed subject to a Software License Agreement found
* in the file LICENSE that is included with this distribution. 
\*************************************************************************/

/*
 $Log: not supported by cvs2svn $
 Revision 1.10  2007/04/04 17:25:06  borland
 Added control of the number of fit terms for baseline fitting.  Output file now
 has an additional column to show which rows are used for baseline.

 Revision 1.9  2006/12/14 22:21:57  soliday
 Updated a bunch of programs because SDDS_SaveLayout is now called by
 SDDS_WriteLayout and it is no longer required to be called directly.
 Also the AutoCheckMode is turned off by default now so I removed calls to
 SDDS_SetAutoCheckMode that would attempt to turn it off. It is now up to
 the programmer to turn it on in new programs until debugging is completed
 and then remove the call to SDDS_SetAutoCheckMode.

 Revision 1.8  2006/10/19 17:55:39  soliday
 Updated to work with linux-x86_64.

 Revision 1.7  2005/11/04 22:46:11  soliday
 Updated code to be compiled by a 64 bit processor.

 Revision 1.6  2003/10/15 18:06:44  borland
 Fixed incorrect option checking that resulted in refusal to run with repeat=1.

 Revision 1.5  2003/09/02 19:16:03  soliday
 Cleaned up code for Linux.

 Revision 1.4  2002/08/14 17:12:40  soliday
 Added Open License

 Revision 1.3  2002/05/24 16:17:36  borland
 Added needed items to usage message.

 Revision 1.2  2001/05/23 17:15:28  borland
 Improved usage message.

 Revision 1.1  2001/04/30 22:55:23  borland
 Added sddscliptails and sddsbaseline to toolkit.

 */
#include "mdb.h"
#include "scan.h"
#include "SDDS.h"

#define CLO_PIPE 0
#define CLO_COLUMNS 1
#define CLO_METHOD 2
#define CLO_SELECT 3
#define CLO_NONNEGATIVE 4
#define CLO_REPEATS 5
#define CLO_DESPIKE 6
#define CLO_MAJOR_ORDER 7
#define CLO_OPTIONS 8

static char *option[CLO_OPTIONS] = {
  "pipe", "columns", "method", "select", "nonnegative", "repeats", "despike", "majorOrder",
    } ;

char *USAGE = "sddsbaseline [<input>] [<output>] [-pipe=[in][,out]]\n\
[-columns=<listOfNames>]\n\
[-nonnegative [-despike=passes=<number>,widthlimit=<value>] [-repeats=<count>]]\n\
[-select={endpoints=<number> | outsideFWHA=<multiplier> | antioutlier=<passes>}] \n\
[-method={average|fit[,terms=<number>]}] [-majorOrder=row|column]\n\
-columns        List of columns to process.\n\
-nonnegative    Forces all values to be nonnegative after baseline subtraction.\n\
                This is accomplished by setting all negative values to 0.\n\
-despike        Specify that positive features narrower than widthLimit shall be set to zero.\n\
-repeats        Specify how many times to apply the baseline removal algorithm.\n\
                Meaningful only if used in combination with -nonnegative.\n\
-select         Specify how to select points to include in baseline determination.\n\
-method         Specify how to process selected points in order to compute baseline.\n\
-majorOrder     Specify write output in row or column major order.\n\n\
Program by Michael Borland. (This is version 2, April 2007.)\n";

#define SELECT_ENDPOINTS   0x0001U
#define SELECT_OUTSIDEFWHA 0x0002U
#define SELECT_ANTIOUTLIER 0x0004U

#define METHOD_FIT     0x0001U
#define METHOD_AVERAGE 0x0002U

long resolveColumnNames(SDDS_DATASET *SDDSin, char ***column, int32_t *columns);
void selectEndpoints(short *selected, long rows, long endPoints);
void selectOutsideFWHA(double *data, double *indepData, short *selected, long rows, 
                       double fwhaLimit);
void selectAntiOutlier(double *data, short *selected, long rows, long passes);
void fitAndRemoveBaseline(double *data, double *indepData, short *selected, long rows, long terms);
void averageAndRemoveBaseline(double *data, short *selected, long rows);
void despikeProfile(double *data, long rows, long widthLimit, long passes);

int main(int argc, char **argv)
{
  int iArg;
  char **inputColumn;
  char *input, *output;
  long i, j, rows, readCode, repeats, repeat, fitTerms=2;
  int32_t columns;
  unsigned long pipeFlags, methodFlags, selectFlags, dummyFlags, majorOrderFlag;
  SCANNED_ARG *scanned;
  SDDS_DATASET SDDSin, SDDSout;
  double *data, *indepData;
  int32_t endPoints, antiOutlierPasses;
  short *selected;
  double outsideFWHA;
  long nonnegative;
  int32_t despikePasses, despikeWidthLimit;
  short columnMajorOrder=-1;
  
  SDDS_RegisterProgramName(argv[0]);
  argc = scanargs(&scanned, argc, argv); 
  if (argc<2)
    bomb(NULL, USAGE);

  output = input = NULL;
  inputColumn = NULL;
  columns = nonnegative = 0;
  repeats = 1;
  pipeFlags = methodFlags = selectFlags = dummyFlags = 0;
  endPoints = antiOutlierPasses = 0;
  outsideFWHA = 0;
  despikePasses = 0;
  despikeWidthLimit = 2;
  
  for (iArg=1; iArg<argc; iArg++) {
    if (scanned[iArg].arg_type==OPTION) {
      /* process options here */
      switch (match_string(scanned[iArg].list[0], option, CLO_OPTIONS, 0)) {
      case CLO_MAJOR_ORDER:
        majorOrderFlag=0;
        scanned[iArg].n_items -=1;
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
      case CLO_COLUMNS:
        if (scanned[iArg].n_items<2)
          SDDS_Bomb("invalid -columns syntax");
        inputColumn = tmalloc(sizeof(*inputColumn)*(columns = scanned[iArg].n_items-1));
        for (i=0; i<columns; i++)
          inputColumn[i] = scanned[iArg].list[i+1];
        break;
      case CLO_PIPE:
        if (!processPipeOption(scanned[iArg].list+1, scanned[iArg].n_items-1, &pipeFlags))
          SDDS_Bomb("invalid -pipe syntax");
        break;
      case CLO_METHOD:
        if (!(scanned[iArg].n_items -= 1))
          SDDS_Bomb("invalid -method syntax");
        if (!scanItemList(&methodFlags, scanned[iArg].list+1, &scanned[iArg].n_items, 0,
                          "average", -1, NULL, 0, METHOD_AVERAGE,
                          "fit", -1, NULL, 0, METHOD_FIT,
                          "terms", SDDS_LONG, &fitTerms, 1, 0,
                          NULL) ||
            bitsSet(methodFlags)!=1 || fitTerms<2) 
          SDDS_Bomb("invalid -method syntax/values");
        break;
      case CLO_SELECT:
        if (!(scanned[iArg].n_items -= 1))
          SDDS_Bomb("invalid -select syntax");
        if (!scanItemList(&selectFlags, scanned[iArg].list+1, &scanned[iArg].n_items, 0,
                          "endpoints", SDDS_LONG, &endPoints, 1, SELECT_ENDPOINTS,
                          "outsidefwha", SDDS_DOUBLE, &outsideFWHA, 1, SELECT_OUTSIDEFWHA,
                          "antioutlier", SDDS_LONG, &antiOutlierPasses, 1, SELECT_ANTIOUTLIER,
                          NULL) ||
            bitsSet(selectFlags)!=1)
          SDDS_Bomb("invalid -select syntax/values");
        break;
      case CLO_NONNEGATIVE:
        nonnegative = 1;
        break;
      case CLO_REPEATS:
        if (scanned[iArg].n_items!=2 ||
            sscanf(scanned[iArg].list[1], "%ld", &repeats)!=1 ||
            repeats<=0)
          SDDS_Bomb("invalid -repeats syntax");
        break;
      case CLO_DESPIKE:
        despikePasses = 1;
        if (!(scanned[iArg].n_items -= 1))
          SDDS_Bomb("invalid -despike syntax");
        if (!scanItemList(&dummyFlags, scanned[iArg].list+1, &scanned[iArg].n_items, 0,
                          "passes", SDDS_LONG, &despikePasses, 1, 0,
                          "widthlimit", SDDS_LONG, &despikeWidthLimit, 1, 0,
                          NULL) ||
            despikePasses<0 || despikeWidthLimit<1)
          SDDS_Bomb("invalid -despike syntax/values");
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

  if (!bitsSet(selectFlags))
    SDDS_Bomb("no -select option given");
  if (!bitsSet(methodFlags))
    SDDS_Bomb("no -method option given");

  if (!nonnegative && despikePasses)
    SDDS_Bomb("not meaningful to despike without setting -nonnegative");
  if (!nonnegative && repeats>1)
    SDDS_Bomb("not meaningful to repeat without setting -nonnegative");
  
  processFilenames("sddsbaseline", &input, &output, pipeFlags, 0, NULL);

  if (!columns)
    SDDS_Bomb("supply the names of columns to process with the -columns option");

  if (!SDDS_InitializeInput(&SDDSin, input))
    SDDS_PrintErrors(stderr, SDDS_VERBOSE_PrintErrors|SDDS_EXIT_PrintErrors);

  if (!resolveColumnNames(&SDDSin, &inputColumn, &columns))
    SDDS_PrintErrors(stderr, SDDS_VERBOSE_PrintErrors|SDDS_EXIT_PrintErrors);
  if (!columns)
    SDDS_Bomb("no columns selected for processing");

  if (!SDDS_InitializeCopy(&SDDSout, &SDDSin, output, "w"))
    SDDS_PrintErrors(stderr, SDDS_VERBOSE_PrintErrors|SDDS_EXIT_PrintErrors);
  if (!SDDS_DefineSimpleColumn(&SDDSout, "SelectedForBaselineDetermination", NULL, SDDS_SHORT))
    SDDS_PrintErrors(stderr, SDDS_VERBOSE_PrintErrors|SDDS_EXIT_PrintErrors);
  if (columnMajorOrder!=-1)
    SDDSout.layout.data_mode.column_major = columnMajorOrder;
  else
    SDDSout.layout.data_mode.column_major = SDDSin.layout.data_mode.column_major ;
  if (!SDDS_WriteLayout(&SDDSout))
    SDDS_PrintErrors(stderr, SDDS_VERBOSE_PrintErrors|SDDS_EXIT_PrintErrors);

  indepData = NULL;
  selected = NULL;
  while ((readCode=SDDS_ReadPage(&SDDSin))>0) {
    if (!SDDS_CopyPage(&SDDSout, &SDDSin))
      SDDS_PrintErrors(stderr, SDDS_VERBOSE_PrintErrors|SDDS_EXIT_PrintErrors);
    if ((rows = SDDS_CountRowsOfInterest(&SDDSin))) {
      if (!(indepData = SDDS_Realloc(indepData, sizeof(*indepData)*rows)) ||
          !(selected = SDDS_Realloc(selected, sizeof(*selected)*rows)))
        SDDS_Bomb("memory allocation failure");
      for (i=0; i<rows; i++)
        indepData[i] = i;
      for (i=0; i<columns; i++) {
        if (!(data = SDDS_GetColumnInDoubles(&SDDSin, inputColumn[i])))
          SDDS_PrintErrors(stderr, SDDS_VERBOSE_PrintErrors|SDDS_EXIT_PrintErrors);
        for (repeat=0; repeat<repeats; repeat++) {
          for (j=0; j<rows; j++) 
            selected[j] = 0;
          switch (selectFlags) {
          case SELECT_ENDPOINTS:
            selectEndpoints(selected, rows, endPoints);
            break;
          case SELECT_OUTSIDEFWHA:
            selectOutsideFWHA(data, indepData, selected, rows, outsideFWHA);
            break;
          case SELECT_ANTIOUTLIER:
            selectAntiOutlier(data, selected, rows, antiOutlierPasses);
            break;
          default:
            SDDS_Bomb("invalid select flag");
            break;
          }
          switch (methodFlags) {
          case METHOD_FIT:
            fitAndRemoveBaseline(data, indepData, selected, rows, fitTerms);
            break;
          case METHOD_AVERAGE:
            averageAndRemoveBaseline(data, selected, rows);
            break;
          default:
            break;
          }
          if (nonnegative) {
            for (j=0; j<rows; j++)
              if (data[j]<0)
                data[j] = 0;
            if (despikePasses)
              despikeProfile(data, rows, despikeWidthLimit, despikePasses);
          }
        }
        if (!SDDS_SetColumnFromDoubles(&SDDSout, SDDS_SET_BY_NAME, data, rows, inputColumn[i]) ||
            !SDDS_SetColumn(&SDDSout, SDDS_SET_BY_NAME, selected, rows, "SelectedForBaselineDetermination"))
          SDDS_PrintErrors(stderr, SDDS_VERBOSE_PrintErrors|SDDS_EXIT_PrintErrors);
        free(data);
      }
    }
    if (!SDDS_WritePage(&SDDSout))
      SDDS_PrintErrors(stderr, SDDS_VERBOSE_PrintErrors|SDDS_EXIT_PrintErrors);
  }
  if (indepData)
    free(indepData);
  if (selected)
    free(selected);
  return(0);
}


long resolveColumnNames(SDDS_DATASET *SDDSin, char ***column, int32_t *columns)
{
  long i;

  SDDS_SetColumnFlags(SDDSin, 0);
  for (i=0; i< *columns; i++) {
    if (!SDDS_SetColumnsOfInterest(SDDSin, SDDS_MATCH_STRING, (*column)[i], SDDS_OR))
      return 0;
  }
  free(*column);
  if (!(*column = SDDS_GetColumnNames(SDDSin, columns)) || *columns==0) {
    SDDS_SetError("no columns found");
    return 0;
  }
  return 1; 
}

void selectEndpoints(short *selected, long rows, long endPoints)
{
  long i;
  for (i=0; i<endPoints && i<rows; i++)
    selected[i] = 1;
  for (i=rows-1; i>rows-endPoints-1 && i>=0; i--)
    selected[i] = 1;
}

void selectOutsideFWHA(double *data, double *indepData, short *selected, long rows, 
                       double fwhaLimit)
{
  double top, base, fwha;
  long i1, i2, i2save, imin, imax, i;
  double point1, point2;
  
  if (rows<3 || fwhaLimit<=0)
    return;

  index_min_max(&imin, &imax, data, rows);
  if (!data[imax])
    return;
  
  if (!findTopBaseLevels(&top, &base, data, rows, 50, 2.0)) 
    return;
  
  if ((i1=findCrossingPoint( 0, data, rows, (top-base)*0.5+base,  1, indepData, &point1))<0 ||
      (i2=i2save=findCrossingPoint(i1, data, rows, (top-base)*0.9+base, -1, NULL, NULL))<0 ||
      (i2=findCrossingPoint(i2, data, rows, (top-base)*0.5+base, -1, indepData, &point2))<0 ) {
    return;
  }
  fwha = point2 - point1;
  
  for (i=0; i<rows; i++)
    selected[i] = 1;
  for (i=imax-fwha*fwhaLimit; i<=imax+fwha*fwhaLimit; i++) {
    if (i<0)
      continue;
    if (i>=rows)
      break;
    selected[i] = 0;
  }
}

void selectAntiOutlier(double *data, short *selected, long rows, long passes)
{
  double ave, sum2, limit;
  long i, count;

  for (i=0; i<rows; i++)
    selected[i] = 1;
  while (--passes >= 0) {
    for (i=ave=count=0; i<rows; i++)
      if (selected[i]) {
        ave += data[i];
        count++;
      }
    if (!count)
      break;
    ave /= count;
    for (i=sum2=0; i<rows; i++)
      if (selected[i])
        sum2 += sqr(data[i]-ave);
    limit = 2*sqrt(sum2/count);
    for (i=0; i<rows; i++)
      if (selected[i] && fabs(data[i]-ave)>limit) 
        selected[i] = 0;
  }
}

void fitAndRemoveBaseline(double *data0, double *indepData0, short *selected, long rows, long fitTerms)
{
  double *data, *indepData=NULL;
  long count, i, j;
  double chi;
  double *coef, *sCoef;
  
  if (!(coef = malloc(sizeof(*coef)*fitTerms)) ||
      !(sCoef = malloc(sizeof(*sCoef)*fitTerms)))
    SDDS_Bomb("allocation failure (fitAndRemoveBaseline)");
  for (i=count=0; i<rows; i++)
    if (selected[i])
      count++;
  if (count<3)
    return;
  if (!(data=malloc(sizeof(*data)*count)) ||
      !(indepData=malloc(sizeof(*indepData)*count)))
    SDDS_Bomb("memory allocation failure");
  for (i=j=0; i<rows; i++) {
    if (selected[i]) {
      data[j] = data0[i];
      indepData[j] = indepData0[i];
      j++;
    }
  }

  if (!lsfn(indepData, data, NULL, count, fitTerms-1, coef, sCoef, &chi, NULL)) 
    return;

  for (i=0; i<rows; i++) {
    double term;
    term = 1;
    for (j=0; j<fitTerms; j++) {
      data0[i] -= term*coef[j];
      term *= indepData0[i];
    }
  }
  
  free(data);
  free(indepData);
  free(coef);
  free(sCoef);
}

void averageAndRemoveBaseline(double *data, short *selected, long rows)
{
  long i, count;
  double ave;
  for (i=count=ave=0; i<rows; i++)
    if (selected[i]) {
      count++;
      ave += data[i];
    }
  
  if (count) {
    ave /= count;
    for (i=0; i<rows; i++)
      data[i] -= ave;
  }
}

void despikeProfile(double *data, long rows, long widthLimit, long passes)
{
  long i, i0, i1, j;
  while (--passes >= 0) {
    for (i=0; i<rows; i++) {
      if (i==0) {
        if (data[i]!=0)
          continue;
      }
      else if (!(data[i-1]==0 && data[i]!=0))
        continue;
      i0 = i;
      for (i1=i+1; i1<rows; i1++)
        if (!data[i1])
          break;
      if ((i1-i0)<=widthLimit)
        for (j=i0; j<i1; j++)
          data[j] = 0;
    }
  }
}
