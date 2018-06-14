/***********************************************************************
name: khalid khaskia
id: 308425420

File: FFDM.C - First Fit Decreasing Method
Overview:
 This program implements the FFDM algorithm for generating a Perfect Hash
 Function (PHF) for a given set of integer search keys.
Invocation:
 FFDM KEYFILE t-VALUE
 KEYFILE is the fully-qualified name of the text file containing the integer
 search keys, one per line.
 t-VALUE is the "magic" number to use for the hash function.  The value of
 t must be such that t*t > max(key).
**************************************************************************/

#include <stdio.h>      // fopen() fclose()
#include <stdlib.h>     // atoi() exit()
#include <string.h>     // strcpy()


// error codes
#define Success           0
#define t_value_ERROR    -1
#define fopen_ERROR      -2
#define key_value_ERROR  -3

// application-specific constants
#define t_Max           100   // must be at least sqrt(max_key)
#define HashTable_Max  1000   // upper limit for
#define InvalidKey       -1   // a key value that's impossible for your app

struct RowStruct
{
   int RowNumber;                      // the row number in array A[][]
   int RowItemCnt;                     // the # of items in this row of A[][]
};

// global data

// the arrays A[][], r[] and C[] are those in the article "Perfect Hashing"
int A[t_Max][t_Max];       // A[i][j]=K (i=K/t, j=K mod t) for each key K
int r[t_Max];              // r[R]=amount row A[R][] was shifted
int C[HashTable_Max];      // the shifted rows of A[][] collapse into C[]

// Row[] exists to facilitate sorting the rows of A[][] by their "fullness"
struct RowStruct Row[t_Max];     // entry counts for the rows in A[][]

/***********************************************************************
Function: void InitArrays(void)
Overview: Prepares the algorithm data structures for use.
Parameters: none
Return Value: none
Notes & Caveats:
-A row offset may be 0, so the items in r[] are set to a negative value to
 indicate that the offset for each row is not known yet.
-Every item in A[][] and C[] is set to a value that is known to be an invalid
 key for the specific application.  -1 is often a good choice.
************************************************************************/

void InitArrays(void)
{
   int row, column;
   for (row = 0; row < t_Max; row++)
   {
      r[row] = -1;                     // valid offsets are non-negative
      Row[row].RowNumber  = row;       // insert the row numbers and
      Row[row].RowItemCnt = 0;         //  indicate that each row is empty

      for (column = 0; column < t_Max; column++)
      {
         A[row][column] = InvalidKey;
      }
   }
   for (row = 0; row < HashTable_Max; row++)
   {
      C[row] = InvalidKey;
   }
}
/*************************************************************************
Function: int ReadKeyData(char *Filename, int t, int *KeyCount)
Overview: Reads the file of seach keys and maps them into the array A[][].
Parameters:
 char *Filename   - the name of the file containing the search keys
 int t            - the number of rows in A[][]; max(key) must be < t*t
 int *KeyCount    - pointer to location to place number of keys read
Return Value:
 fopen_ERROR      - the specified file could not be opened (doesn't exist)
 key_value_ERROR  - a search key value was too large (depends on t)
 Success          - successful completion of responsibilities
Notes & Caveats:
-The number of items in each row is also computed and returned in
 Row[row].RowItemCnt.
-The number of keys is returned to the caller via a pointer.  If an error
 is detected the number of keys reflects how many keys were read before the
 error condition was detected.
************************************************************************/

int ReadKeyData(char *Filename, int t, int *KeyCount)
{
   FILE *fptr;
   int key;
   int row, column;

   *KeyCount = 0;               // set # keys=0 before attempting fopen
   fptr = fopen(Filename, "rt");
   if (fptr == NULL) return(fopen_ERROR);
// fill data structures using search key data
   while ((fscanf(fptr, "%d", &key)) == 1)
   {
      row    = key / t;
      column = key % t;
      if (row >= t) return(key_value_ERROR);
      A[row][column] = key;
      Row[row].RowItemCnt++;
      (*KeyCount)++;
   }
   return(Success);
}
/***********************************************************************
Function: void SortRows(int t)
Overview: sort Row[] in descending order of row fullness.
Parameters:
 int t - the number of rows in A[][]; max(key) must be < t*t
Return Value: none
Notes & Caveats:
-The algorithm needs to know which row of A[][] is most full, 2nd most full,
 etc.  This is most easily done by sorting an array of row-item-counts and
 remembering which row the item counts go with.  That is what the array
 "struct RowStruct Row[]" does for us.
-I saw no point in trying to be clever here, so a simple bubble sort is used.
************************************************************************/

void SortRows(int t)
{
   int i, j;
   struct RowStruct tmp;
   for (i = 0; i < t-1; i++)
   {
      for (j = i+1; j < t; j++)
      {
         if (Row[i].RowItemCnt < Row[j].RowItemCnt)
         {
            tmp    = Row[i];
            Row[i] = Row[j];
            Row[j] = tmp;
         }
      }
   }
}
void main(int argc, char *argv[])
{
   int t, NumKeys;
   int k, ndx, rc, row, offset, PrintFlag;
   char Filename[64];
// process the command-line arguments
   if (argc < 3)
   {
      printf("usage: FFDM KEYFILE t-VALUE\n");
      printf("where: KEYFILE is the name of your file of numeric keys\n");
      printf("       t-VALUE is a number such that t*t > max(key)\n");
      exit(-1);
   }
   strcpy(Filename, argv[1]);
   t = atoi(argv[2]);

   if (t > t_Max)
   {
      printf("t may not exceed %d\n", t_Max);
      exit(t_value_ERROR);
   }
   if (argc > 3) PrintFlag = 1;
   else          PrintFlag = 0;
// initialize data structures
   InitArrays();

// read in the user's key data
   rc = ReadKeyData(Filename, t, &NumKeys);
   if (rc != 0)
   {
      printf("ReadKeyData() failed with error %d\n", rc);
      exit(rc);
   }
// prime the algorithm - sort the rows by their fullness
   SortRows(t);

// do the First-Fit Descending Method algorithm
// For each non-empty row:
// 1. shift the row right until none of its items collide with any of
//    the items in previous rows.
// 2. Record the shift amount in array r[].
// 3. Insert this row into the hash table C[].

   for (ndx = 0; Row[ndx].RowItemCnt > 0; ndx++)
   {
      row = Row[ndx].RowNumber;        // get the next non-empty row
      for (offset = 0; offset < HashTable_Max-t-1; offset++)
      {
         for (k = 0; k < t; k++)       // does this offset avoids collisions?
         {
            if ((C[offset+k] != InvalidKey) &&
                (A[row][k]   != InvalidKey)) break;
         }
         if (k == t)
         {
            r[row] = offset;          // record the shift amount for this row
            for (k = 0; k < t; k++) // insert this row into the hash table
            {
               if (A[row][k] != InvalidKey)
               {
                  C[offset+k] = A[row][k];
               }
            }
            break;
         }
      }
      if (offset == HashTable_Max-t-1)
      {
         printf("failed to fit row %d into the hash table\n", row);
         printf("try increasing the hash table size\n");
         exit(-1);
      }
   }
// all done!  locate the "right-most" hash table entry
   for (k = 0; k < HashTable_Max; k++)
   {
      if (C[k] != InvalidKey) offset = k+1;
   }
// print the results
   printf("t value          : %d\n",  t);
   printf("Number of keys   : %d\n",  NumKeys);
   printf("Hash table size  : %d\n",  offset);
   printf("Table utilization: %f%\n", 100.0*NumKeys/offset);

   if (PrintFlag != 0)
   {
      printf("\noffset table r[]\n");
      printf("row offset\n");
      for (k = 0; k < t; k++)
      {
         printf("%2d  %3d\n", k, r[k]);
      }
      printf("\nhash table C[]\n");
      for (k = 0; k < offset; k++)
      {
         printf("%d\n", C[k]);
      }
   }
}
