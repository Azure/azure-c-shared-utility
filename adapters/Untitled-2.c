#include <stdint.h>
#include <stdio.h>
#include <string.h>

void receiveInput(int32_t ***matrix, int32_t* columns, int32_t* rows)
{
    char buffer[256];
    char* bufferPtr;
    char c;

    // Assumes valid positive integer input followed by newline.
    printf("Provide number of columns: ");  

    bufferPtr = buffer;    
    while((c = getchar()) != '\n')
    {
        *bufferPtr = c;
        bufferPtr++;
    }
    *bufferPtr = 0; // complete c-string with '\0'
    *columns = atoi(buffer);

    printf("Provide number of rows: ");  

    bufferPtr = buffer; // reset bufferPtr to beginning of char buffer;
    while((c = getchar()) != '\n')
    {
        *bufferPtr = c;
        bufferPtr++;
    }
    *bufferPtr = 0; // complete c-string with '\0'
    *rows = atoi(buffer);

    // Allocate dynamic memory for matrix
    *matrix = (int32_t**)calloc(*rows * sizeof(int32_t*));
    assert(*matrix); // better not be NULL;

    for(int i = 0; i < *rows; i++)
    {
        int32_t* matrixRow = (int32_t*)calloc(*columns * sizeof(int32_t));
        assert(matrixRow); // better not be NULL;

        *matrix[i] = matrixRow;
    }

    // Fill in matrix from user info.  Filling one row at a time.  If not enough values provided, left as 0.
    uint64_t numberOfValues = (uint64_t)(*columns * *rows);
    printf("Provide %lu integer values:\n", numberOfValues);
    for (int i = 0; i < *columns; ++i)
    {
        for (int j = 0; j < *rows; ++j)
        {
            bufferPtr = buffer; // reset bufferPtr to beginning of char buffer;
            printf("Value %d/%lu: ", i+1, numberOfValues);
            while((c = getchar()) != '\n')
            {
                *bufferPtr = c;
                bufferPtr++;
            }
            *bufferPtr = 0; // complete c-string with '\0'
            *matrix[i][j] = atoi(buffer);
        }
    }
}

void printMatrix(int32_t **matrix, int32_t columns, int32_t rows) 
{
    printf("Matrix: \n");
    for (int i = 0; i < columns; ++i)
    {
        for (int j = 0; j < rows; ++j)
        {
            printf("%d ", matrix[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

void printSpiral(int32_t ** matrix, int32_t column, int32_t row)
{

}

int main(void)
{
    int32_t column = 0;
    int32_t row = 0;
    int32_t** matrix;

    receiveInput(&matrix, &column, &row);
    printMatrix(matrix, row, column);
    printSpiral(matrix, row, column);

    return 0;
}