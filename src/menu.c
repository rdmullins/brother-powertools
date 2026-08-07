#include <stdio.h>

int main (void)
{
    int a = 0;
    
    printf("Brother PowerTools\n\n");
    printf("1. Send Text\n");
    printf("2. Receive Text\n");
    printf("3. Exit\n\n");
    printf("Enter your choice: ");
    scanf("%d", &a);
    printf("\nYou selected option %d\n", a);
    return 0;
}