#include <stdio.h>

int main (void)
{
    int choice = 0;

    while (1) {
    
    printf("Brother PowerTools\n\n");
    printf("1. Send Text\n");
    printf("2. Receive Text\n");
    printf("3. BASIC\n");
    printf("4. Settings\n");
    printf("5. Exit\n\n");
    printf("Enter your choice: ");
    
    if (scanf("%d", &choice) != 1) {
        printf("scanf failed!\n");
        return 1;
    };

    switch (choice) {
        case 1:
            printf("You selected Send Text\n");
            break;
        case 2:
            printf("You selected Receive Text\n");
            break;
        case 3:
            printf("You selected BASIC\n");
            break;
        case 4:
            printf("You selected Settings\n");
            break;
        case 5:
            printf("Exiting...\n");
            return 0;
        default:
            printf("Invalid option. Please try again.\n");
        }
  
    }
    return 0; 
}