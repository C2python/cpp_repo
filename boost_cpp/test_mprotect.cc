int main(void) {
    char *str;

    /* Stored in read only part of data segment */
    str = "core dump";

    /* Problem:  trying to modify read only memory */
    *(str + 1) = 'n';

    return 0;
}
