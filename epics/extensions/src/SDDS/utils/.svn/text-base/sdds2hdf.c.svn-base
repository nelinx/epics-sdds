#include <stdio.h>
#include "hdf5.h"
#include "mdb.h"
#include "SDDS.h"
#include "scan.h"

int writeStringAttribute(const char* name, const char* value, hid_t dataset_id);
int writeIntAttribute(const char* name, int value, hid_t dataset_id);

#define SET_WAVELENGTH 0
#define SET_PIPE 1
#define N_OPTIONS 2
char *option[N_OPTIONS] = {
  "wavelength", "pipe"
};
char *USAGE = "sdds2hdf [<input-file>] [-pipe=in] <output-file>\n\nProgram by Robert Soliday. ("__DATE__")\n";

int main( int argc, char **argv ) {
  SCANNED_ARG *s_arg;
  long i_arg;
  unsigned long pipeFlags=0;
  char *input = NULL;
  char *output = NULL;
  SDDS_TABLE SDDS_table;
  char **columnNames, **parameterNames, **arrayNames;
  int32_t ncolumns, nparameters, narrays;
  void **columnData=NULL, **parameterData=NULL;
  SDDS_ARRAY **arrayData=NULL;
  long *columnType=NULL, *parameterType=NULL, *arrayType=NULL;
  long *columnhdfType=NULL, *parameterhdfType=NULL, *arrayhdfType=NULL;
  char **columnUnits=NULL, **parameterUnits=NULL, **arrayUnits=NULL;
  hid_t file_id;
  hid_t column_dataspace_id;
  hid_t column_dataset_id;
  hid_t array_dataspace_id;
  hid_t array_dataset_id;
  hid_t parameter_dataspace_id;
  hid_t parameter_dataset_id;
  hid_t page_group_id=0, column_group_id=0, parameter_group_id=0, array_group_id=0;
  hid_t stringType;
  hsize_t column_dims[1];
  hsize_t parameter_dims[1];
  hsize_t **array_dims=NULL;
  herr_t status;
  int32_t i, j, page;
  int32_t rows;
  char buffer[100];
  char *description=NULL, *contents=NULL;

  SDDS_RegisterProgramName(argv[0]);

  argc = scanargs(&s_arg, argc, argv);
  if (argc<3) {
    fprintf(stderr, "%s", USAGE);
    return(1);
  }
  for (i_arg=1; i_arg<argc; i_arg++) {
    if (s_arg[i_arg].arg_type==OPTION) {
      delete_chars(s_arg[i_arg].list[0], "_");
      switch (match_string(s_arg[i_arg].list[0], option, N_OPTIONS, 0)) {
      case SET_PIPE:
        if (!processPipeOption(s_arg[i_arg].list+1, s_arg[i_arg].n_items-1, &pipeFlags)) {
          fprintf(stderr, "invalid -pipe syntax\n");
          return(1);
        }
        break;
      default:
	fprintf(stderr, "Error: unknown switch: %s\n", s_arg[i_arg].list[0]);
	return(1);
      }
    } else {
      if (input == NULL)
        SDDS_CopyString(&input,s_arg[i_arg].list[0]);
      else if (output == NULL)
        SDDS_CopyString(&output,s_arg[i_arg].list[0]);
      else {
	fprintf(stderr, "Error: too many filenames\n");
	return(1);
      }
    }
  }

  if (pipeFlags&USE_STDIN) {
    processFilenames("sdds2hdf", &input, &output, USE_STDIN, 1, NULL);
  }

  if (SDDS_InitializeInput(&SDDS_table, input) == 0) {
    SDDS_PrintErrors(stderr, SDDS_VERBOSE_PrintErrors);
    return(1);
  }
  file_id = H5Fcreate(output, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

  if (!SDDS_GetDescription(&SDDS_table, &description, &contents)) {
    SDDS_PrintErrors(stderr, SDDS_VERBOSE_PrintErrors);
    exit(1);
  }
  if (description) {
    writeStringAttribute("description", description, file_id);
    free(description);
  }
  if (contents) {
    writeStringAttribute("contents", contents, file_id);
    free(contents);
  }

  parameterNames = SDDS_GetParameterNames(&SDDS_table, &nparameters);
  if (nparameters > 0) {
    parameterType = tmalloc(sizeof(*parameterType)*nparameters);
    parameterhdfType = tmalloc(sizeof(*parameterType)*nparameters);
    parameterData = tmalloc(sizeof(*parameterData)*nparameters);
    parameterUnits = tmalloc(sizeof(*parameterUnits)*nparameters);
    parameter_dims[0] = 1;
    for (i=0; i<nparameters; i++) {
      if ((parameterType[i]=SDDS_GetParameterType(&SDDS_table, i))<=0 ||
          SDDS_GetParameterInformation(&SDDS_table, "units", &parameterUnits[i], SDDS_GET_BY_INDEX, i)!=SDDS_STRING) {
        SDDS_PrintErrors(stderr, SDDS_VERBOSE_PrintErrors);
        exit(1);
      }
      if (parameterType[i] == SDDS_DOUBLE) {
        parameterhdfType[i] = H5T_NATIVE_DOUBLE;
      } else if (parameterType[i] == SDDS_FLOAT) {
        parameterhdfType[i] = H5T_NATIVE_FLOAT;
      } else if (parameterType[i] == SDDS_ULONG) {
        parameterhdfType[i] = H5T_NATIVE_UINT;
      } else if (parameterType[i] == SDDS_LONG) {
        parameterhdfType[i] = H5T_NATIVE_INT;
      } else if (parameterType[i] == SDDS_USHORT) {
        parameterhdfType[i] = H5T_NATIVE_USHORT;
      } else if (parameterType[i] == SDDS_SHORT) {
        parameterhdfType[i] = H5T_NATIVE_SHORT;
      } else if (parameterType[i] == SDDS_CHARACTER) {
        parameterhdfType[i] = H5T_NATIVE_CHAR;
      } else if (parameterType[i] == SDDS_STRING) {
        parameterhdfType[i] = H5T_C_S1;
      }
    }
  }

  arrayNames = SDDS_GetArrayNames(&SDDS_table, &narrays);
  if (narrays == 0) {
    if (arrayNames) free(arrayNames);
  }
  if (narrays > 0) {
    arrayType = tmalloc(sizeof(*arrayType)*narrays);
    arrayhdfType = tmalloc(sizeof(*arrayType)*narrays);
    arrayData = tmalloc(sizeof(**arrayData)*narrays);
    arrayUnits = tmalloc(sizeof(*arrayUnits)*narrays);
    array_dims = tmalloc(sizeof(*array_dims)*narrays);

    for (i=0; i<narrays; i++) {
      array_dims[i] = tmalloc(sizeof(*(array_dims[i])) * SDDS_table.layout.array_definition[i].dimensions);
      if ((arrayType[i]=SDDS_GetArrayType(&SDDS_table, i))<=0 ||
          SDDS_GetArrayInformation(&SDDS_table, "units", &arrayUnits[i], SDDS_GET_BY_INDEX, i)!=SDDS_STRING) {
        SDDS_PrintErrors(stderr, SDDS_VERBOSE_PrintErrors);
        exit(1);
      }
      if (arrayType[i] == SDDS_DOUBLE) {
        arrayhdfType[i] = H5T_NATIVE_DOUBLE;
      } else if (arrayType[i] == SDDS_FLOAT) {
        arrayhdfType[i] = H5T_NATIVE_FLOAT;
      } else if (arrayType[i] == SDDS_ULONG) {
        arrayhdfType[i] = H5T_NATIVE_UINT;
      } else if (arrayType[i] == SDDS_LONG) {
        arrayhdfType[i] = H5T_NATIVE_INT;
      } else if (arrayType[i] == SDDS_USHORT) {
        arrayhdfType[i] = H5T_NATIVE_USHORT;
      } else if (arrayType[i] == SDDS_SHORT) {
        arrayhdfType[i] = H5T_NATIVE_SHORT;
      } else if (arrayType[i] == SDDS_CHARACTER) {
        arrayhdfType[i] = H5T_NATIVE_CHAR;
      } else if (arrayType[i] == SDDS_STRING) {
        arrayhdfType[i] = H5T_C_S1;
      }
    }
  }


  columnNames = SDDS_GetColumnNames(&SDDS_table, &ncolumns);
  if (ncolumns > 0) {
    columnType = tmalloc(sizeof(*columnType)*ncolumns);
    columnhdfType = tmalloc(sizeof(*columnType)*ncolumns);
    columnData = tmalloc(sizeof(*columnData)*ncolumns);
    columnUnits = tmalloc(sizeof(*columnUnits)*ncolumns);

    for (i=0; i<ncolumns; i++) {
      if ((columnType[i]=SDDS_GetColumnType(&SDDS_table, i))<=0 ||
          SDDS_GetColumnInformation(&SDDS_table, "units", &columnUnits[i], SDDS_GET_BY_INDEX, i)!=SDDS_STRING) {
        SDDS_PrintErrors(stderr, SDDS_VERBOSE_PrintErrors);
        exit(1);
      }
      if (columnType[i] == SDDS_DOUBLE) {
        columnhdfType[i] = H5T_NATIVE_DOUBLE;
      } else if (columnType[i] == SDDS_FLOAT) {
        columnhdfType[i] = H5T_NATIVE_FLOAT;
      } else if (columnType[i] == SDDS_ULONG) {
        columnhdfType[i] = H5T_NATIVE_UINT;
      } else if (columnType[i] == SDDS_LONG) {
        columnhdfType[i] = H5T_NATIVE_INT;
      } else if (columnType[i] == SDDS_USHORT) {
        columnhdfType[i] = H5T_NATIVE_USHORT;
      } else if (columnType[i] == SDDS_SHORT) {
        columnhdfType[i] = H5T_NATIVE_SHORT;
      } else if (columnType[i] == SDDS_CHARACTER) {
        columnhdfType[i] = H5T_NATIVE_CHAR;
      } else if (columnType[i] == SDDS_STRING) {
        columnhdfType[i] = H5T_C_S1;
      }
    }
  }

  page = SDDS_ReadTable(&SDDS_table);
  if (page <= 0) {
    fprintf(stderr, "No data in SDDS file."); 
    status = H5Fclose(file_id);
    return(1);
  }

  while (page > 0) {
    sprintf(buffer, "/page%" PRId32, page);
    page_group_id = H5Gcreate(file_id, buffer, 0);
    if (nparameters > 0) {
      for (i=0; i<nparameters; i++) {
        if (parameterType[i] == SDDS_STRING) {
          parameterData[i]=SDDS_GetParameterAsString(&SDDS_table, SDDS_table.layout.parameter_definition[i].name, NULL);
        } else {
          parameterData[i]=SDDS_GetParameter(&SDDS_table, SDDS_table.layout.parameter_definition[i].name, NULL);
        }
        if (!(parameterData[i])) {
          SDDS_PrintErrors(stderr, SDDS_VERBOSE_PrintErrors);
          exit(1);
        }
      }
      parameter_group_id = H5Gcreate(page_group_id, "parameters", 0);
      for (i=0; i<nparameters; i++) {
        parameter_dataspace_id = H5Screate_simple(1, parameter_dims, NULL);
        if (parameterType[i] == SDDS_STRING) {
          stringType = H5Tcopy(H5T_C_S1);
          H5Tset_size(stringType, strlen(parameterData[i]) + 1);
          parameter_dataset_id = H5Dcreate(parameter_group_id, SDDS_table.layout.parameter_definition[i].name, stringType, parameter_dataspace_id, H5P_DEFAULT);
          status = H5Dwrite(parameter_dataset_id, stringType, H5S_ALL, H5S_ALL, H5P_DEFAULT, parameterData[i]);
          
        } else {
          parameter_dataset_id = H5Dcreate(parameter_group_id, SDDS_table.layout.parameter_definition[i].name, parameterhdfType[i], parameter_dataspace_id, H5P_DEFAULT);
          status = H5Dwrite(parameter_dataset_id, parameterhdfType[i], H5S_ALL, H5S_ALL, H5P_DEFAULT, parameterData[i]);
        }
        writeStringAttribute("units", parameterUnits[i], parameter_dataset_id);
        status = H5Dclose(parameter_dataset_id);
        status = H5Sclose(parameter_dataspace_id);
        free(parameterData[i]);
      }
      status = H5Gclose(parameter_group_id);
    }
    if (narrays > 0) {
      for (i=0; i<narrays; i++) {
        arrayData[i]=SDDS_GetArray(&SDDS_table, SDDS_table.layout.array_definition[i].name, NULL);
        if (!(arrayData[i])) {
          SDDS_PrintErrors(stderr, SDDS_VERBOSE_PrintErrors);
          exit(1);
        }
      }
      array_group_id = H5Gcreate(page_group_id, "arrays", 0);
      for (i=0; i<narrays; i++) {
        for (j=0; j<SDDS_table.layout.array_definition[i].dimensions; j++) {
          array_dims[i][j] = arrayData[i]->dimension[j];
        }
        array_dataspace_id = H5Screate_simple(SDDS_table.layout.array_definition[i].dimensions, array_dims[i], NULL);
        if (arrayType[i] == SDDS_STRING) {
          stringType = H5Tcopy(H5T_C_S1);
          H5Tset_size(stringType, H5T_VARIABLE);
          array_dataset_id = H5Dcreate(array_group_id, SDDS_table.layout.array_definition[i].name, stringType, array_dataspace_id, H5P_DEFAULT);
          status = H5Dwrite(array_dataset_id, stringType, H5S_ALL, H5S_ALL, H5P_DEFAULT, arrayData[i]->data);
        } else {
          array_dataset_id = H5Dcreate(array_group_id, SDDS_table.layout.array_definition[i].name, arrayhdfType[i], array_dataspace_id, H5P_DEFAULT);
          status = H5Dwrite(array_dataset_id, arrayhdfType[i], H5S_ALL, H5S_ALL, H5P_DEFAULT, arrayData[i]->data);
        }
        writeStringAttribute("units", arrayUnits[i], array_dataset_id);
        status = H5Dclose(array_dataset_id);
        status = H5Sclose(array_dataspace_id);
        SDDS_FreeArray(arrayData[i]);
      }
      status = H5Gclose(array_group_id);
    }
    if (ncolumns > 0) {
      rows = SDDS_RowCount(&SDDS_table);
      for (i=0; i<ncolumns; i++) {
        columnData[i]=SDDS_GetColumn(&SDDS_table, SDDS_table.layout.column_definition[i].name);
        if (!(columnData[i])) {
          SDDS_PrintErrors(stderr, SDDS_VERBOSE_PrintErrors);
          exit(1);
        }
      }
      column_group_id = H5Gcreate(page_group_id, "columns", 0);
      for (i=0; i<ncolumns; i++) {
        column_dims[0] = rows; 
        column_dataspace_id = H5Screate_simple(1, column_dims, NULL);
        if (columnType[i] == SDDS_STRING) {
          stringType = H5Tcopy(H5T_C_S1);
          H5Tset_size(stringType, H5T_VARIABLE);
          column_dataset_id = H5Dcreate(column_group_id, SDDS_table.layout.column_definition[i].name, stringType, column_dataspace_id, H5P_DEFAULT);
          status = H5Dwrite(column_dataset_id, stringType, H5S_ALL, H5S_ALL, H5P_DEFAULT, columnData[i]);
        } else {
          column_dataset_id = H5Dcreate(column_group_id, SDDS_table.layout.column_definition[i].name, columnhdfType[i], column_dataspace_id, H5P_DEFAULT);
          status = H5Dwrite(column_dataset_id, columnhdfType[i], H5S_ALL, H5S_ALL, H5P_DEFAULT, columnData[i]);
        }
        writeStringAttribute("units", columnUnits[i], column_dataset_id);
        status = H5Dclose(column_dataset_id);
        status = H5Sclose(column_dataspace_id);
        if (columnType[i] == SDDS_STRING) {
          SDDS_FreeStringArray(columnData[i], rows);
          free(columnData[i]);
        } else {
          free(columnData[i]);
        }
      }
      status = H5Gclose(column_group_id);
    }
    status = H5Gclose(page_group_id);
    page = SDDS_ReadTable(&SDDS_table);
  }
  status = H5Fclose(file_id);
  if (nparameters > 0) {
    for (i=0; i<nparameters; i++) {
      free(parameterNames[i]);
      if (parameterUnits[i]) free(parameterUnits[i]);
    }
    free(parameterData);
    free(parameterNames);
    free(parameterhdfType);
    free(parameterType);
    free(parameterUnits);
  }
  if (narrays > 0) {
    for (i=0; i<narrays; i++) {
      free(array_dims[i]);
      free(arrayNames[i]);
      if (arrayUnits[i]) free(arrayUnits[i]);
    }
    free(array_dims);
    free(arrayData);
    free(arrayNames);
    free(arrayhdfType);
    free(arrayType);
    free(arrayUnits);
  }
  if (ncolumns > 0) {
    for (i=0; i<ncolumns; i++) {
      free(columnNames[i]);
      if (columnUnits[i]) free(columnUnits[i]);
    }
    free(columnData);
    free(columnNames);
    free(columnhdfType);
    free(columnType);
    free(columnUnits);
  }
  if (SDDS_Terminate(&SDDS_table) == 0) {
    SDDS_PrintErrors(stderr, SDDS_VERBOSE_PrintErrors);
    return(1);
  }
  
  free_scanargs(&s_arg,argc);
  free(output);
  free(input);
  return(0);
}

int writeStringAttribute(const char* name, const char* value, hid_t dataset_id) {
  hid_t attribute_id;
  hid_t atype;
  hid_t a_id;
  herr_t status;

  a_id  = H5Screate(H5S_SCALAR);
  atype = H5Tcopy(H5T_C_S1);
  if (value == NULL)
    H5Tset_size(atype, 1);
  else
    H5Tset_size(atype, strlen(value) + 1);
  H5Tset_strpad(atype,H5T_STR_NULLTERM);
  attribute_id = H5Acreate(dataset_id, name, atype, a_id, H5P_DEFAULT);
  if (value == NULL)
    status = H5Awrite(attribute_id, atype, "");
  else
    status = H5Awrite(attribute_id, atype, value);
  status = H5Aclose(attribute_id);
  status = H5Tclose(atype);
  status = H5Sclose(a_id);

  return(0);
}
int writeIntAttribute(const char* name, int value, hid_t dataset_id) {
  hid_t attribute_id;
  herr_t status;

  attribute_id = H5Acreate(dataset_id, name, H5T_NATIVE_INT, H5S_SCALAR, H5P_DEFAULT);
  status = H5Awrite(attribute_id, H5T_NATIVE_INT, &value);
  status = H5Aclose(attribute_id);

  return(0);
}




