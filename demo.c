/* Demo eines POSIX-C-Programs
 * R. Mentrup 3/2022
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>

int main(int argc, char *argv[])
{
    DIR *dp;
    struct dirent *dirp;

    /* printf("argc: %d\n", argc); */

    if (argc < 2) {
        fprintf(stderr, "Aufruf: %s [Verzeichnisname]\n",argv[0]);
        exit(11);
    }

    /* Die Funktion opendir ist Teil der POSIX-Spezifikation */
    dp = opendir(argv[1]);
    if(dp == NULL) {
        fprintf(stderr, "Fehler: Verzeichnis %s konnte nicht geoeffnet werden!\n",argv[1]);
        switch (errno) {
            case EACCES: printf("Zugriff verweigert\n"); break;
            case ENOENT: printf("Verzeichnis existiert nicht\n"); break;
            case ENOTDIR: printf("'%s' ist kein Verzeichnis\n", argv[1]); break;
        }
        exit(12);
    }

    while ((dirp = readdir(dp)) != NULL) {
        if(strcmp(dirp->d_name, ".") == 0)
            continue;
        if(strcmp(dirp->d_name, "..") == 0)
            continue;
        printf("%s ", dirp->d_name);
        if(dirp->d_type == DT_DIR)   /* ist es ein Verzeichnis? */
            puts("<dir>");
        else
            puts("");
        }

    if (closedir(dp) == -1)
        perror("closedir");

    exit(0);
}

